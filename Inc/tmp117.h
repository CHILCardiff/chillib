/*
 * tmp117.h
 *
 *  Created on: Jul 2, 2024
 *      Author: sgljh3
 */

#ifndef INC_TMP117_H_
#define INC_TMP117_H_

#include "chillib.h"

#ifdef STM32L433xx

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_i2c_ex.h"

#define ADDR_I2C_TMP117 0b1001000

//#define TMP117_ADDR_WRITE 0b10010000
//#define TMP117_ADDR_READ  0b10010001

#ifndef TMP117_TIMEOUT
#define TMP117_TIMEOUT 100 // milliseconds timeout
#endif

// TMP117 Configuration register (0x01)
// defaults to 0x0220
typedef union {
	__attribute__((packed)) struct {
		uint16_t _           : 1; // LSB 0
		uint16_t soft_reset  : 1;
		uint16_t dralert     : 1;
		uint16_t pol         : 1;
		uint16_t tna         : 1;
		uint16_t avg         : 2;
		uint16_t conv        : 3;
		uint16_t mode        : 2;
		uint16_t eeprom_busy : 1;
		uint16_t data_ready  : 1;
		uint16_t low_alert   : 1;
		uint16_t high_alert  : 1; // MSB 15
		//
//		uint16_t high_alert  : 1;
//		uint16_t low_alert   : 1;
//		uint16_t data_ready  : 1;
//		uint16_t eeprom_busy : 1;
//		uint16_t mode        : 2;
//		uint16_t conv        : 3;
//		uint16_t avg         : 2;
//		uint16_t tna         : 1;
//		uint16_t pol         : 1;
//		uint16_t dralert     : 1;
//		uint16_t soft_reset  : 1;
//		uint16_t _           : 1;
	};
	uint16_t value;
} tmp117_config_reg;
#define TMP117_CONFIG_REG_ADDR 0x01

#define TMP117_CONFIG_READ  0b1111111111111111;
#define TMP117_CONFIG_WRITE 0b0000111111111110;

// TMP117 conversion mode
#define TMP117_CONFIG_MODE_CONTINUOUS 0b00
#define TMP117_CONFIG_MODE_SHUTDOWN   0b01
#define TMP117_CONFIG_MODE_ONESHOT    0b11

// TMP117 conversion cycle time
#define TMP117_CONFIG_CONVERSION_TIME_15ms  0b000
#define TMP117_CONFIG_CONVERSION_TIME_125ms 0b001
#define TMP117_CONFIG_CONVERSION_TIME_250ms 0b010
#define TMP117_CONFIG_CONVERSION_TIME_500ms 0b011
#define TMP117_CONFIG_CONVERSION_TIME_1s    0b100
#define TMP117_CONFIG_CONVERSION_TIME_4s    0b101
#define TMP117_CONFIG_CONVERSION_TIME_8s    0b110
#define TMP117_CONFIG_CONVERSION_TIME_16s   0b111

// TMP117 averaging mode
#define TMP117_CONFIG_AVERAGE_NONE 0b00
#define TMP117_CONFIG_AVERAGE_8    0b01
#define TMP117_CONFIG_AVERAGE_32   0b10
#define TMP117_CONFIG_AVERAGE_64   0b11

#define TMP117_TEMPERATURE_REG_ADDR 0x00

typedef struct tmp177_result {
	int16_t  temperature;
	uint8_t  averages;
	uint32_t conversion_time_ms;
	uint32_t conversion_start;
	uint8_t  ready;
} tmp117_result;

typedef struct tmp117_config {
	I2C_HandleTypeDef* i2c_handle;
	uint8_t            i2c_address;
	uint16_t           i2c_timeout;
	// Configuration register
	tmp117_config_reg  configuration;
	// Current result to handle interrupt/polling data ready
	tmp117_result* 	   current_result;
} tmp117_config;

void tmp117_init(tmp117_config* config, I2C_HandleTypeDef*);
void tmp117_init_full(tmp117_config* config, I2C_HandleTypeDef*, uint8_t i2c_address, uint16_t timeout);

uint8_t tmp117_read_configuration(tmp117_config* config);
uint8_t tmp117_write_configuration(tmp117_config* config);


//int16_t 	tmp117_request_temperature(I2C_HandleTypeDef*);
//uint32_t	tmp117_request_partial_id(I2C_HandleTypeDef*);

uint16_t tmp117_get_conversion_time(tmp117_config* config);
uint8_t  tmp117_measure_oneshot_begin(tmp117_config* sensor, tmp117_result* result);
uint8_t  tmp117_measure_oneshot_block(tmp117_config* sensor, tmp117_result* result, uint8_t max_attempts);
uint8_t  tmp117_measure_oneshot_check(tmp117_config* sensor, tmp117_result* result);

#endif
#endif /* INC_TMP117_H_ */
