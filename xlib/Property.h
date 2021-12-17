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
	ret.nitems = length < 0 ? strlen((const char*)ret.value) : length;
	return ret;
}