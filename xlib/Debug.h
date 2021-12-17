#pragma once

#ifdef __cplusplus
class BBitmap;

void WriteBitmapToFile(BBitmap* bitmap, const char* filename);

extern "C" {
#endif

void _x_trace(const char* trace, const char* func);
#ifndef NDEBUG
#	define UNIMPLEMENTED()	_x_trace("UNIMPLEMENTED", __PRETTY_FUNCTION__)
#else
#	define UNIMPLEMENTED()
#endif

#ifdef __cplusplus
}
#endif
