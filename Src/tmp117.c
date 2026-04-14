/*
 * tmp117.c
 *
 *  Created on: Jul 2, 2024
 *      Author: sgljh3
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include "tmp117.h"

void tmp117_init(I2C_HandleTypeDef* hi2c) {

	// Assign buffer
	uint8_t buffer[3];
	buffer[0] = 0x01;
	buffer[1] = 0;
	buffer[2] = 0;

	// Give time for power-up
	HAL_Delay(5);

	// Get existing configuration
	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, buffer, 1, TMP117_TIMEOUT);
	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, &(buffer[1]), 2, TMP117_TIMEOUT);

	// Modify configuration buffer

	// Set to one shot mode
	buffer[1] = buffer[1] | 0b00000011;

	// Write configuration
	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, buffer, 3, TMP117_TIMEOUT);


}

int16_t tmp117_request_temperature(I2C_HandleTypeDef* hi2c) {

	// Assign buffer
	uint8_t buffer[2];
	buffer[0] = 0;
	buffer[1] = 0;

	// Assign result
	int16_t result = 0;

	// Request temperature
	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, buffer, 1, TMP117_TIMEOUT);
	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, buffer, 2, TMP117_TIMEOUT);

	result = buffer[1] | buffer[0] << 8;

	return result;

}

uint32_t tmp117_request_partial_id(I2C_HandleTypeDef* hi2c) {

	// Create buffer to store result in
	uint8_t 	buffer[6];
	uint8_t 	rx_addr;
	uint32_t 	result;

	// Request NIST ID[5], ID[4]
	rx_addr = 0x05;
	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, &rx_addr, 1, TMP117_TIMEOUT);
	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, &(buffer[0]), 2, TMP117_TIMEOUT);

	// Request NIST ID[3], ID[2]
	rx_addr = 0x06;
	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, &rx_addr, 1, TMP117_TIMEOUT);
	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, &(buffer[2]), 2, TMP117_TIMEOUT);

	// Request NIST ID[1], ID[0]
	rx_addr = 0x08;
	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, &rx_addr, 1, TMP117_TIMEOUT);
	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, &(buffer[4]), 2, TMP117_TIMEOUT);

	// Compile NIST ID
    result = result + ((uint64_t)buffer[2] << 24); // (40 = 8*3)
	result = result + ((uint64_t)buffer[3] << 16); // (40 = 8*2)
	result = result + ((uint64_t)buffer[4] <<  8); // (08 = 8*1)
	result = result + ((uint64_t)buffer[5]);

	return result;


}

#ifdef __cplusplus
}
#endif
