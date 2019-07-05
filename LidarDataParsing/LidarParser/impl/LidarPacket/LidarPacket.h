#ifndef LIDAR_PACKET_H
#define LIDAR_PACKET_H

#include <stdint.h>
#include <stdbool.h>

//==============================================================================
// Types
//==============================================================================

#define NUM_BYTES_PER_PACKET 22

typedef struct
{
	uint8_t bytes [NUM_BYTES_PER_PACKET];
}
LidarPacket_t;

typedef enum
{
	PACKET_START_BYTE,
	INDEX_BYTE,
	SPEED_LSB_BYTE,
	SPEED_MSB_BYTE,
	DATA_0_BYTE_0, DATA_0_BYTE_1, DATA_0_BYTE_2, DATA_0_BYTE_3,
	DATA_1_BYTE_0, DATA_1_BYTE_1, DATA_1_BYTE_2, DATA_1_BYTE_3,
	DATA_2_BYTE_0, DATA_2_BYTE_1, DATA_2_BYTE_2, DATA_2_BYTE_3,
	DATA_3_BYTE_0, DATA_3_BYTE_1, DATA_3_BYTE_2, DATA_3_BYTE_3,
	CHECKSUM_LSB_BYTE,
	CHECKSUM_MSB_BYTE
}
LidarPacketByteID_t;

//==============================================================================
// Public Methods
//==============================================================================

/// Initializes the packet so that all of its bytes are 0.
void LidarPacket_Init(LidarPacket_t *);

/// Returns true iff the given packet is valid.
bool LidarPacket_IsValid(LidarPacket_t *);

/// Sets a particular byte within the packet
void LidarPacket_Populate(LidarPacket_t *, LidarPacketByteID_t, uint8_t);

#endif // !LIDAR_PACKET_H
