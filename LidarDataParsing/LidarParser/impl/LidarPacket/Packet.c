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

uint8_t getIndexByte()
{
	return packet_bytes[1];
}


//==============================================================================
// Calculates checksum based on first 20 bytes (including start byte).
//==============================================================================
uint16_t calculateChecksum()
{
	// combine the bytes into 16 bit values
	uint16_t combined[10];
	for (int i = 0; i < 10; ++i)
		combined[i] = packet_bytes[2 * i] + ((uint16_t)packet_bytes[2 * i + 1] << 8);

	// compute the checksum
	uint32_t checksum = 0;
	for (int i = 0; i < 10; ++i)
		checksum = (checksum << 1) + combined[i];
	checksum = (checksum & 0x7FFF) + (checksum >> 15);

	// truncate the result to 15 bits
	return checksum & 0x7FFFF;
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
	// validate the index
	if (!isValidIndex(getIndexByte()))
		return false;

	// validate the checksum
	uint16_t packet_checksum = packet_bytes[20] + ((uint16_t)packet_bytes[21] << 8);
	uint16_t calculated_checksum = calculateChecksum();
	if (packet_checksum != calculated_checksum)
		return false;

	return true;
}

//==============================================================================
// Extracting indices from the packet
//==============================================================================

#define NORMALIZE_INDEX(i, j) ((((i) - LidarPacket_MIN_INDEX) << 2) + (j))

int Packet_getIndex1()
{
	return NORMALIZE_INDEX(getIndexByte(), 0);
}

int Packet_getIndex2()
{
	return NORMALIZE_INDEX(getIndexByte(), 1);
}

int Packet_getIndex3()
{
	return NORMALIZE_INDEX(getIndexByte(), 2);
}

int Packet_getIndex4()
{
	return NORMALIZE_INDEX(getIndexByte(), 3);
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
