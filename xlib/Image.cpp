extern "C" {
#include <X11/Xlib.h>
}

extern "C" int
_XInitImageFuncPtrs(XImage *image)
{
	return 0;
}

XImage *
XCreateImage(Display *display, Visual *visual,
	unsigned int depth, int format, int offset, char *data,
	unsigned int width, unsigned int height,
	int bitmap_pad, int bytes_per_line)
{
	return NULL;
}

extern "C" XImage *
XGetImage(Display *display, Drawable d, int x, int y,
  unsigned int width, unsigned int height, unsigned long plane_mask,  int format)
{
	return NULL;
}

extern "C" int
XPutImage(Display *display, Drawable d, GC gc, XImage *image,
  int src_x, int src_y, int dest_x, int dest_y,
  unsigned int width, unsigned int height)
{
	return BadImplementation;
}
