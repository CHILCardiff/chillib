/*
 * chil_conductivity.h
 *
 *  Created on: April 17, 2026
 *      Author: J. D. Hawkins
 */

#include "chil_conductivity.h"

uint8_t chil_ec_init(
	chil_ec_config* sensor,
	TIM_HandleTypeDef* timer,
	ADC_HandleTypeDef* adc,
	OPAMP_HandleTypeDef* opamp,
	uint32_t timer_pwm_channel,
	uint32_t timer_adc_channel
) {

	// Assign parameters
	sensor->timer_handle = timer;
	sensor->timer_pwm_channel = timer_pwm_channel;
	sensor->timer_adc_channel = timer_adc_channel;
	sensor->adc_handle = adc;
	sensor->opamp_handle = opamp;

}

uint8_t chil_ec_measure(chil_ec_config* sensor, chil_ec_result* result, uint8_t averages) {

	result->ready = 0;

	// Start timer
	HAL_TIM_Base_Start(sensor->timer_handle);
	HAL_TIM_PWM_Init(sensor->timer_handle);
	HAL_TIM_PWM_Start(sensor->timer_handle, sensor->timer_pwm_channel);
	HAL_TIMEx_PWMN_Start(sensor->timer_handle, sensor->timer_pwm_channel);
	HAL_TIM_OC_Start(sensor->timer_handle, sensor->timer_adc_channel);

	// Enable op-amp
	HAL_OPAMP_Start(sensor->opamp_handle);

	// Enable ADC
	HAL_ADC_Start(sensor->adc_handle);

	// Perform averaging
	uint8_t average_count = 0;
	uint32_t ec_average = 0;
	while (average_count++ < averages) {
		HAL_ADC_PollForConversion(sensor->adc_handle, CHIL_EC_ADC_TIMEOUT);
		ec_average += (uint16_t) HAL_ADC_GetValue(sensor->adc_handle);
	}
	// Fix average
	if (average_count > 0) ec_average /= average_count;

	result->averages = average_count;
	result->result = ec_average;
	result->ready = 1;

	// Disable ADC
	HAL_ADC_Stop(sensor->adc_handle);
	// Disable op-amp
	HAL_OPAMP_Stop(sensor->adc_handle);

	// Stop timer
	HAL_TIM_Base_Stop(sensor->timer_handle);
	HAL_TIM_PWM_Stop(sensor->timer_handle, sensor->timer_pwm_channel);
	HAL_TIMEx_PWMN_Stop(sensor->timer_handle, sensor->timer_pwm_channel);
	HAL_TIM_OC_Stop(sensor->timer_handle, sensor->timer_adc_channel);

	return 0;

}
