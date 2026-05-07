/*
 * kellernld.h
 *
 * STM32 HAL-based library for Keller 4LD, 7LD, 7LHP and 9LD pressure sensors.
 *
 *  Created on: Apr 15, 2026
 *      Author: J. D. Hawkins
 */
#ifndef KELLER_NLD
#define KELLER_NLD

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_i2c_ex.h"

#ifndef ADDR_KELLER_PRESSURE
#define ADDR_KELLER_PRESSURE 0x40
#endif


#ifndef ADDR_KELLER_TIMEOUT_MS
#define ADDR_KELLER_TIMEOUT_MS 100
#endif

#define KELLER_SUCCESS_RESPONSE 0b01000000

#define KELLER_COMMAND_MEASURE 0xAC

#define KELLER_MEMORY_CUST_ID0 0x00
#define KELLER_MEMORY_CUST_ID1 0x01
#define KELLER_MEMORY_SCALING0 0x12
#define KELLER_MEMORY_SCALING1 0x13
#define KELLER_MEMORY_SCALING2 0x14
#define KELLER_MEMORY_SCALING3 0x15
#define KELLER_MEMORY_SCALING4 0x16

typedef union {
	__attribute__((packed)) struct {
		uint16_t file;
		uint16_t place    : 10;
		uint8_t equipment : 6;
	};
	uint32_t value;
} keller_pressure_customer_id ;

typedef enum {
	// Section 5.1 of Keller 7LD Communications Manual
	PR        = 0,
	PA        = 1,
	PAA       = 2,
	UNDEFINED = 3
} keller_pressure_type;

typedef struct keller_pressure_result {
	uint16_t pressure;
	uint16_t temperature;
	float    pressure_value;
	float	 temperature_value;
} keller_pressure_result;

typedef struct keller_pressure {

	// Address
	uint8_t i2c_address;
	uint8_t i2c_timeout;
	I2C_HandleTypeDef* i2c_handle;

	// Unique customer ID
	keller_pressure_customer_id customer_id;
	// Calibration date parameter
	uint32_t year; // bits 11..15
	uint8_t  month; // bits 7..10
	uint8_t  day; // bits 2..6

	// Pressure sensor type
	keller_pressure_type type;

	// Minimum and maximum pressure
	float pressure_min;
	float pressure_max;

} keller_pressure;

void keller_pressure_init_full(keller_pressure* sensor, I2C_HandleTypeDef* i2c, uint8_t i2c_addr, uint16_t timeout);
void keller_pressure_init(keller_pressure* sensor, I2C_HandleTypeDef* i2c);
uint8_t keller_read_memory_word(keller_pressure* sensor, uint8_t memory_addr, uint16_t* buffer);
uint8_t keller_read_memory(keller_pressure* sensor);

uint8_t keller_measure(keller_pressure* sensor, keller_pressure_result* result);

#endif
