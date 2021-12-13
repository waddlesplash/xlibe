extern "C" {
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
}

extern "C" Bool
XShmGetImage(Display* display, Drawable d,
	XImage* image, int x, int y, unsigned long plane_mask)
{
	// All images use shared memory (well, if we are using the BBitmap's bits.)
	return XGetSubImage(display, d, x, y, image->width, image->height,
		plane_mask, image->format, image, 0, 0) != NULL;
}

extern "C" Bool
XShmPutImage(Display* display, Drawable d, GC gc,
	XImage* image, int src_x, int src_y, int dst_x, int dst_y,
	unsigned int width, unsigned int height, Bool send_event)
{
	return XPutImage(display, d, gc, image, src_x, src_y, dst_x, dst_y, width, height) == Success;
}
