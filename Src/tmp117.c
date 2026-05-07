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

void tmp117_init_full(tmp117_config* config, I2C_HandleTypeDef* i2c, uint8_t i2c_address, uint16_t timeout) {

	// Assign I2C handle
	config->i2c_handle = i2c;
	// and address
	config->i2c_address = i2c_address;
	config->i2c_timeout = timeout;

	// Set the configuration to the default (power-up) value
	// (7.6.3 in datasheet)
	config->configuration.value = 0x0220;

}

void tmp117_init(tmp117_config* config, I2C_HandleTypeDef* i2c) {
	tmp117_init_full(config, i2c, ADDR_I2C_TMP117, TMP117_TIMEOUT);
}

uint8_t tmp117_read_configuration(tmp117_config* config) {
	uint8_t buffer[2];
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
		config->i2c_handle,
		(config->i2c_address << 1) | 0b00000001,
		TMP117_CONFIG_REG_ADDR,
		1,
		buffer,
		2,
		config->i2c_timeout
	);
	config->configuration.value = (
		((uint16_t)buffer[0] << 8) | buffer[1]
	);
	return status != HAL_OK;
}

uint8_t tmp117_write_configuration(tmp117_config* config) {
	uint8_t buffer[2];
	buffer[0] = (uint8_t)(config->configuration.value >> 8);
	buffer[1] = (uint8_t)(config->configuration.value & 0xff);
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
		config->i2c_handle,
		(config->i2c_address << 1) & 0b11111110,
		TMP117_CONFIG_REG_ADDR,
		1,
		buffer,
		2,
		config->i2c_timeout
	);
	return status != HAL_OK;
}

uint8_t tmp117_set_conversion_mode(tmp117_config* config, uint8_t conversion_mode) {

	// Check the conversion mode is valid
	if (conversion_mode > 3) return 1;

	// Update configuration and return with error (1) if fail
	if (tmp117_read_configuration(config)) return 1;

	// Assign conversion mode
	config->configuration.mode = conversion_mode & 0b11;

	// Write the updated config
	if (tmp117_write_configuration(config)) return 1;

	// Update configuration and return with error (1) if fail
	if (tmp117_read_configuration(config)) return 1;

	// Otherwise assume success!
	return 0;

}

uint8_t tmp117_measure_oneshot_begin(tmp117_config* sensor, tmp117_result* result) {

	// Set conversion mode to oneshot
	if (tmp117_set_conversion_mode(sensor, TMP117_CONFIG_MODE_ONESHOT)) return 1; // fail if this fails
	// Assign conversion time and start tick
	result->conversion_time_ms = tmp117_get_conversion_time(sensor);
	result->conversion_start = HAL_GetTick();
	result->ready = 0;

	// Assign current result for tracking
	sensor->current_result = result;

	return 0;

}

uint8_t tmp117_measure_oneshot_check(tmp117_config* sensor, tmp117_result* result) {

	uint8_t buffer[2];

	// If we haven't yielded enough time
	if (HAL_GetTick() <= result->conversion_start + result->conversion_time_ms) return 0;

	// Otherwise read the configuration register
	tmp117_read_configuration(sensor);
	// This clears the Data_Ready flag if it is complete, so we need to read the result now.
	// To catch the case that we have already cleared the result and stored the flag, we
	// also check whether the result itself is ready
	if (sensor->configuration.data_ready || result->ready) {
		// Set result flag to ready
		result->ready = 1;
		// Read temperature register
		HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
			sensor->i2c_handle,
			(sensor->i2c_address << 1) + 1,
			TMP117_TEMPERATURE_REG_ADDR,
			1,
			buffer,
			2,
			sensor->i2c_timeout
		);
		// Return 1 on failure
		if (status != HAL_OK) return 1;

		// Convert the temperature result
		result->temperature = ((uint16_t)buffer[0]) << 8 | buffer[1];
		// if it is equal to -256, we are not ready so try again -
		// something has really gone wrong for this to fail!
		if (result->temperature == -256) return 1;
		// and set averaging used
		switch(sensor->configuration.avg) {
			case TMP117_CONFIG_AVERAGE_NONE:
				result->averages = 0;
				break;
			case TMP117_CONFIG_AVERAGE_8:
				result->averages = 8;
				break;
			case TMP117_CONFIG_AVERAGE_32:
				result->averages = 32;
				break;
			case TMP117_CONFIG_AVERAGE_64:
				result->averages = 64;
				break;
		}
		// Return 0 on success
		return 0;
	// Otherwise return 1 to indicate not ready
	} else return 1;

}

uint8_t tmp117_measure_oneshot_block(tmp117_config* sensor, tmp117_result* result, uint8_t max_attempts) {
	if (max_attempts == 0) return 1;
	while (max_attempts-- > 0) {
		HAL_Delay(result->conversion_time_ms);
		if (!tmp117_measure_oneshot_check(sensor, result)) return 0; // valid measurement!
	}
	return 1; // failed
}

void tmp117_clear_result(tmp117_result* result) {
	result->averages = 0;
	result->conversion_start = 0;
	result->conversion_time_ms = 0;
	result->ready = 0;
	result->temperature = 0x8000; // value used by TMP117 to indicate bad data
}

/*
 * tmp117_get_conversion_time
 *
 * Description: get conversion time (in ms) from the configuration
 *              register, determined by CONV and AVG values.
 *
 * Input arguments:
 *
 *     (tmp117_config*) config
 *     Pointer to the TMP117 configuration register
 *
 * Return value:
 *
 *     Conversion time in ms. Returns 0 if invalid.
 */
uint16_t tmp117_get_conversion_time(tmp117_config* config) {
	uint16_t conversion_time = 0;
	// Read current configuration
	tmp117_read_configuration(config);
	// Convert conversion time to numeric value
	switch (config->configuration.conv) {
		case TMP117_CONFIG_CONVERSION_TIME_15ms :
			conversion_time = 16; // round-up
			break;
		case TMP117_CONFIG_CONVERSION_TIME_125ms:
			conversion_time = 125;
			break;
		case TMP117_CONFIG_CONVERSION_TIME_250ms:
			conversion_time = 250;
			break;
		case TMP117_CONFIG_CONVERSION_TIME_500ms:
			conversion_time = 500;
			break;
		case TMP117_CONFIG_CONVERSION_TIME_1s   :
			conversion_time = 1000;
			break;
		case TMP117_CONFIG_CONVERSION_TIME_4s   :
			conversion_time = 4000;
			break;
		case TMP117_CONFIG_CONVERSION_TIME_8s   :
			conversion_time = 8000;
			break;
		case TMP117_CONFIG_CONVERSION_TIME_16s  :
			conversion_time = 16000;
			break;
		default:
			return 0;
	}

	// Correct for averaging behaviour (see Table 7-7 in TMP117 datasheet)
	if (config->configuration.avg == TMP117_CONFIG_AVERAGE_8 && conversion_time < 125)
		conversion_time = 125;
	else if (config->configuration.avg == TMP117_CONFIG_AVERAGE_32 && conversion_time < 500)
		conversion_time = 500;
	else if (config->configuration.avg == TMP117_CONFIG_AVERAGE_64 && conversion_time < 1000)
		conversion_time = 1000;

	// Return value
	return conversion_time;

}

//void tmp117_init(I2C_HandleTypeDef* hi2c) {
//
//	// Assign buffer
//	uint8_t buffer[3];
//	buffer[0] = 0x01;
//	buffer[1] = 0;
//	buffer[2] = 0;
//
//	// Get existing configuration
//	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, buffer, 1, TMP117_TIMEOUT);
//	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, &(buffer[1]), 2, TMP117_TIMEOUT);
//
//	// Modify configuration buffer
//
//	// Set to one shot mode
//	buffer[1] = buffer[1] | 0b00000011;
//
//	// Write configuration
//	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, buffer, 3, TMP117_TIMEOUT);
//
//
//}
//
//int16_t tmp117_request_temperature(I2C_HandleTypeDef* hi2c) {
//
//	// Assign buffer
//	uint8_t buffer[2];
//	buffer[0] = 0;
//	buffer[1] = 0;
//
//	// Assign result
//	int16_t result = 0;
//
//	// Request temperature
//	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, buffer, 1, TMP117_TIMEOUT);
//	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, buffer, 2, TMP117_TIMEOUT);
//
//	result = buffer[1] | buffer[0] << 8;
//
//	return result;
//
//}
//
//uint32_t tmp117_request_partial_id(I2C_HandleTypeDef* hi2c) {
//
//	// Create buffer to store result in
//	uint8_t 	buffer[6];
//	uint8_t 	rx_addr;
//	uint32_t 	result;
//
//	// Request NIST ID[5], ID[4]
//	rx_addr = 0x05;
//	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, &rx_addr, 1, TMP117_TIMEOUT);
//	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, &(buffer[0]), 2, TMP117_TIMEOUT);
//
//	// Request NIST ID[3], ID[2]
//	rx_addr = 0x06;
//	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, &rx_addr, 1, TMP117_TIMEOUT);
//	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, &(buffer[2]), 2, TMP117_TIMEOUT);
//
//	// Request NIST ID[1], ID[0]
//	rx_addr = 0x08;
//	HAL_I2C_Master_Transmit(hi2c, TMP117_ADDR_WRITE, &rx_addr, 1, TMP117_TIMEOUT);
//	HAL_I2C_Master_Receive(hi2c, TMP117_ADDR_READ, &(buffer[4]), 2, TMP117_TIMEOUT);
//
//	// Compile NIST ID
//    result = result + ((uint64_t)buffer[2] << 24); // (40 = 8*3)
//	result = result + ((uint64_t)buffer[3] << 16); // (40 = 8*2)
//	result = result + ((uint64_t)buffer[4] <<  8); // (08 = 8*1)
//	result = result + ((uint64_t)buffer[5]);
//
//	return result;
//
//
//}

#ifdef __cplusplus
}
#endif
