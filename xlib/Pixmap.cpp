#include <X11/Xlib.h>
#include <interface/Bitmap.h>

#include "Drawables.h"
#include "Debug.h"

extern "C" Pixmap
XCreatePixmap(Display* display, Drawable d,
	unsigned int width, unsigned int height, unsigned int depth)
{
	BRect rect(BPoint(0, 0), BSize(width, height));
	XPixmap* pixmap = new XPixmap(display, rect, depth);
	return pixmap->id();
}

extern "C" Pixmap
XCreateBitmapFromData(Display* display, Drawable d,
	const char* data, unsigned int width, unsigned int height)
{
	BRect rect(BPoint(0, 0), BSize(width, height));
	XPixmap* pixmap = new XPixmap(display, rect, 1);
	pixmap->offscreen()->ImportBits(data, width * height, (width+7)/8, 0, B_GRAY1);
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
