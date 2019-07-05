#pragma once

#include <deque>
#include <algorithm>

static std::deque<uint8_t> s_bytes;

void MockLidarInputStream_Reset()
{
	s_bytes.clear();
}

void MockLidarInputStream_AddBytes(std::deque<uint8_t> bytes)
{
	std::copy(bytes.begin(), bytes.end(), std::back_inserter(s_bytes));
}

uint8_t MockLidarInputStream_GetByte()
{
	uint8_t byte = s_bytes.front();
	s_bytes.pop_front();
	return byte;
}

bool MockLidarInputStream_IsEmpty()
{
	return s_bytes.empty();
}