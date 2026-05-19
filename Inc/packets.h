/*
 * packets.h
 *
 * Header file for formating and receiving CHIL packets
 *
 *  Created on: May 7, 2026
 *      Author: sgljh3
 */

#ifndef CHIL_PACKETS_H
#define CHIL_PACKETS_H

#include "chillib.h"

// Maximum number of blocks per MBus frame to transmit
// assuming these all have a block length of zero. The
// total length of the frame must not exceed 246 bytes.
#define CHIL_FRAME_MAX_BLOCKS 123

// Number of level of blocks that exist (encapsulating
// sub-blocks
#define CHIL_BLOCK_LEVELS 3

// Define a default control layer value
#define CHIL_DEFAULT_CI_FIELD 0xAC

#include "keller_nldpressure.h"
#include "lsm303.h"
#include "tmp117.h"

typedef struct __attribute__((packed)) block_L1_A_LSM303 {
	uint8_t block_identifier;
	uint8_t block_length;
	int16_t magnetometer_x;
	int16_t magnetometer_y;
	int16_t magnetometer_z;
	int16_t accelerometer_x;
	int16_t accelerometer_y;
	int16_t accelerometer_z;
} block_L1_A_LSM303;

typedef struct __attribute__((packed)) block_L1_B_BMA400 {
	uint8_t block_identifier;
	uint8_t block_length;
	int16_t accelerometer_x;
	int16_t accelerometer_y;
	int16_t accelerometer_z;
} block_L1_B_BMA400;

typedef struct __attribute__((packed)) block_L1_C_CHIL {
	uint8_t 	block_identifier;
	uint8_t 	block_length;
	uint8_t 	sequence_number;
	uint16_t 	voltage_battery;
	uint16_t 	conductivity;
	int16_t 	temperature_tmp117;
} block_L1_C_CHIL;

typedef struct __attribute__((packed)) block_L1_E_CHIL {
	uint8_t 	block_identifier;
	uint8_t 	block_length;
	float 		pressure_ms5607;
	float		temperature_ms5607;
	uint16_t	temperature_sht30;
	uint16_t 	relative_humidity_sht30;
} block_L1_E_CHIL;

typedef struct __attribute__((packed)) block_L1_K_Pressure {
	uint8_t 	block_identifier;
	uint8_t 	block_length;
	uint16_t 	pressure;
	uint16_t	temperature;
	uint8_t		date_and_type;
	float		scaling_pressure_min;
	float		scaling_pressure_max;
} block_L1_K_Pressure;

typedef struct __attribute__((packed)) block_L1_T_TILT05 {
	uint8_t block_identifier;
	uint8_t block_length;
	int16_t	accelerometer_x;
	int16_t	accelerometer_y;
	int16_t	accelerometer_z;
	int16_t	pitch;
	int16_t	roll;
} block_L1_T_TILT05;

typedef struct __attribute__((packed)) block_L1_V_Voltage {
	uint8_t 	block_identifier;
	uint8_t 	block_length;
	uint16_t 	voltage_battery;
	uint16_t	voltage_channel1_shunt;
	uint16_t	voltage_channel1_bus;
	uint16_t	voltage_channel2_shunt;
	uint16_t	voltage_channel2_bus;
	uint16_t	voltage_channel3_shunt;
	uint16_t	voltage_channel3_bus;
} block_L1_V_Voltage;

typedef struct __attribute__((packed)) block_L2_M_MBus {
	uint8_t 	block_identifier;
	uint8_t 	block_length;
	uint8_t		channel;
	uint8_t		c_field_mbus;
	uint16_t	m_field_mbus;
	uint32_t	uid_mbus;
	uint8_t		version_mbus;
	uint8_t		device_mbus;
	uint8_t		ci_field_mbus;
	// Payload goes here... need to treat carefully
	uint8_t		rssi_mbus;
} block_L2_M_MBus;

typedef struct __attribute__((packed)) block_L2_R_Receiver {
	uint8_t 	block_identifier;
	uint8_t 	block_length;
} block_L2_R_Receiver;

// Used for both D and H which are the same at the moment, with the exception
// of the block_identifier....
typedef struct __attribute__((packed)) block_L3_Context {
	uint8_t 	block_identifier;
	uint16_t 	block_length;
	uint32_t	receiver_id;
	uint32_t	timestamp;
	uint8_t		sequence_number;
} block_L3_Context;

typedef struct __attribute__((packed)) block_L1L2_header {
	uint8_t 	block_identifier;
	uint8_t 	block_length;
} block_L1L2_header;

typedef struct __attribute__((packed)) block_L3_header {
	uint8_t 	block_identifier;
	uint16_t 	block_length;
} block_L3_header;

typedef struct chil_packet {
	void* blocks[CHIL_FRAME_MAX_BLOCKS];
	uint8_t block_level[CHIL_FRAME_MAX_BLOCKS];
	uint32_t block_index;
	// Define a stack to store the index of superior blocks
	void* stack_block[CHIL_BLOCK_LEVELS+1];
	uint8_t stack_level[CHIL_BLOCK_LEVELS+1];
	uint8_t stack_index;
} chil_packet ;
/*****************************************************************************/
/* Function definitions */
/*****************************************************************************/

void chil_instrument_packet_init(chil_packet* packet);
CHIL_STATUS append_block_to_packet(void* block, uint8_t block_level, chil_packet* packet);
CHIL_STATUS write_packet_to_mbus_buffer(chil_packet* packet, uint8_t* buffer);

#endif
