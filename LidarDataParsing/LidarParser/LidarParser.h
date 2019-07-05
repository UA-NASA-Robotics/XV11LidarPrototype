#ifndef LIDAR_PARSER_H
#define LIDAR_PARSER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "LidarInputStream.h"
#include "LidarMeasurementBuffer.h"

///=============================================================================
/// TODO: add documentation
///=============================================================================
void LidarParser_Init (LidarInputStream_i *, LidarMeasurementBuffer_i *);

///=============================================================================
/// Processes the bytes from the input stream, placing any valid lidar
/// measurements into the buffer.
///
/// Preconditions:
///  - Module has been initialized.
///
/// Returns:
///  - Returns the number of valid messages transferred to the buffer.
///=============================================================================
int LidarParser_Parse ();

#ifdef __cplusplus
}
#endif
#endif  // !LIDAR_PARSER_H
