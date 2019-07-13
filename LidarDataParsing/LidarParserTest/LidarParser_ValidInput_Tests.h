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

	//============================================================================================================================================================================================================
	// Valid Packet Data
	//
	// Data bytes:
	//   0: <distance 7:0>
	//   1: <invalid data flag> <strength warning flag> <distance 13:8>
	//   2: <signal strength 7:0>
	//   3: <signal strength 15:8>
	//
	//                                    start, index, speed (lsb), speed (msb), <-------Data 0------->, <-------Data 1------->, <--------Data 2------>, <--------Data 3------>, checksum (lsb), checksum (msb)
	//============================================================================================================================================================================================================
	std::deque<uint8_t> valid_packet_0 = { 0xfa,  0xa0,        0x27,        0x4b, 0x97, 0x01, 0xbb, 0x01, 0x97, 0x01, 0xb4, 0x00, 0x98, 0x01, 0x53, 0x00, 0x99, 0x01, 0x93, 0x00,           0x4e,          0x28 };
	std::deque<uint8_t> valid_packet_1 = { 0xfa,  0xa1,        0x35,        0x4b, 0x9a, 0x01, 0xb9, 0x01, 0x9a, 0x01, 0x8b, 0x02, 0x9a, 0x01, 0x71, 0x02, 0x9b, 0x01, 0x8b, 0x02,           0xa6,          0x5f };
	std::deque<uint8_t> valid_packet_2 = { 0xfa,  0xa2,        0x35,        0x4b, 0x9c, 0x01, 0x90, 0x02, 0x9d, 0x01, 0x89, 0x02, 0x9e, 0x01, 0x72, 0x02, 0xa0, 0x01, 0x8a, 0x02,           0xd8,          0x16 };
	std::deque<uint8_t> valid_packet_3 = { 0xfa,  0xa3,        0x35,        0x4b, 0xa1, 0x01, 0x85, 0x02, 0xa2, 0x01, 0x7d, 0x02, 0xa4, 0x01, 0x76, 0x02, 0xa7, 0x01, 0x6a, 0x02,           0xaa,          0x16 };
	std::deque<uint8_t> valid_packet_4 = { 0xfa,  0xa4,        0x35,        0x4b, 0xa8, 0x01, 0x5c, 0x02, 0xaa, 0x01, 0x78, 0x02, 0xad, 0x01, 0x6f, 0x02, 0xaf, 0x01, 0x4b, 0x02,           0xbb,          0x10 };
	std::deque<uint8_t> valid_packet_5 = { 0xfa,  0xa5,        0x35,        0x4b, 0xb2, 0x01, 0x57, 0x02, 0xb4, 0x01, 0x61, 0x02, 0xa8, 0x01, 0xc3, 0x00, 0x97, 0x01, 0x9a, 0x00,           0x96,          0x0b };
	std::deque<uint8_t> valid_packet_6 = { 0xfa,  0xa6,        0x35,        0x4b, 0x35, 0x80, 0x00, 0x00, 0x79, 0x01, 0xab, 0x00, 0x21, 0x80, 0xab, 0x00, 0x61, 0x01, 0xf9, 0x00,           0x7a,          0x08 };
	std::deque<uint8_t> valid_packet_7 = { 0xfa,  0xa7,        0x5b,        0x4b, 0x55, 0x01, 0x26, 0x01, 0x4a, 0x01, 0x44, 0x01, 0x41, 0x01, 0x41, 0x01, 0x37, 0x01, 0x45, 0x01,           0x78,          0x16 };
	std::deque<uint8_t> valid_packet_8 = { 0xfa,  0xa8,        0x5b,        0x4b, 0x2f, 0x01, 0x77, 0x01, 0x26, 0x01, 0x8b, 0x01, 0x1f, 0x01, 0x9b, 0x01, 0x17, 0x01, 0xa4, 0x01,           0x23,          0x18 };

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
	MockLidarInputStream_AddBytes(valid_packet_0);
	LidarParser_Parse();
	EXPECT_EQ(4, message_buffer.GetSize());
}

//==============================================================================
// Verify that 8 measurements are transferred to the measurement buffer when
// two valid packets are received.
//==============================================================================
TEST_F(LidarParser_ValidInput, TwoValidPackets)
{
	MockLidarInputStream_AddBytes(valid_packet_0);
	MockLidarInputStream_AddBytes(valid_packet_1);
	LidarParser_Parse();
	EXPECT_EQ(8, message_buffer.GetSize());
}

//==============================================================================
// Verify that the correct distances are passed into the measurement buffer
// when a single valid packet is received.
//==============================================================================
TEST_F(LidarParser_ValidInput, OneValidPacket_CorrectDistances)
{
	MockLidarInputStream_AddBytes(valid_packet_0);
	LidarParser_Parse();
	EXPECT_EQ(0x0197, MockLidarMeasurementBuffer_GetDistance(0));
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
	MockLidarInputStream_AddBytes(valid_packet_0);
	MockLidarInputStream_AddBytes(valid_packet_1);
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
	MockLidarInputStream_AddBytes(valid_packet_0);
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
	MockLidarInputStream_AddBytes({ 0xAA, 0xBB, 0xCC, 0xDD });
	MockLidarInputStream_AddBytes(valid_packet_0);
	LidarParser_Parse();
	EXPECT_EQ(0x0197, MockLidarMeasurementBuffer_GetDistance(0));
	EXPECT_EQ(0x0197, MockLidarMeasurementBuffer_GetDistance(1));
	EXPECT_EQ(0x0198, MockLidarMeasurementBuffer_GetDistance(2));
	EXPECT_EQ(0x0199, MockLidarMeasurementBuffer_GetDistance(3));
}

//==============================================================================
// Verify that the parser correctly extracts multiple valid packets even when
// it is surrounded by invalid bytes.
//==============================================================================
TEST_F(LidarParser_ValidInput, MultipleValidPacketsSurroundedByTrashBytes)
{
	MockLidarInputStream_AddBytes({ 1, 2, 3, 4, 5 });
	MockLidarInputStream_AddBytes(valid_packet_0);
	MockLidarInputStream_AddBytes({ 1, 2, 3, 4, 5 });
	MockLidarInputStream_AddBytes(valid_packet_1);
	MockLidarInputStream_AddBytes({ 0xA0, 2, 0xA0, 4, 5 });
	MockLidarInputStream_AddBytes(valid_packet_2);
	MockLidarInputStream_AddBytes({ 1, 2, 3, 4, 5 });

	LidarParser_Parse();

	// verify correct number of measurements were parsed
	EXPECT_EQ(12, message_buffer.GetSize());
}
