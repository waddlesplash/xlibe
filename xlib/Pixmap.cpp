/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <X11/Xlib.h>
#include <interface/Bitmap.h>

#include "Drawing.h"
#include "Drawables.h"
#include "Debug.h"
#include "Bits.h"

extern "C" XPixmapFormatValues*
XListPixmapFormats(Display* dpy, int* count_return)
{
	// We theoretically support more formats than this.
	static const XPixmapFormatValues formats[] = {
		{.depth = 32, .bits_per_pixel = 32, .scanline_pad = 32},
		{.depth = 24, .bits_per_pixel = 24, .scanline_pad = 32},
		{.depth = 16, .bits_per_pixel = 16, .scanline_pad = 16},
	};
	static const int count = B_COUNT_OF(formats);

	XPixmapFormatValues* ret = (XPixmapFormatValues*)malloc(sizeof(XPixmapFormatValues) * count);
	memcpy(ret, formats, sizeof(XPixmapFormatValues) * count);
	*count_return = count;
	return ret;
}

extern "C" Pixmap
XCreatePixmap(Display* display, Drawable d,
	unsigned int width, unsigned int height, unsigned int depth)
{
	BRect rect(brect_from_xrect(make_xrect(0, 0, width, height)));
	XPixmap* pixmap = new XPixmap(display, rect, depth);
	return pixmap->id();
}

extern "C" Pixmap
XCreateBitmapFromData(Display* display, Drawable d,
	const char* data, unsigned int width, unsigned int height)
{
	BRect rect(brect_from_xrect(make_xrect(0, 0, width, height)));
	XPixmap* pixmap = new XPixmap(display, rect, 1);
	pixmap->offscreen()->ImportBits(data, width * height, ROUNDUP(width, 8), 0, B_GRAY1);
	return pixmap->id();
}

extern "C" Pixmap
XCreatePixmapFromBitmapData(Display* display, Drawable d,
	char* data, unsigned int width, unsigned int height,
	unsigned long fg, unsigned long bg, unsigned int depth)
{
	if (fg == 1 && bg == 0 && depth == 1)
		return XCreateBitmapFromData(display, d, data, width, height);

	debugger("Unimplemented");
	return 0;
}

extern "C" int
XFreePixmap(Display *display, Pixmap pxm)
{
	XPixmap* pixmap = Drawables::get_pixmap(pxm);
	if (!pxm)
		return BadPixmap;

	delete pixmap;
	return Success;
}
