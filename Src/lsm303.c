/*
 * lsm303.c
 *
 *  Created on: Apr 8, 2026
 *      Author: J. D. Hawkins
 */

#include "chillib.h"
#ifdef STM32L433xx
#include "lsm303.h"

CHIL_STATUS lsm303_init_config(lsm303_config* config, I2C_HandleTypeDef* i2c, uint8_t buffer_length) {

	// Reset all registers
	config->cfg_reg_a_m     = (lsm303_cfg_reg_a_m){(uint8_t)0b10000000};
	config->cfg_reg_c_m     = (lsm303_cfg_reg_c_m){(uint8_t)0b00000000};
	config->ctrl_reg1_a     = (lsm303_ctrl_reg1_a){(uint8_t)0b00000111};
	config->ctrl_reg3_a     = (lsm303_ctrl_reg3_a){(uint8_t)0b00000000};
	config->ctrl_reg4_a     = (lsm303_ctrl_reg4_a){(uint8_t)0b00000000};
	config->ctrl_reg5_a     = (lsm303_ctrl_reg5_a){(uint8_t)0b00000000};
	config->fifo_ctrl_reg_a = (lsm303_fifo_ctrl_reg_a){(uint8_t)0b00000000};
	// Assign I2C handle
	config->i2c_handle 		= i2c;
	// buffer_length is number of samples for each axis
	config->buffer_length   = buffer_length;
	config->enabled			= 0;
	config->data_available  = 0;
	config->samples         = 0;

	// Write config as-is
	return CHIL_OK;
}

// Add register commands to change power
CHIL_STATUS lsm303_enable(lsm303_config* config) {
	CHIL_STATUS status = write_lsm303_acc_reg(config, LSM303_ADDR_CTRL_REG1_A, config->ctrl_reg1_a.value);
	config->enabled = (status == CHIL_OK) ? 1 : 0;
	return status;
}

CHIL_STATUS lsm303_disable(lsm303_config* config) {

	CHIL_STATUS status;
	// Read current register value
	read_lsm303_acc_reg(config, LSM303_ADDR_CTRL_REG1_A, &(config->ctrl_reg1_a.value), 1);
	// Rewrite with ODR blanked to zero
	status = write_lsm303_acc_reg(config, LSM303_ADDR_CTRL_REG1_A, config->ctrl_reg1_a.value & 0b00001111);

	// If succesful, set enabled = 0
	config->enabled = (status == CHIL_OK) ? 0 : 1;
	return status;

}

// Generic read-write commands
CHIL_STATUS write_lsm303_reg(lsm303_config* config, uint8_t acc0_mag1, uint8_t reg_addr, uint8_t value) {
	HAL_StatusTypeDef status;
	uint8_t device_addr = acc0_mag1 ? ADDR_LSM303MAG : ADDR_LSM303ACC;
	uint8_t reg_value;
	status = HAL_I2C_Mem_Write(config->i2c_handle, (device_addr << 1) & 0xFE, reg_addr, 1, &value, 1, LSM303_I2C_TIMEOUT);
	if (status != HAL_OK) return CHIL_ERROR;

	status = HAL_I2C_Mem_Read(config->i2c_handle, (device_addr << 1) | 1, reg_addr, 1, &reg_value, 1, LSM303_I2C_TIMEOUT);
	if (status != HAL_OK) {
		return CHIL_ERROR;
	}
	return CHIL_OK;
}

CHIL_STATUS  read_lsm303_reg(lsm303_config* config, uint8_t acc0_mag1, uint8_t reg_addr, uint8_t* buffer, uint8_t len) {
	HAL_StatusTypeDef status;
	uint8_t device_addr = acc0_mag1 ? ADDR_LSM303MAG : ADDR_LSM303ACC;
	status = HAL_I2C_Mem_Read(config->i2c_handle, (device_addr << 1) | 1, reg_addr, 1, buffer, len, LSM303_I2C_TIMEOUT);
	return (status == HAL_OK) ? CHIL_OK : CHIL_ERROR;
}

// Accelerometer/magnetometer specific read-write commands
CHIL_STATUS write_lsm303_acc_reg(lsm303_config* config, uint8_t reg_addr, uint8_t value) {
	return write_lsm303_reg(config, 0, reg_addr, value);
}
CHIL_STATUS write_lsm303_mag_reg(lsm303_config* config, uint8_t reg_addr, uint8_t value) {
	return write_lsm303_reg(config, 1, reg_addr, value);
}
CHIL_STATUS  read_lsm303_acc_reg(lsm303_config* config, uint8_t reg_addr, uint8_t* buffer, uint8_t len) {
	return read_lsm303_reg(config, 0, reg_addr, buffer, len);
}
CHIL_STATUS  read_lsm303_mag_reg(lsm303_config* config, uint8_t reg_addr, uint8_t* buffer, uint8_t len) {
	return read_lsm303_reg(config, 1, reg_addr, buffer, len);
}

void lsm303_callback(lsm303_config* config, uint8_t* buffer) {
	if (config->enabled && config->data_available == 0) {
		config->data_available = 1;
	}
}

uint16_t lsm303_get_sample_rate(lsm303_config* config) {
	switch (config->ctrl_reg1_a.output_data_rate) {
		case LSM303_ACC_ODR_POWER_DOWN:
			return 0;
		case LSM303_ACC_ODR_1Hz:
			return 1;
		case LSM303_ACC_ODR_10Hz:
			return 10;
		case LSM303_ACC_ODR_25Hz:
			return 25;
		case LSM303_ACC_ODR_50Hz:
			return 50;
		case LSM303_ACC_ODR_100Hz:
			return 100;
		case LSM303_ACC_ODR_200Hz:
			return 200;
		case LSM303_ACC_ODR_400Hz:
			return 400;
		case LSM303_ACC_ODR_1344Hz:
			return 1344;
		case LSM303_ACC_ODR_1620Hz:
			return 1620;
		default:
			return 0;
	}
}

int16_t lsm303_acc_convert_buffer_to_data(lsm303_config* ecompass_config, uint8_t* lsm303_buffer, uint16_t offset) {
	return (int16_t)((uint16_t)(lsm303_buffer[offset+1] << 8) + lsm303_buffer[offset]);
}


CHIL_STATUS read_lsm303_data(lsm303_config* config, lsm303_data_3axis* data, uint8_t acc0_mag1) {

	// Setup buffer and read 6 values from data into it
	uint8_t buffer[6]; CHIL_STATUS status;
	// the values are ordered X(LSB), X(MSB), Y(L), Y(H), Z(L), Z(H).
	status = read_lsm303_reg(config, acc0_mag1, 0x80 | (acc0_mag1 ? LSM303_ADDR_OUTX_L_REG_M : LSM303_ADDR_OUTX_L_A), buffer, 6);
	if (status == CHIL_OK) {

		// Cast and format the data
		data->x = (int16_t)((uint16_t)buffer[0] | (uint16_t)buffer[1] << 8);
		data->y = (int16_t)((uint16_t)buffer[2] | (uint16_t)buffer[3] << 8);
		data->z = (int16_t)((uint16_t)buffer[4] | (uint16_t)buffer[5] << 8);

	} else return CHIL_ERROR;

	// We made it!
	return CHIL_OK;

}
CHIL_STATUS read_lsm303_acc_data(lsm303_config* config, lsm303_data_3axis* data) {
	return read_lsm303_data(config, data, 0);
}
CHIL_STATUS read_lsm303_mag_data(lsm303_config* config, lsm303_data_3axis* data) {
	return read_lsm303_data(config, data, 1);
}

#endif
