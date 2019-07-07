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
	return 0;
}

int Packet_getIndex2()
{
	return 0;
}

int Packet_getIndex3()
{
	return 0;
}

int Packet_getIndex4()
{
	return 0;
}

//==============================================================================
// Extracting distances from packet
//==============================================================================

int Packet_getDistance1()
{
	return 0;
}

int Packet_getDistance2()
{
	return 0;
}

int Packet_getDistance3()
{
	return 0;
}

int Packet_getDistance4()
{
	return 0;
}
