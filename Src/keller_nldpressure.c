/*
 * kellernld.c
 *
 * STM32 HAL-based library for Keller 4LD, 7LD, 7LHP and 9LD pressure sensors.
 *
 *  Created on: Apr 15, 2026
 *      Author: J. D. Hawkins
 */
#include "keller_nldpressure.h"

void keller_pressure_init_full(keller_pressure* sensor, I2C_HandleTypeDef* i2c, uint8_t i2c_addr, uint16_t timeout) {

	// Assign I2C values
	sensor->i2c_handle = i2c;
	sensor->i2c_address = i2c_addr;
	sensor->i2c_timeout = timeout;

	// Define initial invalid values
	// Invalid since min > max
	sensor->pressure_min = 1;
	sensor->pressure_max = -1;
	sensor->type = UNDEFINED;
	sensor->customer_id.value = 0xffffffff;
	sensor->year = 0;
	sensor->month = 0;
	sensor->day = 0;

}

void keller_pressure_init(keller_pressure* sensor, I2C_HandleTypeDef* i2c) {
	keller_pressure_init_full(sensor, i2c, ADDR_KELLER_PRESSURE, ADDR_KELLER_TIMEOUT_MS);
}

CHIL_STATUS keller_read_memory_word(keller_pressure* sensor, uint8_t memory_addr, uint16_t* buffer) {

	uint8_t raw[3];
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
		sensor->i2c_handle,
		(sensor->i2c_address << 1) + 1,
		memory_addr,
		1,
		raw,
		3,
		sensor->i2c_timeout
	);

	// Swap MSB and LSB
	if (status == HAL_OK) *buffer = ((uint16_t)raw[1] << 8) | raw[2];
	// Check that the status byte is valid
	if ((raw[0] & KELLER_SUCCESS_RESPONSE) != KELLER_SUCCESS_RESPONSE) return 1;

	// Return status
	return (status == HAL_OK) ? CHIL_OK : CHIL_ERROR;

}

CHIL_STATUS keller_read_memory(keller_pressure* sensor) {

	// Request measurement
	uint16_t keller_buffer;
	// Temporary customer ID
	uint32_t customer_id = 0;
	uint32_t p_raw_msword = 0;
	uint32_t p_raw_lsword = 0;

	//I2C status
	CHIL_STATUS status;

	// Read customer ID
	status = keller_read_memory_word(sensor, KELLER_MEMORY_CUST_ID0, &keller_buffer);
	if (!status) customer_id = ((uint32_t)keller_buffer) << 16; else return status;
	HAL_Delay(1);

	status = keller_read_memory_word(sensor, KELLER_MEMORY_CUST_ID1, &keller_buffer);
	if (status == CHIL_OK) {
		customer_id |= keller_buffer;
		// Assign to sensor
		sensor->customer_id.value = customer_id;
	} else return status;
	HAL_Delay(1);


	// Read date and pressure mode
	status = keller_read_memory_word(sensor, KELLER_MEMORY_SCALING0, &keller_buffer);
	if (status == CHIL_OK) {
		// Strip parameters
		// Set type
		sensor->type = (keller_buffer & 0b11);
		// Set date of calibration
		sensor->day = (keller_buffer >> 2) & 0b11111;
		sensor->month = (keller_buffer > 7) & 0b1111;
		sensor->year = 2010 + ((keller_buffer >> 11) & 0b11111);
	} else return status;
	HAL_Delay(1);

	/*************************************************************************/
	// Read raw pressure scaling
	/*************************************************************************/
	// Read pressure min, first read
	status = keller_read_memory_word(sensor, KELLER_MEMORY_SCALING1, &keller_buffer);
	if (status == CHIL_OK) p_raw_lsword = ((uint32_t)keller_buffer); else return status;
	HAL_Delay(1);
	// Wait and then read second word
	status = keller_read_memory_word(sensor, KELLER_MEMORY_SCALING2, &keller_buffer);
	HAL_Delay(1);

	if (status == CHIL_OK) {
		p_raw_msword = ((uint32_t)keller_buffer);
		p_raw_msword = (p_raw_lsword) | (p_raw_msword << 16);
		sensor->pressure_min = *(float*)(&p_raw_msword);
	} else return status;

	// Reset buffer variables
	keller_buffer = 0;
	p_raw_lsword = 0;
	p_raw_msword = 0;

	// Read  raw pressure scaling
	status = keller_read_memory_word(sensor, KELLER_MEMORY_SCALING3, &keller_buffer);
	if (status == CHIL_OK) p_raw_lsword = ((uint32_t)keller_buffer); else return status;
	HAL_Delay(1);

	status = keller_read_memory_word(sensor, KELLER_MEMORY_SCALING4, &keller_buffer);
	HAL_Delay(1);

	if (status == CHIL_OK) {
		p_raw_msword = ((uint32_t)keller_buffer) << 16;
		p_raw_msword |= (p_raw_lsword);
		sensor->pressure_max = *(float*)(&p_raw_msword);
	} else return status;

	// Perform validation
	if (sensor->pressure_max < sensor->pressure_min) return CHIL_ERROR;

	// Return success!
	return CHIL_OK;

}

CHIL_STATUS keller_measure(keller_pressure* sensor, keller_pressure_result* result) {

	// Setup measure command
	uint8_t buffer[5] = {0,0,0,0,0};
	buffer[0] = KELLER_COMMAND_MEASURE;
	// send to device
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
		sensor->i2c_handle,
		(sensor->i2c_address << 1),
		buffer,
		1,
		sensor->i2c_timeout
	);
	if (status != HAL_OK) return CHIL_ERROR;
	// wait 10 ms for conversion
	HAL_Delay(10);
	// retrieve data from device - first reset buffer
	buffer[0] = 0;
	status = HAL_I2C_Master_Receive(
		sensor->i2c_handle,
		(sensor->i2c_address << 1),
		buffer,
		5,
		sensor->i2c_timeout
	);
	if (status != HAL_OK) return CHIL_ERROR;
	// Check that status byte is valid
	if ((buffer[0] & KELLER_SUCCESS_RESPONSE) != KELLER_SUCCESS_RESPONSE) return CHIL_ERROR;
	// Perform conversion
	result->pressure    = (((uint16_t)buffer[1]) << 8) | buffer[2];
	result->temperature = (((uint16_t)buffer[3]) << 8) | buffer[5];

	// Check sensor configuration is valid
	if (sensor->pressure_max <= sensor->pressure_min) {
		// Set values to NaN - these are only used in debugging
		result->pressure_value = 0xffffffff;
		result->temperature_value = 0xffffffff;
	} else {
		// Perform value conversion
		result->pressure_value = (float)(result->pressure - 16384) * (sensor->pressure_max - sensor->pressure_min) / 32768 + sensor->pressure_min;
		result->temperature_value = (float)(result->temperature / 16 - 24) * 0.05f - 50.f;
	}

	return CHIL_OK;

}
