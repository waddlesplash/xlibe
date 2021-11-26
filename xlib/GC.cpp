#include "XInnerWindow.h"
#include "Color.h"

#include "FontList.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

GC XCreateGC(Display *display, Window window,
		unsigned long mask, XGCValues *gc_values)
{
	GC gc = new _XGC;
	gc->values.fill_style = FillSolid;
	gc->values.line_style = LineSolid;
	gc->values.line_width = 0;
	gc->values.cap_style = CapButt;
	gc->values.join_style = JoinMiter;
	gc->values.font = 0;
	gc->dirty = True;
	return gc;
}

int XFreeGC(Display* display, GC gc)
{
	delete gc;
}

extern "C" int XSetForeground(Display *display, GC gc, unsigned long color) {
	gc->values.foreground = color;
	gc->dirty = True;
	return 0;
}

extern "C" int XSetBackground(Display *display, GC gc, unsigned long color) {
	gc->values.background = color;
	gc->dirty = True;
	return 0;
}

int XSetGraphicsExposures(Display *display, GC gc, Bool graphics_exposures) {
	gc->values.graphics_exposures = graphics_exposures;
	return 0;
}

extern "C" int XSetLineAttributes(Display* display, GC gc,
		unsigned int line_width, int line_style, int cap_style, int join_style)
{
	gc->values.line_width = line_width;
	gc->values.line_style = line_style;
	gc->values.cap_style = cap_style;
	gc->values.join_style = join_style;
	gc->dirty = True;
	return 0;
}

int XSetFillStyle(Display* display, GC gc, int fill_style)
{
	gc->values.fill_style = fill_style;
	gc->dirty = True;
	return 0;
}

int
XSetFont(Display *display, GC gc, Font font)
{
	gc->values.font = font;
	return 0;
}

void bex_check_gc(XWindow *window, GC gc)
{
	if (!gc) {
		// Use the window's default GC, or make one for it.
		if (!window->default_gc)
			window->default_gc = XCreateGC(NULL, 0, 0, NULL);
		gc = window->default_gc;
	}
	if (window->gc == gc && !gc->dirty)
		return;

	window->gc = gc;
	window->SetHighColor(create_rgb(gc->values.foreground));
	window->SetLowColor(create_rgb(gc->values.background));
	window->SetPenSize(gc->values.line_width);

	if (gc->values.font) {
		XFontStruct xf;
		xf.fid = gc->values.font;
		BFont* bfont = bfont_from_xfontstruct(&xf);
		window->SetFont(bfont);
	}

	cap_mode cap;
	switch(gc->values.cap_style) {
	case CapRound:
		cap = B_ROUND_CAP;
		break;
	case CapProjecting:
		cap = B_SQUARE_CAP;
		break;
	case CapNotLast:
	case CapButt:
	default:
		cap = B_ROUND_CAP;
		break;
	}

	join_mode join;
	switch(gc->values.join_style) {
	case JoinRound:
		join = B_ROUND_JOIN;
		break;
	case JoinBevel:
		join = B_BEVEL_JOIN;
		break;
	case JoinMiter:
	default:
		join = B_MITER_JOIN;
		break;
	}
	window->SetLineMode(cap, join);

	gc->dirty = False;
}
