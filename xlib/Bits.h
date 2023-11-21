/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ROUNDDOWN(a, b)	(((a) / (b)) * (b))
#define ROUNDUP(a, b)	ROUNDDOWN((a) + (b) - 1, b)

static uint8_t
REVERSE_BITS(uint8_t b)
{
	// http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64BitsDiv
	return ((b * 0x0202020202ULL) & 0x010884422010ULL) % 0x3ff;
}

#ifdef __cplusplus
} // extern "C"
#endif
