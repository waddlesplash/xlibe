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

#ifdef __cplusplus
} // extern "C"
#endif
