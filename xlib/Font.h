#ifndef FONT_LIST_H
#define FONT_LIST_H

#include <interface/Font.h>

extern "C" {
#include <X11/Xlib.h>
}

void _x_init_font();
void _x_finalize_font();

BFont bfont_from_font(Font font);

#endif
