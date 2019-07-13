#pragma once

#include "gtest/gtest.h"
#include "LidarParser.h"

// interfaces
#include "LidarInputStream.h"
#include "LidarMeasurementBuffer.h"

// mock implementations
#include "MockLidarInputStream.h"
#include "MockLidarMeasurementBuffer.h"

class LidarParser_InvalidInput : public testing::Test
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
// Verifies that if a packet contains an index less than minimum, the
// measurements are NOT added to the measurement buffer.
//==============================================================================
TEST_F(LidarParser_InvalidInput, IndexTooSmall_MeasurementsNotAddedToBuffer)
{
	MockLidarInputStream_AddBytes({
		0xFA,                   // start byte
		0xA0 - 1,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0x4e, 0x28,		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(0, message_buffer.GetSize());
}

//==============================================================================
// Verifies that if a packet contains an index greater than maximum, the
// measurements are NOT added to the measurement buffer.
//==============================================================================
TEST_F(LidarParser_InvalidInput, IndexTooLarge_MeasurementsNotAddedToBuffer)
{
	MockLidarInputStream_AddBytes({
		0xFA,                   // start byte
		0xA0 + 90,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0x4e, 0x28,		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(0, message_buffer.GetSize());
}

//==============================================================================
// Verifies that if a packet contains an incorrect checksum, measurements
// are NOT added to the measurement buffer.
//==============================================================================
TEST_F(LidarParser_InvalidInput, IncorrectChecksum_MeasurementsNotAddedToBuffer)
{
	MockLidarInputStream_AddBytes({
		0xFA,                   // start byte
		0xA0 + 0,		        // index (note: indices are offset by 0xA0)
		0x27, 0x4b,		        // speed bytes (lsb, msb)
		0x97, 0x01, 0xbb, 0x01,	// Data 1
		0x97, 0x01, 0xb4, 0x00,	// Data 2
		0x98, 0x01, 0x53, 0x00, // Data 3
		0x99, 0x01, 0x93, 0x00, // Data 4
		0xAA, 0xBB,		        // checksum bytes (lsb, msb)
	});
	LidarParser_Parse();
	EXPECT_EQ(0, message_buffer.GetSize());
}
