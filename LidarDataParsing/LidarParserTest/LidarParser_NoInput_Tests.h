#pragma once

#include "gtest/gtest.h"
#include "LidarParser.h"

// interfaces
#include "LidarInputStream.h"
#include "LidarMeasurementBuffer.h"

// mock implementations
#include "EmptyLidarInputStream.h"
#include "MockLidarMeasurementBuffer.h"

class LidarParser_NoInput : public testing::Test
{
protected:
	LidarInputStream_i input_stream;
	LidarMeasurementBuffer_i message_buffer;

	void SetUp ()
	{
		// reset the mock modules
		EmptyLidarInputStream_Reset();
		MockLidarMeasurementBuffer_Reset();

		// configure test input stream
		input_stream.GetByte = EmptyLidarInputStream_GetByte;
		input_stream.IsEmpty = EmptyLidarInputStream_IsEmpty;

		// configure test message buffer
		message_buffer.AddMeasurement = MockLidarMeasurementBuffer_AddMeasurement;
		message_buffer.GetSize        = MockLidarMeasurementBuffer_GetSize;

		// initialize the parser
		LidarParser_Init(&input_stream, &message_buffer);
	}
};

/// If there is no input, no valid messages are parsed.
TEST_F (LidarParser_NoInput, ParsingEmptyStreamYieldsZeroMessages)
{
	// verify parse detects 0 valid measurements
	EXPECT_EQ(0, LidarParser_Parse());

	// verify size of measurement buffer is 0
	EXPECT_EQ(0, message_buffer.GetSize());
}

