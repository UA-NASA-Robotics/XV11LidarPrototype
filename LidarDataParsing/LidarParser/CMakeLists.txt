﻿cmake_minimum_required (VERSION 3.8)

add_library(LidarParser
	./impl/LidarParser.c
	./impl/LidarPacket/Packet.c
	./impl/Buffer.c
)
target_include_directories(LidarParser
	PUBLIC
		.
)
