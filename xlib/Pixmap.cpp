#include <X11/Xlib.h>
#include <interface/Bitmap.h>

#include "Drawables.h"
#include "Debug.h"

extern "C" Pixmap
XCreatePixmap(Display* display, Drawable d,
	unsigned int width, unsigned int height, unsigned int depth)
{
	BRect rect(0, 0, width - 1, height - 1);
	XPixmap* pixmap = new XPixmap(rect, depth);
	return pixmap->id();
}

extern "C" Pixmap
XCreateBitmapFromData(Display* display, Drawable d,
	const char* data, unsigned int width, unsigned int height)
{
	BRect rect(0, 0, width - 1, height - 1);
	XPixmap* pixmap = new XPixmap(rect, 1);
	pixmap->offscreen()->ImportBits(data, width * height, B_ANY_BYTES_PER_ROW, 0, B_GRAY8);
	return pixmap->id();
}

extern "C" int
XFreePixmap(Display *display, Pixmap pxm)
{
	XPixmap* pixmap = Drawables::get_pixmap(pxm);
	if (!pxm)
		return BadPixmap;

	delete pixmap;
	return Success;
}

extern "C" int
XCopyArea(Display* display, Drawable src, Drawable dest, GC gc,
	int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y)
{
	XDrawable* src_d = Drawables::get(src);
	XDrawable* dest_d = Drawables::get(dest);
	if (!src_d || !dest_d)
		return BadDrawable;

	BRect src_rect(src_x, src_y, src_x + width - 1, src_y + height - 1);
	BRect dest_rect(dest_x, dest_y, dest_x + width - 1, dest_y + height - 1);

	if (src_d == dest_d) {
		src_d->view()->LockLooper();
		src_d->view()->CopyBits(src_rect, dest_rect);
		src_d->view()->UnlockLooper();
		return Success;
	}

	// TODO?
	UNIMPLEMENTED();
	return BadValue;
}

extern "C" int
XCopyPlane(Display *display, Drawable src, Drawable dest, GC gc,
	int src_x, int src_y, unsigned int width, unsigned int height,
	int dest_x, int dest_y, unsigned long plane)
{
	// TODO: Actually use "plane"!
	return XCopyArea(display, src, dest, gc, src_x, src_y, width, height, dest_x, dest_y);
}
