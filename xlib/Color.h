/*
 * Copyright 2003, Shibukawa Yoshiki. All rights reserved.
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <interface/GraphicsDefs.h>

extern "C" {
#include <X11/Xlib.h>
}

static inline void
_x_get_rgb_masks(Visual* v)
{
	v->red_mask     = 255 << 16;
	v->green_mask   = 255 << 8;
	v->blue_mask    = 255;
}

static inline unsigned long
_x_rgb_to_pixel(rgb_color color)
{
	long result = long(color.blue) | (long(color.green) << 8)
		| (long(color.red) << 16) | (long(color.alpha) << 24);
	return result;
}

static inline rgb_color
_x_pixel_to_rgb(unsigned long color, bool alpha = false)
{
	rgb_color rgb;
	rgb.set_to((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
	if (alpha)
		rgb.alpha = (color >> 24) & 0xFF;
	return rgb;
}

color_space _x_color_space_for(Visual* v, int bits_per_pixel);
