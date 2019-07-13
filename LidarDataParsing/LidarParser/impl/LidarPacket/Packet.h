#ifndef PACKET_H
#define PACKET_H

#include <stdbool.h>

#define LidarPacket_START_BYTE 0xFA
#define LidarPacket_NUM_BYTES_PER_PACKET 22

void Packet_reset();
void Packet_add(uint8_t);
bool Packet_isValid();

int Packet_getIndex1();
int Packet_getIndex2();
int Packet_getIndex3();
int Packet_getIndex4();

int Packet_getDistance1();
int Packet_getDistance2();
int Packet_getDistance3();
int Packet_getDistance4();

#endif // PACKET_H