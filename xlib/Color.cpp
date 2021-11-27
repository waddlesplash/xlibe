#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <interface/InterfaceDefs.h>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

#include "ColorTable.h"
#include "Color.h"

static XID sDummy;

long RGB(unsigned short red, unsigned short green, unsigned short blue)
{
	long result = red / 256 + green + (blue & 0xFF00) * 256;
	return result;
}

rgb_color create_rgb(unsigned long color)
{
	rgb_color rgb;
	rgb.set_to(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
	return rgb;
}

static int
FindColor(const char *name, XColor *def)
{
	static int numXColors = 0;
	if (numXColors == 0) {
		XColorEntry *ePtr;
		for (ePtr = xColors; ePtr->name != NULL; ePtr++) {
			numXColors++;
		}
	}

	int l, u, r, i;
	l = 0;
	u = numXColors - 1;
	while (l <= u) {
		i = (l + u) / 2;
		r = strcasecmp(name, xColors[i].name);
		if (r == 0) {
			break;
		} else if (r < 0) {
			u = i-1;
		} else {
			l = i+1;
		}
	}
	if (l > u) {
		return 0;
	}
	def->red   = xColors[i].red << 8;
	def->green = xColors[i].green << 8;
	def->blue  = xColors[i].blue << 8;
	return 1;
}

extern "C" Status
XLookupColor(Display *dpy, Colormap cmap,
	const char *colorname, XColor *hard_def, XColor *exact_def)
{
	XParseColor(dpy, cmap, colorname, exact_def);
	hard_def->pixel = exact_def->pixel;
	return 1;
}

extern "C" Status
XAllocNamedColor(Display *dpy, Colormap cmap,
	const char *colorname, XColor *hard_def, XColor *exact_def)
{
	return XLookupColor(dpy, cmap, colorname, hard_def, exact_def);
}

Status
XParseColor(Display *dpy, Colormap cmap, const char *spec, XColor *def)
{
	if (spec[0] == '#') {
		char fmt[16];
		int i, red, green, blue;

		if ((i = strlen(spec+1))%3) {
			return 0;
		}
		i /= 3;

		sprintf(fmt, "%%%dx%%%dx%%%dx", i, i, i);
		if (sscanf(spec+1, fmt, &red, &green, &blue) != 3) {
			return 0;
		}
		def->red = ((unsigned short) red) << 8;
		def->green = ((unsigned short) green) << 8;
		def->blue = ((unsigned short) blue) << 8;
	} else if (strncmp(spec, "rgb:", 4) == 0) {
		int red, green, blue;
		if (sscanf(spec+4, "%2x/%2x/%2x", &red, &green, &blue) != 3)
			return 0;
		def->red = ((unsigned short) red) << 8;
		def->green = ((unsigned short) green) << 8;
		def->blue = ((unsigned short) blue) << 8;
	} else {
		if (!FindColor(spec, def))
			return 0;
	}
	def->pixel = RGB(def->red, def->green, def->blue);
	def->flags = DoRed | DoGreen | DoBlue;
	def->pad   = 0;

	return 1;
}

Status
XQueryColors(Display *display, Colormap colormap,
	XColor *defs_in_out, int ncolors)
{
	int i;
	uint32 rm, gm, bm;
	int rs, gs, bs;

	Visual* v = display->screens[0].root_visual;
	rm = v->red_mask;
	gm = v->green_mask;
	bm = v->blue_mask;

	rs = ffs(rm);
	gs = ffs(gm);
	bs = ffs(bm);

	for (i = 0; i < ncolors; i++) {
		defs_in_out[i].red =
			((defs_in_out[i].pixel & rm) >> rs) / 255.0 * USHRT_MAX;
		defs_in_out[i].green =
			((defs_in_out[i].pixel & gm) >> gs) / 255.0 * USHRT_MAX;
		defs_in_out[i].blue =
			((defs_in_out[i].pixel & bm) >> bs) / 255.0 * USHRT_MAX;
	}
	return Success;
}

int
XAllocColor(Display *dpy, Colormap cmap, XColor *def)
{
	def->pixel = RGB(def->red, def->green, def->blue);
	return 1;
}

int
XFreeColors(Display *display, Colormap colormap,
	unsigned long *pixels, int npixels, unsigned long planes)
{
	// Nothing to do.
	return Success;
}

extern "C" Colormap
XCreateColormap(Display* display, Window window, Visual* visual, int allocate)
{
	// Return a dummy colormap for TrueColor, so things do not complain.
	if (allocate == AllocNone && ((visual && visual->c_class == TrueColor) || !visual))
		return (Colormap)&sDummy;
	return None;
}

extern "C" Status
XFreeColormap(Display* display, Colormap colormap)
{
	// Nothing to do, we never allocate colormaps.
	return Success;
}
