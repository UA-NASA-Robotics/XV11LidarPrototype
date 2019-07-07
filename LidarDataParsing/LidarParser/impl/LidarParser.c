#include "LidarParser.h"
#include "LidarInputStream.h"
#include "LidarMeasurementBuffer.h"
#include "LidarPacket/LidarPacket.h"
#include "Buffer.h"

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
	ResettingParser,

	// pause parsing (until next call to parse function)
	PauseParsing
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
	// finite state of parsing system
	ParsingStage_t stage;

	// number of data bytes that have been parsed in current packet
	int num_data_bytes;

	// buffer of bytes in current packet
	LidarPacket_t packet;

	// flag to indicate whether the FSM loop should continue
	bool continue_parsing;

	// buffer containing raw bytes to be parsed
	Buffer_t buffer;
	size_t index;
}
parser;

void LidarParser_Init (LidarInputStream_i * stream, LidarMeasurementBuffer_i * buffer)
{
	s_stream = stream;
	s_buffer = buffer;
	
	// initialize finite state machine
	parser.stage = GettingStartByte;
	parser.num_data_bytes = 0;
	parser.continue_parsing = true;

	// initialize buffer of raw bytes
	Buffer_init(&parser.buffer);
	parser.index = 0;
}

//==============================================================================
// Helper Functions
//==============================================================================

bool allBytesScanned()
{
	return parser.index >= Buffer_size(&parser.buffer);
}

uint8_t nextByte()
{
	return Buffer_get(&parser.buffer, parser.index);
}

//==============================================================================
// State Machine Handlers
//==============================================================================

void Handler_GettingStartByte()
{
	parser.stage = allBytesScanned() ? PauseParsing : ValidateStartByte;
}

void Handler_ValidateStartByte()
{
	uint8_t byte = nextByte();
	LidarPacket_Populate(&parser.packet, PACKET_START_BYTE, byte);
	parser.stage = byte == START_BYTE ? GettingIndexByte : GettingStartByte;
}

void Handler_GettingIndexByte()
{
	parser.stage = allBytesScanned() ? PauseParsing : ValidatingIndexByte;
}

void Handler_ValidateIndexByte()
{
	uint8_t byte = nextByte();
	if (byte >= MIN_INDEX && byte <= MAX_INDEX)
	{
		LidarPacket_Populate(&parser.packet, INDEX_BYTE, byte);
		parser.stage = GettingSpeedLSB;
		return;
	}

	parser.stage = GettingStartByte;
}

void Handler_GettingSpeedLSB()
{
	parser.stage = allBytesScanned() ? PauseParsing : StoringSpeedLSB;
}

void Handler_StoringSpeedLSB()
{
	uint8_t byte = nextByte();
	LidarPacket_Populate(&parser.packet, SPEED_LSB_BYTE, byte);
	parser.stage = GettingSpeedMSB;
}

void Handler_GettingSpeedMSB()
{
	parser.stage = allBytesScanned() ? PauseParsing : StoringSpeedMSB;
}

void Handler_StoringSpeedMSB()
{
	uint8_t byte = nextByte();
	LidarPacket_Populate(&parser.packet, SPEED_MSB_BYTE, byte);
	parser.stage = PrepareToGetDataBytes;
}

void Handler_PrepareToGetDataBytes()
{
	parser.num_data_bytes = 0;
	parser.stage = GetDataByte;
}

void Handler_GetDataByte()
{
	parser.stage = allBytesScanned() ? PauseParsing : StoreDataByte;
}

void Handler_StoreDataByte()
{
	uint8_t byte = nextByte();
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
}

void Handler_GetChecksumByteLSB()
{
	parser.stage = allBytesScanned() ? PauseParsing : StoreChecksumByte_LSB;
}

void Handler_StoreChecksumByteLSB()
{
	LidarPacket_Populate(&parser.packet, CHECKSUM_LSB_BYTE, nextByte());
	parser.stage = GetChecksumByte_MSB;
}

void Handler_GetChecksumByteMSB()
{
	parser.stage = allBytesScanned() ? PauseParsing : StoreChecksumByte_MSB;
}

void Handler_StoreChecksumByteMSB()
{
	LidarPacket_Populate(&parser.packet, CHECKSUM_MSB_BYTE, nextByte());
	parser.stage = ValidatingChecksum;
}

void Handler_ValidatingChecksum()
{
	bool valid_checksum = true;
	parser.stage = valid_checksum ? AddingMeasurementToBuffer : GettingStartByte;
}

void Handler_AddingMeasurementToBuffer()
{
	for (int i = 0; i < 4; ++i)
	{
		// for now, just adding correct number of placeholders
		s_buffer->AddMeasurement(0, 0);
	}

	// start over
	parser.stage = ResettingParser;
}

void Handler_ResettingParser()
{
	parser.stage = GettingStartByte;
}

void Handler_PauseParsing()
{
	parser.continue_parsing = false;
}

//==============================================================================
// State Machine Loop
//==============================================================================

void LidarParser_Parse()
{
	// transfer as many bytes as possible from UART buffer to parsing buffer
	while (!Buffer_full(&parser.buffer) && !s_stream->IsEmpty())
	{
		uint8_t byte = s_stream->GetByte();
		Buffer_push(&parser.buffer, byte);
	}

	// FSM loop
	while (parser.continue_parsing)
	{
		switch (parser.stage)
		{
		case GettingStartByte: Handler_GettingStartByte(); break;
		case ValidateStartByte: Handler_ValidateStartByte(); break;
		case GettingIndexByte: Handler_GettingIndexByte(); break;
		case ValidatingIndexByte: Handler_ValidateIndexByte(); break;
		case GettingSpeedLSB: Handler_GettingSpeedLSB(); break;
		case StoringSpeedLSB: Handler_StoringSpeedLSB(); break;
		case GettingSpeedMSB: Handler_GettingSpeedMSB(); break;
		case StoringSpeedMSB: Handler_StoringSpeedMSB(); break;
		case PrepareToGetDataBytes: Handler_PrepareToGetDataBytes(); break;
		case GetDataByte: Handler_GetDataByte(); break;
		case StoreDataByte: Handler_StoreDataByte(); break;
		case GetChecksumByte_LSB: Handler_GetChecksumByteLSB(); break;
		case StoreChecksumByte_LSB: Handler_StoreChecksumByteLSB(); break;
		case GetChecksumByte_MSB: Handler_GetChecksumByteMSB(); break;
		case StoreChecksumByte_MSB: Handler_StoreChecksumByteMSB(); break;
		case ValidatingChecksum: Handler_ValidatingChecksum(); break;
		case AddingMeasurementToBuffer: Handler_AddingMeasurementToBuffer(); break;
		case ResettingParser: Handler_ResettingParser(); break;
		case PauseParsing: Handler_PauseParsing(); break;
		}
	}
}

