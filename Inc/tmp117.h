/*
 * tmp117.h
 *
 *  Created on: Jul 2, 2024
 *      Author: sgljh3
 */

#ifndef INC_TMP117_H_
#define INC_TMP117_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_i2c_ex.h"

#define TMP117_ADDR_WRITE 0b10010000
#define TMP117_ADDR_READ  0b10010001

#define TMP117_TIMEOUT 100 // milliseconds timeout

void 		tmp117_init(I2C_HandleTypeDef*);
int16_t 	tmp117_request_temperature(I2C_HandleTypeDef*);
uint32_t	tmp117_request_partial_id(I2C_HandleTypeDef*);

#endif /* INC_TMP117_H_ */
