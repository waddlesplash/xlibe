#ifndef FONT_LIST_H
#define FONT_LIST_H

#include <interface/Font.h>

extern "C" {
#include <X11/Xlib.h>
}

void init_font();
void finalize_font();

BFont* bfont_from_xfontstruct(XFontStruct* font_struct);

#endif
