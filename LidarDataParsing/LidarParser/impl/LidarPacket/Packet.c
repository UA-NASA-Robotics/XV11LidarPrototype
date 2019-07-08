#include "Packet.h"

#include <stdint.h>

#define NUM_BYTES_PER_PACKET 22

static uint8_t packet_bytes[NUM_BYTES_PER_PACKET];
static int index;

void Packet_reset()
{
	index = 0;
}

void Packet_add(uint8_t byte)
{
	packet_bytes[index++] = byte;
}

bool Packet_isValid()
{
	return true;
}

//==============================================================================
// Extracting indices from the packet
//==============================================================================

int Packet_getIndex1()
{
	return packet_bytes[1] << 2 + 0;
}

int Packet_getIndex2()
{
	return packet_bytes[1] << 2 + 1;
}

int Packet_getIndex3()
{
	return packet_bytes[1] << 2 + 2;
}

int Packet_getIndex4()
{
	return packet_bytes[1] << 2 + 3;
}

//==============================================================================
// Extracting distances from packet
//==============================================================================

#define DISTANCE_MASK ~(1 << 14 | 1 << 15)

int Packet_getDistance1()
{
	uint16_t lsb = packet_bytes[4];
	uint16_t msb = packet_bytes[5];
	uint16_t distance = (lsb + (msb << 8)) & DISTANCE_MASK;
	return distance;
}

int Packet_getDistance2()
{
	uint16_t lsb = packet_bytes[8];
	uint16_t msb = packet_bytes[9];
	uint16_t distance = (lsb + (msb << 8)) & DISTANCE_MASK;
	return distance;
}

int Packet_getDistance3()
{
	uint16_t lsb = packet_bytes[12];
	uint16_t msb = packet_bytes[13];
	uint16_t distance = (lsb + (msb << 8)) & DISTANCE_MASK;
	return distance;
}

int Packet_getDistance4()
{
	uint16_t lsb = packet_bytes[16];
	uint16_t msb = packet_bytes[17];
	uint16_t distance = (lsb + (msb << 8)) & DISTANCE_MASK;
	return distance;
}
