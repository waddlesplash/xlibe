#include <interface/Region.h>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#define XYWH_TO_CLIPPING_RECT(X, Y, W, H) (clipping_rect){ (X), (Y), ((X) + (W)), ((Y) + (H)) }

Region
XCreateRegion()
{
	BRegion* region = new BRegion;
	return (Region)region;
}

int
XDestroyRegion(Region r)
{
	BRegion* region = (BRegion*)r;
	delete region;
	return 0;
}

extern "C" Bool
XEmptyRegion(Region r)
{
	BRegion* region = (BRegion*)r;
	return region->CountRects() == 0;
}

int
XIntersectRegion(Region srcA, Region srcB, Region res)
{
	BRegion* sourceA = (BRegion*)srcA, *sourceB = (BRegion*)srcB,
		*result = (BRegion*)res;
	*result = *sourceA;
	result->IntersectWith(sourceB);
	return Success;
}

int
XRectInRegion(Region r, int x, int y, unsigned int width, unsigned int height)
{
	BRegion* region = (BRegion*)r;
	clipping_rect c = XYWH_TO_CLIPPING_RECT(x, y, width, height);
	if (region->Intersects(c)) {
		BRegion intersect;
		intersect.Set(c);
		intersect.IntersectWith(region);

		if (intersect.Frame().Width() && intersect.Frame().Height())
			return RectangleIn;
		return RectanglePart;
	}
	return RectangleOut;
}

int
XSubtractRegion(Region srcA, Region srcB, Region res)
{
	BRegion* sourceA = (BRegion*)srcA, *sourceB = (BRegion*)srcB,
		*result = (BRegion*)res;
	*result = *sourceA;
	result->Exclude(sourceB);
	return Success;
}

int
XUnionRectWithRegion(XRectangle* rect, Region src, Region res)
{
	BRegion* source = (BRegion*)src, *result = (BRegion*)res;
	*result = *source;
	result->Include(XYWH_TO_CLIPPING_RECT(rect->x, rect->y, rect->width, rect->height));
	return Success;
}

int
XClipBox(Region r, XRectangle* rect_return)
{
	BRegion* region = (BRegion*)r;
	BRect rect = region->Frame();
	rect_return->x = abs(rect.LeftTop().x);
	rect_return->y = abs(rect.LeftTop().y);
	rect_return->width = rect.IntegerWidth();
	rect_return->height = rect.IntegerHeight();
	return Success;
}
