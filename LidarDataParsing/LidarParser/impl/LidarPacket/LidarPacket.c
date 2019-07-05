#include "LidarPacket.h"

void LidarPacket_Init(LidarPacket_t * packet)
{
	for (int i = 0; i < NUM_BYTES_PER_PACKET; ++i)
		packet->bytes[i] = 0;
}

bool LidarPacket_IsValid(LidarPacket_t * packet)
{
	return false;
}

void LidarPacket_Populate(LidarPacket_t * packet, LidarPacketByteID_t byte_id, uint8_t byte_value)
{
	packet->bytes[byte_id] = byte_value;
}
