/*
 * lsm303agr.h
 *
 *  Created on: Apr 8, 2026
 *      Author: J. D. Hawkins
 */

#ifndef INC_LSM303_H_
#define INC_LSM303_H_

#include "stm32l4xx_hal.h"

// Define I2C addresses
#ifndef ADDR_LSM303ACC
#define ADDR_LSM303ACC 0b0011001
#endif

#ifndef ADDR_LSM303MAG
#define ADDR_LSM303MAG 0b0011110
#endif

#define LSM303_I2C_MULTIPLE_READ 0x80
#define LSM303_I2C_TIMEOUT 1000

/*****************************************************************************/
// Define register address (magnetometer)
/*****************************************************************************/
#define LSM303_ADDR_CFG_REG_A_M 0x60

#define LSM303_MAG_ODR_10Hz  0b00;
#define LSM303_MAG_ODR_20Hz  0b01;
#define LSM303_MAG_ODR_50Hz  0b10;
#define LSM303_MAG_ODR_100Hz 0b11;

#define LSM303_MAG_MODE_CONTINUOUS 0b00;
#define LSM303_MAG_MODE_SINGLE     0b01;
#define LSM303_MAG_MODE_IDLE       0b10;

typedef union {
	__attribute__((packed)) struct {
		uint8_t mode_select     : 2;
		uint8_t output_data_rate: 2;
		uint8_t low_power       : 1;
		uint8_t soft_reset      : 1;
		uint8_t reboot          : 1;
		uint8_t comp_temp_en    : 1;
	};
	uint8_t value;
} lsm303_cfg_reg_a_m;

#define LSM303_ADDR_CFG_REG_C_M 0x62

typedef union {
	__attribute__((packed)) struct {
		uint8_t int_mag_drdy      : 1;
		uint8_t self_test         : 1;
		uint8_t __                : 1;
		uint8_t ble               : 1;
		uint8_t bdu               : 1;
		uint8_t i2c_disable       : 1;
		uint8_t interrupt_mag_pin : 1;
		uint8_t _                 : 1;
	};
	uint8_t value;
} lsm303_cfg_reg_c_m;

/*****************************************************************************/
// Define register address (accelerometer)
/*****************************************************************************/
#define LSM303_ADDR_FIFO_START 0x28
#define LSM303_ADDR_CTRL_REG1_A 0x20

typedef union {
	__attribute__((packed)) struct {
		uint8_t x_axis_enable    : 1;
		uint8_t y_axis_enable    : 1;
		uint8_t z_axis_enable    : 1;
		uint8_t low_power_enable : 1;
		uint8_t output_data_rate : 4;
	};
	uint8_t value;
} lsm303_ctrl_reg1_a;

#define LSM303_ACC_ODR_POWER_DOWN 0b0000
#define LSM303_ACC_ODR_1Hz        0b0001
#define LSM303_ACC_ODR_10Hz       0b0010
#define LSM303_ACC_ODR_25Hz       0b0011
#define LSM303_ACC_ODR_50Hz       0b0100
#define LSM303_ACC_ODR_100Hz      0b0101
#define LSM303_ACC_ODR_200Hz      0b0110
#define LSM303_ACC_ODR_400Hz      0b0111
#define LSM303_ACC_ODR_1620Hz     0b1000
#define LSM303_ACC_ODR_1344Hz     0b1001

#define LSM303_ADDR_CTRL_REG3_A 0x22

typedef union {
	__attribute__((packed)) struct {
		uint8_t _                   : 1;
		uint8_t int1_fifo_overrun   : 1;
		uint8_t int1_fifo_watermark : 1;
		uint8_t int1_drdy2          : 1;
		uint8_t int1_drdy1          : 1;
		uint8_t int1_aoi2           : 1;
		uint8_t int1_aoi1           : 1;
		uint8_t int1_click          : 1;
	};
	uint8_t value;
} lsm303_ctrl_reg3_a;

#define LSM303_ADDR_CTRL_REG4_A 0x23

typedef union {
	__attribute__((packed)) struct {
		uint8_t spi_enable        : 1;
		uint8_t self_test         : 2;
		uint8_t high_resolution   : 1;
		uint8_t full_scale        : 2;
		uint8_t ble : 1;
		uint8_t bdu : 1;
	};
	uint8_t value;
} lsm303_ctrl_reg4_a;

#define LSM303_ACC_BDU_CONTINUOUS  0x00
#define LSM303_ACC_BLE_LSB_LOWER   0x00
#define LSM303_ACC_BLE_LSB_UPPER   0x00
#define LSM303_ACC_FULL_SCALE_2G   0b00
#define LSM303_ACC_FULL_SCALE_4G   0b01
#define LSM303_ACC_FULL_SCALE_8G   0b10
#define LSM303_ACC_FULL_SCALE_16G  0b11

#define LSM303_ADDR_CTRL_REG5_A 0x24

typedef union {
	__attribute__((packed)) struct {
		uint8_t d4d_enable_int2 : 1;
		uint8_t latch_int2      : 1;
		uint8_t d4d_enable_int1 : 1;
		uint8_t latch_int1      : 1;
		uint8_t _               : 2;
		uint8_t fifo_enable     : 1;
		uint8_t boot            : 1;
	};
	uint8_t value;
} lsm303_ctrl_reg5_a;

#define LSM303_ADDR_FIFO_CTRL_REG_A 0x2e

typedef union {
	__attribute__((packed)) struct {
		uint8_t fifo_watermark_level : 5;
		uint8_t trigger              : 1;
		uint8_t fifo_mode            : 2;
	};
	uint8_t value;
} lsm303_fifo_ctrl_reg_a;

#define LSM303_ACC_FIFO_MODE_BYPASS         0b00
#define LSM303_ACC_FIFO_MODE_FIFO           0b01
#define LSM303_ACC_FIFO_MODE_STREAM         0b10
#define LSM303_ACC_FIFO_MODE_STREAM_TO_FIFO 0b11

/*****************************************************************************/
typedef struct lsm303_config {
	I2C_HandleTypeDef* i2c_handle;
	lsm303_cfg_reg_a_m cfg_reg_a_m;
	lsm303_cfg_reg_c_m cfg_reg_c_m;
	lsm303_ctrl_reg1_a ctrl_reg1_a;
	lsm303_ctrl_reg3_a ctrl_reg3_a;
	lsm303_ctrl_reg4_a ctrl_reg4_a;
	lsm303_ctrl_reg5_a ctrl_reg5_a;
	lsm303_fifo_ctrl_reg_a fifo_ctrl_reg_a;
	uint8_t buffer_length;
	volatile uint8_t data_available;
	volatile uint8_t enabled;
	volatile uint32_t samples;
} lsm303_config;
/*****************************************************************************/

uint8_t lsm303_enable(lsm303_config* config);
void lsm303_disable(lsm303_config* config);

void init_lsm303_config(lsm303_config* config, I2C_HandleTypeDef* i2c, uint8_t buffer_length);

// Generic read-write commands
uint8_t write_lsm303_reg(lsm303_config* config, uint8_t acc0_mag1, uint8_t reg_addr, uint8_t value);
uint8_t  read_lsm303_reg(lsm303_config* config, uint8_t acc0_mag1, uint8_t reg_addr, uint8_t* buffer, uint8_t len);
// Accelerometer/magnetometer specific read-write commands
uint8_t write_lsm303_acc_reg(lsm303_config* config, uint8_t reg_addr, uint8_t value);
uint8_t write_lsm303_mag_reg(lsm303_config* config, uint8_t reg_addr, uint8_t value);
uint8_t  read_lsm303_acc_reg(lsm303_config* config, uint8_t reg_addr, uint8_t* buffer, uint8_t len);
uint8_t  read_lsm303_mag_reg(lsm303_config* config, uint8_t reg_addr, uint8_t* buffer, uint8_t len);

void lsm303_callback(lsm303_config* config, uint8_t* buffer);

uint16_t lsm303_get_sample_rate(lsm303_config* config);
int16_t lsm303_acc_convert_buffer_to_data(lsm303_config* ecompass_config, uint8_t* lsm303_buffer, uint16_t offset);

#endif /* INC_LSM303_H_ */
