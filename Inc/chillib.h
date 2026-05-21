/*
 * Auth: J. D. Hawkins
 * Description: Header file containing definitions for the generic CHIL library
 */

#ifndef CHILLIB_H
#define CHILLIB_H

typedef enum {
	CHIL_OK = 0,
	CHIL_ERROR = 1,
	CHIL_TIMEOUT = 2
} CHIL_STATUS;

#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

#ifdef ARDUINO
#include "Arduino.h"
#else
#ifdef STM32L433xx
#include "stm32l4xx.h"
#endif
#endif

#endif
