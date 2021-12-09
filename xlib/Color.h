#ifndef COLOR_H
#define COLOR_H

#include <interface/GraphicsDefs.h>

extern "C" {
#include <X11/Xlib.h>
}

rgb_color create_rgb(unsigned long color);

color_space x_color_space(Visual* v, int bits_per_pixel);

#endif
