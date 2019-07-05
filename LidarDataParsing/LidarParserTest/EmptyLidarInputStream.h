#pragma once

void EmptyLidarInputStream_Reset()
{
	/* intentionally empty */
}

uint8_t EmptyLidarInputStream_GetByte ()
{
	assert(false); // violated input stream contract
	return 0;
}

bool EmptyLidarInputStream_IsEmpty ()
{
	return true;
}