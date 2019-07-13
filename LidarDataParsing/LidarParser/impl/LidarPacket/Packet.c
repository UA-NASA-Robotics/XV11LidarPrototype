#include "Packet.h"

#include <stdint.h>

//==============================================================================
// constants
//==============================================================================

#define LidarPacket_MIN_INDEX 0xA0
#define LidarPacket_MAX_INDEX 0xF9

//==============================================================================
// packet state
//==============================================================================

static uint8_t packet_bytes[LidarPacket_NUM_BYTES_PER_PACKET];
static int index;

//==============================================================================
// helper methods
//==============================================================================

bool isValidIndex(uint8_t byte)
{
	return (byte >= LidarPacket_MIN_INDEX) && (byte <= LidarPacket_MAX_INDEX);
}

//==============================================================================
// public methods
//==============================================================================

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
	bool valid = true;

	// validate the index
	if (!isValidIndex(packet_bytes[1]))
		valid = false;

	return valid;
}

//==============================================================================
// Extracting indices from the packet
//==============================================================================

int Packet_getIndex1()
{
	return ((packet_bytes[1] - 0xA0) << 2) + 0;
}

int Packet_getIndex2()
{
	return ((packet_bytes[1] - 0xA0) << 2) + 1;
}

int Packet_getIndex3()
{
	return ((packet_bytes[1] - 0xA0) << 2) + 2;
}

int Packet_getIndex4()
{
	return ((packet_bytes[1] - 0xA0) << 2) + 3;
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
