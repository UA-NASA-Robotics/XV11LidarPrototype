#ifndef LIDAR_MEASUREMENT_BUFFER_H
#define LIDAR_MEASUREMENT_BUFFER_H

typedef struct {

	/// Method to construct a measurement and add it to the buffer.
	void (*AddMeasurement) (uint16_t index, uint16_t distance);

	/// Method to query the number of measurements in the buffer.
	int (*GetSize) ();

} LidarMeasurementBuffer_i;

#endif // LIDAR_MEASUREMENT_BUFFER_H
