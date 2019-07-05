#pragma once
#include <vector>
#include <tuple>

static std::vector<std::tuple<uint16_t, uint16_t>> measurements;

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