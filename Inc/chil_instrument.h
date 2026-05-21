/*
 * Auth: J. D. Hawkins
 * Description: Header file containing definitions for the generic CHIL library
 */

#ifndef CHIL_INSTRUMENT_H
#define CHIL_INSTRUMENT_H

#include "chillib.h"
#ifdef STM32L433xx
// STM32 HAL includes
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_i2c_ex.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_hal_tim_ex.h"
#include "stm32l4xx_hal_opamp.h"
#include "stm32l4xx_hal_opamp_ex.h"
#include "stm32l4xx_hal_adc.h"
#include "stm32l4xx_hal_adc_ex.h"
#include "stm32l4xx_hal_rtc.h"

// Timeout period for each ADC read
#ifndef CHIL_EC_ADC_TIMEOUT
#define CHIL_EC_ADC_TIMEOUT 100
#endif

// Default timer prescaler value
#ifndef CHIL_EC_TIMER_PRESCALER
#define CHIL_EC_TIMER_PRESCALER (16-1) // For clock rate of 16 MHz
#endif

// Default tiemr period value
#ifndef CHIL_EC_TIMER_PERIOD
#define CHIL_EC_TIMER_PERIOD    1000   // for 16 MHz clock, 1 kHz clock
#endif

// Define the ADC channel that takes the single-ended OPAMP input
#ifndef CHIL_EC_ADC_CHANNEL
#define CHIL_EC_ADC_CHANNEL ADC_CHANNEL_8
#endif

// Define the PWR backup registers to store the transmit and deployment
// sequence numbers to
#ifndef CHIL_INSTRUMENT_BKUP_SEQUENCE
#define CHIL_INSTRUMENT_BKUP_SEQUENCE 2
#endif

#ifndef CHIL_INSTRUMENT_BKUP_DEPLOYMENT
#define CHIL_INSTRUMENT_BKUP_DEPLOYMENT 0
#endif

typedef enum {
	PACKET_FORMAT_LATEST           = 0,
	PACKET_FORMAT_PRE2026          = 1
} chil_packet_format_t;

// Define a system register to keep track of the backup registers
typedef struct chil_instrument_config {
	// Keep track of the HAL RTC handle
	RTC_HandleTypeDef* rtc;
	// Instrument variables
	uint8_t  transmit_sequence;
	uint32_t deployment_packets;
	uint32_t deployment_sleep_period_s;
	uint32_t long_term_sleep_period_s;
	uint32_t max_deployment_packets;
	uint8_t  invert_ec;
	// Packet format
	chil_packet_format_t packet_format;
} chil_instrument_config;

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
	uint16_t timer_prescaler;
	uint16_t timer_period;

	// ADC parameters
	ADC_HandleTypeDef* adc_handle;
	// OPAMP parameters
	OPAMP_HandleTypeDef* opamp_handle;

	uint8_t  averages;
	chil_ec_result* current_result;
	uint8_t  invert_ec;

} chil_ec_config;

/*****************************************************************************/
/* CHIL INSTRUMENT SYSTEM FUNCTIONS                                          */
/*****************************************************************************/
CHIL_STATUS chil_instrument_init(chil_instrument_config* config, RTC_HandleTypeDef* rtc);
void chil_instrument_sleep(chil_instrument_config* config);

/*****************************************************************************/
/* EC SENSOR                                                                 */
/*****************************************************************************/
CHIL_STATUS chil_ec_init(chil_ec_config* sensor, TIM_HandleTypeDef* timer, ADC_HandleTypeDef* adc, OPAMP_HandleTypeDef* opamp, uint32_t timer_pwn_channel, uint32_t timer_adc_channel, uint8_t invert_ec);
CHIL_STATUS __chil_ec_stm32_hal_setup(chil_ec_config* sensor);
CHIL_STATUS chil_ec_measure(chil_ec_config* sensor, chil_ec_result* result, uint8_t averages);

/*****************************************************************************/
/* BATTERY VOLTAGE SENSING                                                   */
/*****************************************************************************/
CHIL_STATUS chil_battery_voltage_init(ADC_HandleTypeDef* adc_handle);
CHIL_STATUS chil_battery_voltage_read(ADC_HandleTypeDef* adc_handle, uint16_t* battery_voltage);

#endif
#endif
