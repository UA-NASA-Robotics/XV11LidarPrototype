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
	GettingStartByte,
	GettingIndexByte,
	GettingPayloadBytes,
	ValidatingPacket,
	AddingMeasurementToBuffer,
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
	parser.index = 0;
}

void Handler_GettingStartByte()
{
	// handle not enough bytes
	if (allBytesScanned())
	{
		parser.stage = StopParsing;
		return;
	}

	// get next byte
	uint8_t byte = nextByte();

	// handle invalid start byte
	if (!isValidStartByte(byte))
	{
		Buffer_pop(&parser.buffer);
		parser.stage = ResettingParser;
		return;
	}

	// add the start byte
	Packet_add(byte);
	parser.stage = GettingIndexByte;
}

void Handler_GettingIndexByte()
{
	// handle not enough bytes
	if (allBytesScanned())
	{
		parser.stage = StopParsing;
		return;
	}

	// get next byte
	uint8_t byte = nextByte();

	// handle invalid index byte
	if (!isValidIndexByte(byte))
	{
		Buffer_pop(&parser.buffer);
		parser.stage = ResettingParser;
		return;
	}

	Packet_add(byte);
	parser.stage = GettingPayloadBytes;
}

void Handler_GetPayloadBytes()
{
	// 2 speed bytes, 16 data bytes, 2 checksum bytes
	const int NUM_PAYLOAD_BYTES = 20;

	for (int i = 0; i < NUM_DATA_BYTES_PER_PACKET; ++i)
	{
		if (allBytesScanned())
		{
			parser.stage = StopParsing;
			return;
		}
		Packet_add(nextByte());
	}
	parser.stage = ValidatingPacket;
}

void Handler_ValidatingPacket()
{
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
	// transfer as many bytes as possible from stream to parsing buffer
	while (!Buffer_full(&parser.buffer) && !s_stream->IsEmpty())
	{
		uint8_t byte = s_stream->GetByte();
		Buffer_push(&parser.buffer, byte);
	}

	while (parser.continue_parsing)
	{
		switch (parser.stage)
		{
		case ResettingParser:           Handler_ResettingParser(); break;
		case GettingStartByte:          Handler_GettingStartByte(); break;
		case GettingIndexByte:          Handler_GettingIndexByte(); break;
		case GettingPayloadBytes:       Handler_GetPayloadBytes(); break;
		case ValidatingPacket:          Handler_ValidatingPacket(); break;
		case AddingMeasurementToBuffer: Handler_AddingMeasurementToBuffer(); break;
		case StopParsing:               Handler_StopParsing(); break;
		}
	}
}

