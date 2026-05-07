/*
 * chil_conductivity.h
 *
 *  Created on: April 17, 2026
 *      Author: J. D. Hawkins
 */

#ifndef CHIL_CONDUCTIVITY_H
#define CHIL_CONDUCTIVITY_H

#ifndef CHIL_EC_ADC_TIMEOUT
#define CHIL_EC_ADC_TIMEOUT 100
#endif

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_i2c_ex.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_hal_tim_ex.h"
#include "stm32l4xx_hal_opamp.h"
#include "stm32l4xx_hal_opamp_ex.h"
#include "stm32l4xx_hal_adc.h"
#include "stm32l4xx_hal_adc_ex.h"

// Limit the number of averages to a maximum value so we can
// statically allocate the result struct size
#define CHIL_EC_MAX_AVERAGES 32

typedef enum {
	DISABLED,
	STARTUP,
	MEASURING,
	AVERAGING,
	SHUTDOWN,
	DATA_READY,
} chil_ec_state;

typedef struct chil_ec_result {
	// Raw result buffer
	uint16_t buffer[CHIL_EC_MAX_AVERAGES];
	// Result
	uint16_t result;
	// Number of averages used
	uint8_t averages;
	// Used to indicate if result is ready
	uint8_t ready;
} chil_ec_result;

typedef struct chil_ec_config {

	// Timer parameters
	TIM_HandleTypeDef* timer_handle;
	uint32_t timer_pwm_channel;
	uint32_t timer_adc_channel;
	// ADC parameters
	ADC_HandleTypeDef* adc_handle;
	// OPAMP parameters
	OPAMP_HandleTypeDef* opamp_handle;

	uint8_t  averages;
	uint16_t timer_frequency;
	chil_ec_result* current_result;
} chil_ec_config;

// make conductivity measurement
uint8_t chil_ec_init(chil_ec_config* sensor, TIM_HandleTypeDef* timer, ADC_HandleTypeDef* adc, OPAMP_HandleTypeDef* opamp, uint32_t timer_pwn_channel, uint32_t timer_adc_channel);

uint8_t chil_ec_measure(chil_ec_config* sensor, chil_ec_result* result, uint8_t averages);

// 1. ENABLE TIMER
// 2. START OP AMP
// 3. ENABLE ADC
// 4. Wait for conversion
// 5. GET RESULT
// 6. DISABLE ADC
// 7. DISABLE OP AMP
// 8. DISABLE TIMER

//
//// enable timer output
//HAL_TIM_Base_Start(&htim1);
//HAL_TIM_PWM_Init(&htim1);
//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//HAL_TIMEx_PWMN_Start  (&htim1, TIM_CHANNEL_1);
//HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_4);
//
//// turn on OPAMP
//HAL_OPAMP_Start(&hopamp1);
//
//// enable ADC conversion - in blocking mode
//HAL_ADC_Start(&hadc1);
//
//HAL_ADC_PollForConversion(&hadc1, ADC_POLL_TIMEOUT);
//
//// get result
//uint16_t ec_adc_val;
//
//ec_adc_val = (uint16_t)HAL_ADC_GetValue(&hadc1);
//
//// all done - turn off EC measurement
//
//// switch off ADC
//HAL_ADC_Stop(&hadc1);
//
//// turn off OPAMP
//HAL_OPAMP_Stop(&hopamp1);
//
//// turn off timers
//HAL_TIM_Base_Stop(&htim1);
//HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
//HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
//HAL_TIM_OC_Stop(&htim1, TIM_CHANNEL_4);

#endif
