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
		0xF0, 0x4A,		        // speed bytes (lsb, msb)
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 1
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 2
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 3
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 4
		0xAA, 0xBB		        // checksum bytes (lsb, msb)
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
		0xF0, 0x4A,		        // speed bytes (lsb, msb)
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 1
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 2
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 3
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 4
		0xAA, 0xBB,		        // checksum bytes (lsb, msb)

		0xFA,                   // start byte
		0xA0 + 1,		        // index (note: indices are offset by 0xA0)
		0xF0, 0x4A,		        // speed bytes (lsb, msb)
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 1
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 2
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 3
		0xFF, 0xFF, 0xFF, 0xFF,	// Data 4
		0xAA, 0xBB,		        // checksum bytes (lsb, msb)
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
		0xF0, 0x4A,		        // speed bytes (lsb, msb)
		5,  0, 0xFF, 0xFF,	    // Data 1
		10, 0, 0xFF, 0xFF,		// Data 2
		15, 0, 0xFF, 0xFF,		// Data 3
		20, 0, 0xFF, 0xFF,		// Data 4
		0xAA, 0xBB		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(5,  MockLidarMeasurementBuffer_GetDistance(0));
	EXPECT_EQ(10, MockLidarMeasurementBuffer_GetDistance(1));
	EXPECT_EQ(15, MockLidarMeasurementBuffer_GetDistance(2));
	EXPECT_EQ(20, MockLidarMeasurementBuffer_GetDistance(3));
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
		0xF0, 0x4A,		        // speed bytes (lsb, msb)
		0xAA, 0x0A, 0xFF, 0xFF,	// Data 1
		0xBB, 0x0B, 0xFF, 0xFF,	// Data 2
		0xCC, 0x0C, 0xFF, 0xFF,	// Data 3
		0xDD, 0x0D, 0xFF, 0xFF,	// Data 4
		0xAA, 0xBB,		        // checksum bytes (lsb, msb)

		0xFA,                   // start byte
		0xA0 + 1,		        // index (note: indices are offset by 0xA0)
		0xF0, 0x4A,		        // speed bytes (lsb, msb)
		0xAB, 0x0B, 0xFF, 0xFF,	// Data 1
		0xAC, 0x0C, 0xFF, 0xFF,	// Data 2
		0xAD, 0x0D, 0xFF, 0xFF,	// Data 3
		0xAE, 0x0E, 0xFF, 0xFF,	// Data 4
		0xAA, 0xBB,		        // checksum bytes (lsb, msb)

	});
	LidarParser_Parse();
	EXPECT_EQ(0x0AAA, MockLidarMeasurementBuffer_GetDistance(0));
	EXPECT_EQ(0x0BBB, MockLidarMeasurementBuffer_GetDistance(1));
	EXPECT_EQ(0x0CCC, MockLidarMeasurementBuffer_GetDistance(2));
	EXPECT_EQ(0x0DDD, MockLidarMeasurementBuffer_GetDistance(3));
	EXPECT_EQ(0x0BAB, MockLidarMeasurementBuffer_GetDistance(4));
	EXPECT_EQ(0x0CAC, MockLidarMeasurementBuffer_GetDistance(5));
	EXPECT_EQ(0x0DAD, MockLidarMeasurementBuffer_GetDistance(6));
	EXPECT_EQ(0x0EAE, MockLidarMeasurementBuffer_GetDistance(7));
}
