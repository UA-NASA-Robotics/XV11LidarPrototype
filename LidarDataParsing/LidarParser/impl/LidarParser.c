#include "LidarParser.h"
#include "LidarInputStream.h"
#include "LidarMeasurementBuffer.h"
#include "LidarPacket/LidarPacket.h"

// interfaces
static LidarInputStream_i * s_stream;
static LidarMeasurementBuffer_i * s_buffer;

// parser's finite states
typedef enum
{
	// start byte
	GettingStartByte,
	ValidateStartByte,

	// index byte
	GettingIndexByte,
	ValidatingIndexByte,

	// speed bytes
	GettingSpeedLSB,
	StoringSpeedLSB,
	GettingSpeedMSB,
	StoringSpeedMSB,

	// data bytes
	PrepareToGetDataBytes,
	GetDataByte,
	StoreDataByte,

	// checksum bytes
	GetChecksumByte_LSB,
	StoreChecksumByte_LSB,
	GetChecksumByte_MSB,
	StoreChecksumByte_MSB,
	ValidatingChecksum,

	// transfer bytes to measurement buffer
	AddingMeasurementToBuffer,

	// reset the state of the parser
	ResettingParser
}
ParsingStage_t;

// constants
#define START_BYTE 0xFA
#define NUM_TOTAL_BYTES_PER_PACKET 22
#define NUM_DATA_BYTES_PER_PACKET 16
#define MIN_INDEX 0xA0
#define MAX_INDEX 0XF9

static struct
{
	/// finite state of parsing system
	ParsingStage_t stage;

	/// number of data bytes that have been parsed in current packet
	int num_data_bytes;

	/// buffer of bytes in current packet
	LidarPacket_t packet;
}
parser;

void LidarParser_Init (LidarInputStream_i * stream, LidarMeasurementBuffer_i * buffer)
{
	s_stream = stream;
	s_buffer = buffer;
	
	// initialize finite state machine
	parser.stage = GettingStartByte;
	parser.num_data_bytes = 0;
}

int LidarParser_Parse()
{
	bool continue_parsing = true;
	int measurements_transferred = 0;
	while (continue_parsing)
	{
		switch (parser.stage)
		{
			case GettingStartByte:
			{
				if (s_stream->IsEmpty())
					return measurements_transferred;
				parser.stage = ValidateStartByte;
				break;
			}

			case ValidateStartByte:
			{
				uint8_t byte = s_stream->GetByte();
				LidarPacket_Populate(&parser.packet, PACKET_START_BYTE, byte);
				parser.stage = byte == START_BYTE ? GettingIndexByte : GettingStartByte;
				break;
			}

			case GettingIndexByte:
			{
				if (s_stream->IsEmpty())
					return measurements_transferred;
				parser.stage = ValidatingIndexByte;
				break;
			}

			// ensure that the index byte is within the correct range
			// (inclusive range: [0xA0, 0xF9])
			case ValidatingIndexByte:
			{
				uint8_t byte = s_stream->GetByte();
				if (byte >= MIN_INDEX && byte <= MAX_INDEX)
				{
					LidarPacket_Populate(&parser.packet, INDEX_BYTE, byte);
					parser.stage = GettingSpeedLSB;
				}
				else
				{
					// invalid index, so return to initial state
					parser.stage = GettingStartByte;
				}
				break;
			}

			case GettingSpeedLSB:
			{
				if (s_stream->IsEmpty())
					return measurements_transferred;
				parser.stage = StoringSpeedLSB;
				break;
			}

			case StoringSpeedLSB:
			{
				uint8_t byte = s_stream->GetByte();
				LidarPacket_Populate(&parser.packet, SPEED_LSB_BYTE, byte);
				parser.stage = GettingSpeedMSB;
				break;
			}

			case GettingSpeedMSB:
			{
				if (s_stream->IsEmpty())
					return measurements_transferred;
				parser.stage = StoringSpeedMSB;
				break;
			}

			case StoringSpeedMSB:
			{
				uint8_t byte = s_stream->GetByte();
				LidarPacket_Populate(&parser.packet, SPEED_MSB_BYTE, byte);
				parser.stage = PrepareToGetDataBytes;
				break;
			}

			// set the number of bytes collected so far to 0
			case PrepareToGetDataBytes:
			{
				parser.num_data_bytes = 0;
				parser.stage = GetDataByte;
				break;
			}

			case GetDataByte:
			{
				if (s_stream->IsEmpty())
					return measurements_transferred;
				parser.stage = StoreDataByte;
				break;
			}

			case StoreDataByte:
			{
				uint8_t byte = s_stream->GetByte();
				switch (parser.num_data_bytes)
				{
					case 0:  LidarPacket_Populate(&parser.packet, DATA_0_BYTE_0, byte); break;
					case 1:  LidarPacket_Populate(&parser.packet, DATA_0_BYTE_1, byte); break;
					case 2:  LidarPacket_Populate(&parser.packet, DATA_0_BYTE_2, byte); break;
					case 3:  LidarPacket_Populate(&parser.packet, DATA_0_BYTE_3, byte); break;
					case 4:  LidarPacket_Populate(&parser.packet, DATA_1_BYTE_0, byte); break;
					case 5:  LidarPacket_Populate(&parser.packet, DATA_1_BYTE_1, byte); break;
					case 6:  LidarPacket_Populate(&parser.packet, DATA_1_BYTE_2, byte); break;
					case 7:  LidarPacket_Populate(&parser.packet, DATA_1_BYTE_3, byte); break;
					case 8:  LidarPacket_Populate(&parser.packet, DATA_2_BYTE_0, byte); break;
					case 9:  LidarPacket_Populate(&parser.packet, DATA_2_BYTE_1, byte); break;
					case 10: LidarPacket_Populate(&parser.packet, DATA_2_BYTE_2, byte); break;
					case 11: LidarPacket_Populate(&parser.packet, DATA_2_BYTE_3, byte); break;
					case 12: LidarPacket_Populate(&parser.packet, DATA_3_BYTE_0, byte); break;
					case 13: LidarPacket_Populate(&parser.packet, DATA_3_BYTE_1, byte); break;
					case 14: LidarPacket_Populate(&parser.packet, DATA_3_BYTE_2, byte); break;
					case 15: LidarPacket_Populate(&parser.packet, DATA_3_BYTE_3, byte); break;
				}
				++parser.num_data_bytes;
				parser.stage = parser.num_data_bytes < NUM_DATA_BYTES_PER_PACKET ? GetDataByte : GetChecksumByte_LSB;
				break;
			}

			case GetChecksumByte_LSB:
			{
				if (s_stream->IsEmpty())
					return measurements_transferred;
				parser.stage = StoreChecksumByte_LSB;
				break;
			}

			case StoreChecksumByte_LSB:
			{
				uint8_t byte = s_stream->GetByte();
				LidarPacket_Populate(&parser.packet, CHECKSUM_LSB_BYTE, byte);
				// TODO: Store the LSB of checksum
				parser.stage = GetChecksumByte_MSB;
				break;
			}

			case GetChecksumByte_MSB:
			{
				if (s_stream->IsEmpty())
					return measurements_transferred;
				parser.stage = StoreChecksumByte_MSB;
				break;
			}

			case StoreChecksumByte_MSB:
			{
				uint8_t byte = s_stream->GetByte();
				LidarPacket_Populate(&parser.packet, CHECKSUM_MSB_BYTE, byte);
				parser.stage = ValidatingChecksum;
				break;
			}

			case ValidatingChecksum:
			{
				// TODO: Validate the checksum (or the packet as a whole...)
				bool valid_checksum = true;
				parser.stage = valid_checksum ? AddingMeasurementToBuffer : GettingStartByte;
				break;
			}

			case AddingMeasurementToBuffer:
			{
				// TODO: Add measurements to the buffer
				for (int i = 0; i < 4; ++i)
				{
					// for now, just adding correct number of placeholders
					s_buffer->AddMeasurement(0, 0);
					++measurements_transferred;
				}

				// start over
				parser.stage = ResettingParser;
				break;
			}

			case ResettingParser:
			{
				parser.stage = GettingStartByte;
				break;
			}
		}
	}
	return measurements_transferred;
}

