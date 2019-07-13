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
	GettingPayloadBytes,
	ValidatingPacket,
	AddingMeasurementToBuffer,
	StopParsing
}
ParsingStage_t;

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
	return byte == LidarPacket_START_BYTE;
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


//=============================================================================
// Handler_GettingStartByte
//
// The objective of this function is to remove any/all bytes from the parsing
// buffer until a valid "start byte" is encountered.  If and when a valid
// start byte is encountered, it is added to the current packet and the
// parser is advanced to the next stage.
//=============================================================================
void Handler_GettingStartByte()
{
	while (true)
	{
		// stop loop if no more bytes available in parsing buffer
		if (allBytesScanned())
		{
			parser.stage = StopParsing;
			return;
		}

		// get the next byte
		uint8_t byte = nextByte();

		// if byte is a valid start byte, add it to the packet and stop loop
		if (isValidStartByte(byte))
		{
			Packet_add(byte);
			parser.stage = GettingPayloadBytes;
			return;
		}

		// trash byte if not a valid start byte
		Buffer_pop(&parser.buffer);
	}
}

void Handler_GetPayloadBytes()
{
	for (int i = 1; i < LidarPacket_NUM_BYTES_PER_PACKET; ++i)
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
	// for invalid packets, remove first byte from buffer and restart parser
	if (!Packet_isValid())
	{
		Buffer_pop(&parser.buffer);
		parser.stage = ResettingParser;
		return;
	}

	// for valid packets, add their payload to the measurement buffer
	parser.stage = AddingMeasurementToBuffer;
}

void Handler_AddingMeasurementToBuffer()
{
	s_buffer->AddMeasurement(Packet_getIndex1(), Packet_getDistance1());
	s_buffer->AddMeasurement(Packet_getIndex2(), Packet_getDistance2());
	s_buffer->AddMeasurement(Packet_getIndex3(), Packet_getDistance3());
	s_buffer->AddMeasurement(Packet_getIndex4(), Packet_getDistance4());

	// remove bytes from buffer
	for (int i = 0; i < LidarPacket_NUM_BYTES_PER_PACKET; ++i)
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
		case GettingPayloadBytes:       Handler_GetPayloadBytes(); break;
		case ValidatingPacket:          Handler_ValidatingPacket(); break;
		case AddingMeasurementToBuffer: Handler_AddingMeasurementToBuffer(); break;
		case StopParsing:               Handler_StopParsing(); break;
		}
	}
}

