/*
 * packets.c
 *
 * Format, transmit and receive CHIL instrument packets
 *
 * Created on: May 7, 2026
 *      Author: J. D. Hawkins
 */
#include "packets.h"
#include "string.h"

void chil_instrument_packet_init(chil_packet* packet) {

	// Set all blocks to NULL pointers
	for (uint32_t bi = 0; bi < CHIL_FRAME_MAX_BLOCKS; bi++) {
		packet->blocks[bi] = NULL;
		packet->block_level[bi] = 0;
	}

	packet->block_index = 0; // reset block index;
//	packet-> = 0; // no blocks assigned yet, length = 0

	for (uint8_t bl = 0; bl < CHIL_BLOCK_LEVELS + 1; bl++) {
		packet->stack_block[bl] = NULL;
		packet->stack_level[bl] = 0;
	}
	packet->stack_index = 0;
	packet->stack_level[0] = CHIL_BLOCK_LEVELS + 1;
}

CHIL_STATUS append_block_to_packet(void* block, uint8_t block_level, chil_packet* packet) {

	// Fail if we provide an invalid block level
	if (block_level < 1 || block_level > CHIL_BLOCK_LEVELS) return 1;

	// We also can't add a block if the current level is greater than the new level by 2
	// unless we're at the beginning
	if (packet->stack_level[packet->stack_index] - block_level > 1 && packet->stack_index > 0) return 1;

	if (block_level > packet->stack_level[packet->stack_index]) {
		// Finish up with the current packet
		while (packet->stack_level[packet->stack_index] < block_level) {
			packet->stack_level[packet->stack_index] = 0;
			packet->stack_block[packet->stack_index] = NULL;
			packet->stack_index--;
		}
		// Assign the new block to the stack
		packet->stack_level[packet->stack_index] = block_level;
		packet->stack_block[packet->stack_index] = block;
		// And assign it to the packet
		packet->blocks[packet->block_index] = block;
		packet->block_level[packet->block_index++] = block_level;

	} else {

		if (block_level < packet->stack_level[packet->stack_index]) {
			// We're moving down to a new level
			packet->stack_index++;
			packet->stack_level[packet->stack_index] = block_level;
			packet->stack_block[packet->stack_index] = block;
		} else {
			// We're staying on the same level - don't change the stack
		}
		packet->blocks[packet->block_index] = block;
		packet->block_level[packet->block_index++] = block_level;

		// Work our way down the stack
		uint8_t stack_count = packet->stack_index - 1;
		uint32_t block_length = 0;
		// Find the current block length
		if (block_level < 3) {
			block_length += sizeof(block_L1L2_header);
			block_L1L2_header* header = (block_L1L2_header*)block;
			block_length += header->block_length;
		} else {
			block_length += sizeof(block_L3_header);
			block_L3_header* header = (block_L3_header*)block;
			block_length += header->block_length;
		}

		// At each step, add the current block length to the parent block.
		while (stack_count-- > 0) {
			if (packet->stack_level[packet->stack_index] < 3) {
				block_L1L2_header* header = (block_L1L2_header*)(packet->stack_block[packet->stack_index]);
				header->block_length += block_length;
			} else {
				block_L3_header* header = (block_L3_header*)(packet->stack_block[packet->stack_index]);
				header->block_length += block_length;
			}
		}

	}

	return CHIL_OK;

}

CHIL_STATUS write_packet_to_mbus_buffer(chil_packet* packet, uint8_t* buffer) {

	// buffer[0] is our running packet length
	uint8_t offset = 2;
	buffer[1] = 0xAA;
	buffer[0] = 1;
	for (uint8_t bi = 0; bi < packet->block_index; bi++) {
		uint8_t length = 0;
		if (packet->block_level[bi] < 3) {
			block_L1L2_header header = *(block_L1L2_header*)packet->blocks[bi];
			length = sizeof(header) + header.block_length;
		} else {
			block_L3_header header = *(block_L3_header*)packet->blocks[bi];
			length = sizeof(header) + header.block_length;
		}
		buffer[0] += length;
		memcpy(buffer + offset, packet->blocks[bi], length);
		offset += length;
	}

	return CHIL_ERROR; // fail
}
