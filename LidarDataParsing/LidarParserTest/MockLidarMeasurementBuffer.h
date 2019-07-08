#pragma once
#include <vector>
#include <tuple>

static std::vector<std::tuple<uint16_t, uint16_t>> measurements;

//==============================================================================
// Interface Overrides
//==============================================================================

void MockLidarMeasurementBuffer_Reset()
{
	measurements.clear();
}

void MockLidarMeasurementBuffer_AddMeasurement (uint16_t index, uint16_t distance)
{
	measurements.emplace_back(index, distance);
}

int MockLidarMeasurementBuffer_GetSize ()
{
	return static_cast<int>(measurements.size());
}

//==============================================================================
// Test Helpers
//==============================================================================

uint16_t MockLidarMeasurementBuffer_GetDistance(int i)
{
	return std::get<1>(measurements.at(i));
}

uint16_t MockLidarMeasurementBuffer_GetIndex(int i)
{
	return std::get<0>(measurements.at(i));
}
