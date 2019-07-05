#ifndef LIDAR_INPUT_STREAM_H
#define LIDAR_INPUT_STREAM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {

	/// Method to retrieve a byte from the input stream.
	uint8_t (*GetByte) ();

	/// Method to query whether the input stream is empty
	bool (*IsEmpty) ();

} LidarInputStream_i;

#endif // LIDAR_INPUT_STREAM_H
