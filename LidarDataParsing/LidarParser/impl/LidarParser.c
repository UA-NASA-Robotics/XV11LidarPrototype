#include "LidarParser.h"
#include "LidarInputStream.h"
#include "LidarMeasurementBuffer.h"
#include "LidarPacket/Packet.h"
#include "Buffer.h"

// interfaces
static LidarInputStream_i * s_stream;
static LidarMeasurementBuffer_i * s_buffer;

// parser's finite states
typedef enum
{
	ResettingParser,

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

	// pause parsing (until next call to parse function)
	StopParsing
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
	parser.stage = ResettingParser;
	parser.continue_parsing = true;

	// initialize buffer of raw bytes
	Buffer_init(&parser.buffer);
	parser.index = 0;

	// reset the packet
	Packet_reset();
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
	return Buffer_get(&parser.buffer, parser.index++);
}

bool isValidStartByte(uint8_t byte)
{
	return byte == START_BYTE;
}

bool isValidIndexByte(uint8_t byte)
{
	return byte >= MIN_INDEX && byte <= MAX_INDEX;
}

//==============================================================================
// State Machine Handlers
//==============================================================================

void Handler_ResettingParser()
{
	// empty the packet
	Packet_reset();

	parser.stage = GettingStartByte;
	parser.num_data_bytes = 0;
	parser.index = 0;
}

void Handler_GettingStartByte()
{
	parser.stage = allBytesScanned() ? StopParsing : ValidateStartByte;
}

void Handler_ValidateStartByte()
{
	uint8_t byte = nextByte();

	// handle invalid start byte
	if (!isValidStartByte(byte))
	{
		Buffer_pop(&parser.buffer);
		parser.stage = ResettingParser;
		return;
	}

	Packet_add(byte);
	parser.stage = GettingIndexByte;
}

void Handler_GettingIndexByte()
{
	parser.stage = allBytesScanned() ? StopParsing : ValidatingIndexByte;
}

void Handler_ValidateIndexByte()
{
	uint8_t byte = nextByte();

	// handle invalid index byte
	if (!isValidIndexByte(byte))
	{
		Buffer_pop(&parser.buffer);
		parser.stage = ResettingParser;
		return;
	}

	Packet_add(byte);
	parser.stage = GettingSpeedLSB;
}

void Handler_GettingSpeedLSB()
{
	parser.stage = allBytesScanned() ? StopParsing : StoringSpeedLSB;
}

void Handler_StoringSpeedLSB()
{
	Packet_add(nextByte());
	parser.stage = GettingSpeedMSB;
}

void Handler_GettingSpeedMSB()
{
	parser.stage = allBytesScanned() ? StopParsing : StoringSpeedMSB;
}

void Handler_StoringSpeedMSB()
{
	Packet_add(nextByte());
	parser.stage = GetDataByte;
}

void Handler_GetDataByte()
{
	parser.stage = allBytesScanned() ? StopParsing : StoreDataByte;
}

void Handler_StoreDataByte()
{
	Packet_add(nextByte());
	++parser.num_data_bytes;
	parser.stage = parser.num_data_bytes < NUM_DATA_BYTES_PER_PACKET ? GetDataByte : GetChecksumByte_LSB;
}

void Handler_GetChecksumByteLSB()
{
	parser.stage = allBytesScanned() ? StopParsing : StoreChecksumByte_LSB;
}

void Handler_StoreChecksumByteLSB()
{
	Packet_add(nextByte());
	parser.stage = GetChecksumByte_MSB;
}

void Handler_GetChecksumByteMSB()
{
	parser.stage = allBytesScanned() ? StopParsing : StoreChecksumByte_MSB;
}

void Handler_StoreChecksumByteMSB()
{
	Packet_add(nextByte());
	parser.stage = ValidatingChecksum;
}

// TODO: Rework this to validate the packet as a whole (not just the checksum)
void Handler_ValidatingChecksum()
{
	// handle invalid packet
	if (!Packet_isValid())
	{
		Buffer_pop(&parser.buffer);
		parser.stage = ResettingParser;
		return;
	}

	parser.stage = AddingMeasurementToBuffer;
}

void Handler_AddingMeasurementToBuffer()
{
	s_buffer->AddMeasurement(Packet_getIndex1(), Packet_getDistance1());
	s_buffer->AddMeasurement(Packet_getIndex2(), Packet_getDistance2());
	s_buffer->AddMeasurement(Packet_getIndex3(), Packet_getDistance3());
	s_buffer->AddMeasurement(Packet_getIndex4(), Packet_getDistance4());

	// remove bytes from buffer
	for (int i = 0; i < NUM_TOTAL_BYTES_PER_PACKET; ++i)
		Buffer_pop(&parser.buffer);

	// start over
	parser.stage = ResettingParser;
}

void Handler_StopParsing()
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
		case ResettingParser: Handler_ResettingParser(); break;
		case GettingStartByte: Handler_GettingStartByte(); break;
		case ValidateStartByte: Handler_ValidateStartByte(); break;
		case GettingIndexByte: Handler_GettingIndexByte(); break;
		case ValidatingIndexByte: Handler_ValidateIndexByte(); break;
		case GettingSpeedLSB: Handler_GettingSpeedLSB(); break;
		case StoringSpeedLSB: Handler_StoringSpeedLSB(); break;
		case GettingSpeedMSB: Handler_GettingSpeedMSB(); break;
		case StoringSpeedMSB: Handler_StoringSpeedMSB(); break;
		case GetDataByte: Handler_GetDataByte(); break;
		case StoreDataByte: Handler_StoreDataByte(); break;
		case GetChecksumByte_LSB: Handler_GetChecksumByteLSB(); break;
		case StoreChecksumByte_LSB: Handler_StoreChecksumByteLSB(); break;
		case GetChecksumByte_MSB: Handler_GetChecksumByteMSB(); break;
		case StoreChecksumByte_MSB: Handler_StoreChecksumByteMSB(); break;
		case ValidatingChecksum: Handler_ValidatingChecksum(); break;
		case AddingMeasurementToBuffer: Handler_AddingMeasurementToBuffer(); break;
		case StopParsing: Handler_StopParsing(); break;
		}
	}
}

