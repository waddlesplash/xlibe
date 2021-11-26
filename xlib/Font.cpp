#include <interface/Font.h>
#include <cstdlib>

#include "XInnerWindow.h"
#include "GC.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlcint.h>
}

// TODO: Support fonts besides the default!

int
XTextWidth(XFontStruct* font_struct, const char *string, int count)
{
	return be_plain_font->StringWidth(string, count);
}

int
XTextWidth16(XFontStruct *font_struct, const XChar2b *string, int count)
{
	return XTextWidth(font_struct, (const char*)string, count);
}

XFontStruct*
XLoadQueryFont(Display *display, const char *name)
{
	XFontStruct* font = new XFontStruct;
	return font;
}

int
XFreeFont(Display *dpy, XFontStruct *fs)
{
	delete fs;
	return 0;
}

XFontSet
XCreateFontSet(Display* display, _Xconst char* base_font_name_list,
	char*** missing_charset_list, int* missing_charset_count, char** def_string)
{
	return new _XOC;
}

Font
XLoadFont(Display *display, const char *name)
{
	return 0;
}

int
XSetFont(Display *display, GC gc, Font font)
{
	return 0;
}
