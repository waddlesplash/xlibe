#include <interface/Region.h>
#include <interface/Polygon.h>

#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#define XYWH_TO_CLIPPING_RECT(X, Y, W, H) (clipping_rect){ (X), (Y), ((X) + (W)), ((Y) + (H)) }

extern "C" Region
XCreateRegion()
{
	BRegion* region = new BRegion;
	return (Region)region;
}

extern "C" int
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

extern "C" int
XUnionRegion(Region srcA, Region srcB, Region res)
{
	BRegion* sourceA = (BRegion*)srcA, *sourceB = (BRegion*)srcB,
		*result = (BRegion*)res;
	*result = *sourceA;
	result->Include(sourceB);
	return Success;
}

extern "C" int
XUnionRectWithRegion(XRectangle* rect, Region src, Region res)
{
	BRegion* source = (BRegion*)src, *result = (BRegion*)res;
	*result = *source;
	result->Include(XYWH_TO_CLIPPING_RECT(rect->x, rect->y, rect->width, rect->height));
	return Success;
}

extern "C" int
XSubtractRegion(Region srcA, Region srcB, Region res)
{
	BRegion* sourceA = (BRegion*)srcA, *sourceB = (BRegion*)srcB,
		*result = (BRegion*)res;
	*result = *sourceA;
	result->Exclude(sourceB);
	return Success;
}

extern "C" int
XIntersectRegion(Region srcA, Region srcB, Region res)
{
	BRegion* sourceA = (BRegion*)srcA, *sourceB = (BRegion*)srcB,
		*result = (BRegion*)res;
	*result = *sourceA;
	result->IntersectWith(sourceB);
	return Success;
}

extern "C" int
XXorRegion(Region srcA, Region srcB, Region res)
{
	BRegion* sourceA = (BRegion*)srcA, *sourceB = (BRegion*)srcB,
		*result = (BRegion*)res;
	*result = *sourceA;
	result->ExclusiveInclude(sourceB);
	return Success;
}

extern "C" int
XOffsetRegion(Region r, int dx, int dy)
{
	BRegion* region = (BRegion*)r;
	region->OffsetBy(dx, dy);
	return Success;
}

extern "C" Bool
XEqualRegion(Region srcA, Region srcB)
{
	BRegion* sourceA = (BRegion*)srcA, *sourceB = (BRegion*)srcB;
	return *sourceA == *sourceB;
}

extern "C" Bool
XPointInRegion(Region r, int x, int y)
{
	BRegion* region = (BRegion*)r;
	return region->Contains(x, y);
}

extern "C" int
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

extern "C" int
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

extern "C" Region
XPolygonRegion(XPoint* points, int npoints, int fill_rule)
{
	Region r = XCreateRegion();
	BRegion* region = (BRegion*)r;
	BPolygon polygon;
	for (int i = 0; i < npoints; i++) {
		BPoint point(points[i].x, points[i].y);
		polygon.AddPoints(&point, 1);
	}
	// FIXME: take fill_rule into account!
	// FIXME: this is not the smallest possible region!
	UNIMPLEMENTED();
	region->Include(polygon.Frame());
	return r;
}
