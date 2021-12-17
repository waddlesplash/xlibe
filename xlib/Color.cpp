#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <interface/InterfaceDefs.h>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
}

#include "tables/ColorTable.h"
#include "Color.h"

static XID sDummy;

color_space
_x_color_space(Visual* v, int bits_per_pixel)
{
	// We assume everything is little-endian at present.
	static_assert(B_RGBA32 == B_RGBA32_LITTLE);

	switch (bits_per_pixel) {
	case 1:  return B_GRAY1;
	case 8:  return B_GRAY8;
	case 15: return B_RGB15;
	case 16: return B_RGB16;
	case 24: return B_RGB24;
	case 32: return B_RGBA32;
	default:
		debugger("Unsupported color space!");
		return B_RGB24;
	}
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
XParseColor(Display *dpy, Colormap cmap, const char *spec, XColor *def)
{
	if (spec[0] == '#') {
		int i;
		if (((i = strlen(spec + 1)) % 3) != 0)
			return 0;
		i /= 3;

		char fmt[16];
		int red, green, blue;
		sprintf(fmt, "%%%dx%%%dx%%%dx", i, i, i);
		if (sscanf(spec + 1, fmt, &red, &green, &blue) != 3)
			return 0;

		def->red = red << 8;
		def->green = green << 8;
		def->blue = blue << 8;
	} else if (strncmp(spec, "rgb:", 4) == 0) {
		int red, green, blue;
		if (sscanf(spec + 4, "%2x/%2x/%2x", &red, &green, &blue) != 3)
			return 0;

		def->red = red << 8;
		def->green = green << 8;
		def->blue = blue << 8;
	} else {
		if (!FindColor(spec, def))
			return 0;
	}
	def->pixel = _x_rgb_to_pixel(make_color(def->red / 257, def->green / 257, def->blue / 257));
	def->flags = DoRed | DoGreen | DoBlue;
	def->pad = 0;

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

extern "C" Status
XQueryColors(Display *display, Colormap colormap,
	XColor *defs_in_out, int ncolors)
{
	for (int i = 0; i < ncolors; i++) {
		rgb_color color = _x_pixel_to_rgb(defs_in_out[i].pixel);
		defs_in_out[i].red = color.red * 257;
		defs_in_out[i].green = color.green * 257;
		defs_in_out[i].blue = color.blue * 257;
		defs_in_out[i].flags = DoRed | DoGreen | DoBlue;
		defs_in_out[i].pad = 0;
	}
	return Success;
}

extern "C" Status
XQueryColor(Display *display, Colormap colormap,
	XColor* defs_in_out)
{
	return XQueryColors(display, colormap, defs_in_out, 1);
}

extern "C" int
XAllocColor(Display* dpy, Colormap cmap, XColor* def)
{
	def->pixel = _x_rgb_to_pixel(make_color(def->red / 257, def->green / 257, def->blue / 257));
	return 1;
}

extern "C" int
XFreeColors(Display *display, Colormap colormap,
	unsigned long *pixels, int npixels, unsigned long planes)
{
	// Nothing to do.
	return Success;
}

extern "C" XStandardColormap*
XAllocStandardColormap()
{
	return (XStandardColormap*)malloc(sizeof(XStandardColormap));
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
XAllocColorCells(Display *display, Colormap colormap,
	Bool contig, unsigned long* plane_masks_return, unsigned int nplanes,
	unsigned long* pixels_return, unsigned int npixels)
{
	if (colormap == (Colormap)&sDummy)
		return Success;

	// We don't support anything else.
	return BadImplementation;
}

extern "C" Status
XInstallColormap(Display* display, Colormap colormap)
{
	if (colormap == (Colormap)&sDummy)
		return Success;
	return BadImplementation;
}

extern "C" Status
XStoreColors(Display* display, Colormap colormap, XColor* color, int ncolors)
{
	return BadColor;
}

extern "C" Status
XStoreColor(Display* dpy, Colormap cmap, XColor* color)
{
	return XStoreColors(dpy, cmap, color, 1);
}

extern "C" Status
XFreeColormap(Display* display, Colormap colormap)
{
	return Success;
}
