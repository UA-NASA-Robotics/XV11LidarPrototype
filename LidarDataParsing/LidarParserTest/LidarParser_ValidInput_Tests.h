#pragma once

#include "gtest/gtest.h"
#include "LidarParser.h"

// interfaces
#include "LidarInputStream.h"
#include "LidarMeasurementBuffer.h"

// mock implementations
#include "MockLidarInputStream.h"
#include "MockLidarMeasurementBuffer.h"

class LidarParser_ValidInput : public testing::Test
{
protected:
	LidarInputStream_i input_stream;
	LidarMeasurementBuffer_i message_buffer;

	void SetUp()
	{
		// reset the mock modules
		MockLidarInputStream_Reset();
		MockLidarMeasurementBuffer_Reset();

		// configure test input stream
		input_stream.GetByte = MockLidarInputStream_GetByte;
		input_stream.IsEmpty = MockLidarInputStream_IsEmpty;

		// configure test message buffer
		message_buffer.AddMeasurement = MockLidarMeasurementBuffer_AddMeasurement;
		message_buffer.GetSize = MockLidarMeasurementBuffer_GetSize;

		// initialize the parser
		LidarParser_Init(&input_stream, &message_buffer);
	}
};

//==============================================================================
// Verify that 4 measurements are transferred to the measurement buffer when
// a single valid packet is received.
//==============================================================================
TEST_F(LidarParser_ValidInput, OneValidPacket)
{
	MockLidarInputStream_AddBytes({
		0xFA,                   // start byte
		0xA0 + 0,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0x4e, 0x28		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(4, message_buffer.GetSize());
}

//==============================================================================
// Verify that 8 measurements are transferred to the measurement buffer when
// two valid packets are received.
//==============================================================================
TEST_F(LidarParser_ValidInput, TwoValidPackets)
{
	MockLidarInputStream_AddBytes({
		0xFA,                   // start byte
		0xA0 + 0,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0x4e, 0x28,		        // checksum bytes (lsb, msb)

		0xFA,                   // start byte
		0xA0 + 1,		        // index (note: indices are offset by 0xA0)
		0x35, 0x4b,		        // speed bytes (lsb, msb)
		0x9a, 0x01, 0xb9, 0x01, // Data 1
		0x9a, 0x01, 0x8b, 0x02, // Data 2
		0x9a, 0x01, 0x71, 0x02, // Data 3
		0x9b, 0x01, 0x8b, 0x02, // Data 4
		0xa6, 0x5f,		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(8, message_buffer.GetSize());
}

//==============================================================================
// Verify that the correct distances are passed into the measurement buffer
// when a single valid packet is received.
//==============================================================================
TEST_F(LidarParser_ValidInput, OneValidPacket_CorrectDistances)
{
	MockLidarInputStream_AddBytes({
		0xFA,                   // start byte
		0xA0 + 0,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0x4e, 0x28,		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(0x0197,  MockLidarMeasurementBuffer_GetDistance(0));
	EXPECT_EQ(0x0197, MockLidarMeasurementBuffer_GetDistance(1));
	EXPECT_EQ(0x0198, MockLidarMeasurementBuffer_GetDistance(2));
	EXPECT_EQ(0x0199, MockLidarMeasurementBuffer_GetDistance(3));
}

//==============================================================================
// Verify that the correct distances are passed into the measurement buffer
// when two valid packets are received
//==============================================================================
TEST_F(LidarParser_ValidInput, TwoValidPackets_CorrectDistances)
{
	MockLidarInputStream_AddBytes({
		0xFA,                   // start byte
		0xA0 + 0,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0x4e, 0x28,		        // checksum bytes (lsb, msb)

		0xFA,                   // start byte
		0xA0 + 1,		        // index (note: indices are offset by 0xA0)
		0x35, 0x4b,		        // speed bytes (lsb, msb)
		0x9a, 0x01, 0xb9, 0x01, // Data 1
		0x9a, 0x01, 0x8b, 0x02, // Data 2
		0x9a, 0x01, 0x71, 0x02, // Data 3
		0x9b, 0x01, 0x8b, 0x02, // Data 4
		0xa6, 0x5f,		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(0x0197, MockLidarMeasurementBuffer_GetDistance(0));
	EXPECT_EQ(0x0197, MockLidarMeasurementBuffer_GetDistance(1));
	EXPECT_EQ(0x0198, MockLidarMeasurementBuffer_GetDistance(2));
	EXPECT_EQ(0x0199, MockLidarMeasurementBuffer_GetDistance(3));
	EXPECT_EQ(0x019a, MockLidarMeasurementBuffer_GetDistance(4));
	EXPECT_EQ(0x019a, MockLidarMeasurementBuffer_GetDistance(5));
	EXPECT_EQ(0x019a, MockLidarMeasurementBuffer_GetDistance(6));
	EXPECT_EQ(0x019b, MockLidarMeasurementBuffer_GetDistance(7));
}

//==============================================================================
// Verify that the correct indices are passed into the measurement buffer
// when a single valid packet is received.
//==============================================================================
TEST_F(LidarParser_ValidInput, OneValidPacket_CorrectIndices)
{
	MockLidarInputStream_AddBytes({
		0xFA,                   // start byte
		0xA0 + 0,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0x4e, 0x28,		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(0, MockLidarMeasurementBuffer_GetIndex(0));
	EXPECT_EQ(1, MockLidarMeasurementBuffer_GetIndex(1));
	EXPECT_EQ(2, MockLidarMeasurementBuffer_GetIndex(2));
	EXPECT_EQ(3, MockLidarMeasurementBuffer_GetIndex(3));
}

//==============================================================================
// Verify that the parser correctly extracts a valid packet even when it is
// preceded by invalid bytes (bytes that should be trashed).
//==============================================================================
TEST_F(LidarParser_ValidInput, OneValidPacket_AfterTrashBytes)
{
	MockLidarInputStream_AddBytes({
		// trash bytes
		0xAA, 0xBB, 0xCC, 0xDD,

		// valid packet
		0xFA,                   // start byte
		0xA0 + 0,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0x4e, 0x28		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(0x0197,  MockLidarMeasurementBuffer_GetDistance(0));
	EXPECT_EQ(0x0197, MockLidarMeasurementBuffer_GetDistance(1));
	EXPECT_EQ(0x0198, MockLidarMeasurementBuffer_GetDistance(2));
	EXPECT_EQ(0x0199, MockLidarMeasurementBuffer_GetDistance(3));
}