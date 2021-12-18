/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <interface/Rect.h>

extern "C" {
#include <X11/Xlib.h>
}

static inline XRectangle
make_xrect(int x, int y, int w, int h)
{
	XRectangle r;
	r.x = x;
	r.y = y;
	r.width = w;
	r.height = h;
	return r;
}

static inline XRectangle
xrect_from_brect(const BRect& rect)
{
	return make_xrect(rect.LeftTop().x, rect.LeftTop().y,
		rect.IntegerWidth() + 1, rect.IntegerHeight() + 1);
}

static inline BRect
brect_from_xrect(const XRectangle& xrect)
{
	return BRect(xrect.x, xrect.y,
		(xrect.x + xrect.width) - 1, (xrect.y + xrect.height) - 1);
}
