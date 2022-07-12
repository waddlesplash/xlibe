/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <interface/Region.h>
#include <interface/Polygon.h>

#include "Drawing.h"
#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

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
	result->Include(brect_from_xrect(*rect));
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
	BRect rect = brect_from_xrect(make_xrect(x, y, width, height));
	if (region->Intersects(rect)) {
		BRegion intersect;
		intersect.Set(rect);
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
	*rect_return = xrect_from_brect(region->Frame());
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
