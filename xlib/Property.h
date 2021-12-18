/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <cstring>

extern "C" {
#include <X11/Xutil.h>
}

static inline XTextProperty
make_text_property(Atom type, int format, const void* data, int length = -1, bool copy = false)
{
	XTextProperty ret;
	ret.encoding = type;
	ret.format = format;
	ret.value = copy ? (unsigned char*)strdup((const char*)data) : (unsigned char*)data;
	ret.nitems = length < 0 ? (ret.value ? strlen((const char*)ret.value) : 0) : length;
	return ret;
}
