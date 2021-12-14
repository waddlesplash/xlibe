#pragma once

#include <interface/GraphicsDefs.h>

extern "C" {
#include <X11/Xlib.h>
}

static inline unsigned long
_x_rgb_to_pixel(rgb_color color)
{
	long result = long(color.red) | (long(color.green) << 8)
		| (long(color.blue) << 16)/* | (long(color.alpha) << 24)*/;
	return result;
}

static inline rgb_color
_x_pixel_to_rgb(unsigned long color, bool alpha = false)
{
	rgb_color rgb;
	rgb.set_to(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
	if (alpha)
		rgb.alpha = (color >> 24) & 0xFF;
	return rgb;
}

color_space _x_color_space(Visual* v, int bits_per_pixel);
