#include <interface/Bitmap.h>
#include <interface/Polygon.h>

#include "Drawables.h"
#include "Drawing.h"
#include "Font.h"
#include "GC.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

#include "Debug.h"

static pattern
pattern_for(GC gc)
{
	pattern ptn;
	switch (gc->values.fill_style) {
	default:
	case FillSolid:
		ptn = B_SOLID_HIGH;
	break;
	case FillTiled:
	case FillStippled:
		// TODO: proper implementation?
	case FillOpaqueStippled:
		ptn = B_MIXED_COLORS;
	break;
	}
	return ptn;
}

extern "C" int
XDrawLine(Display *display, Drawable w, GC gc,
	int x1, int y1, int x2, int y2)
{
	XSegment seg;
	seg.x1 = x1;
	seg.y1 = y1;
	seg.x2 = x2;
	seg.y2 = y2;
	return XDrawSegments(display, w, gc, &seg, 1);
}

extern "C" int
XDrawSegments(Display *display, Drawable w, GC gc,
	XSegment *segments, int ns)
{
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for(int i = 0; i < ns; i++) {
		BPoint point1(segments[i].x1, segments[i].y1);
		BPoint point2(segments[i].x2, segments[i].y2);
		view->StrokeLine(point1, point2, pattern_for(gc));
	}
	view->UnlockLooper();
	return 0;
}

extern "C" int
XDrawLines(Display *display, Drawable w, GC gc,
	XPoint *points, int np, int mode)
{
	int	i;
	short	wx, wy;
	wx = 0;
	wy = 0;
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	switch( mode ) {
	case CoordModeOrigin :
		for( i=0; i<(np-1); i++ ) {
			BPoint point1(points[i].x, points[i].y);
			BPoint point2(points[i+1].x, points[i+1].y);
			view->StrokeLine(point1, point2, pattern_for(gc));
		}
		break;
	case CoordModePrevious:
		for( i=0; i<np; i++ ) {
			if ( i==0 ) {
				wx = wx + points[i].x;
				wy = wy + points[i].y;
				BPoint point1( wx, wy );
				BPoint point2( wx, wy );
				view->StrokeLine(point1, point2, pattern_for(gc));
			}
			else {
				BPoint point3( wx, wy );
				wx = wx + points[i].x;
				wy = wy + points[i].y;
				BPoint point4( wx, wy );
				view->StrokeLine(point3, point4, pattern_for(gc));
			}
		}
		break;
	}
	view->UnlockLooper();
	return 0;
}

extern "C" int
XDrawRectangle(Display *display, Drawable w, GC gc,
	int x,int y, unsigned int width, unsigned int height)
{
	XRectangle rect;
	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;
	return XDrawRectangles(display, w, gc, &rect, 1);
}

extern "C" int
XDrawRectangles(Display *display, Drawable w, GC gc,
	XRectangle *rect, int n)
{
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for (int i = 0; i < n; i++) {
		view->StrokeRect(brect_from_xrect(rect[i]), pattern_for(gc));
	}
	view->UnlockLooper();
	return 0;
}

extern "C" int
XFillRectangle(Display *display, Drawable win, GC gc,
	int x, int y, unsigned int w, unsigned int h)
{
	XRectangle rect;
	rect.x = x;
	rect.y = y;
	rect.width = w;
	rect.height = h;
	return XFillRectangles(display, win, gc, &rect, 1);
}

extern "C" int
XFillRectangles(Display *display, Drawable w, GC gc,
	XRectangle *rect, int n)
{
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for (int i = 0; i < n; i++) {
		view->FillRect(brect_from_xrect(rect[i]), pattern_for(gc));
	}
	view->UnlockLooper();
	return 0;
}

extern "C" int
XDrawArc(Display *display, Drawable w, GC gc,
	int x, int y, unsigned int width,unsigned height, int a1, int a2)
{
	XArc arc;
	arc.x = x;
	arc.y = y;
	arc.width = width;
	arc.height = height;
	arc.angle1 = a1;
	arc.angle2 = a2;
	return XDrawArcs(display, w, gc, &arc, 1);
}

extern "C" int
XDrawArcs(Display *display, Drawable w, GC gc, XArc *arc, int n)
{
	// FIXME: Take arc_mode into account!
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for (int i = 0; i < n; i++) {
		view->StrokeArc(brect_from_xrect(make_xrect(arc[i].x, arc[i].y, arc[i].width, arc[i].height)),
			((float)arc[i].angle1) / 64, ((float)arc[i].angle2) / 64,
			pattern_for(gc));
	}
	view->UnlockLooper();
	return 0;
}

extern "C" int
XFillArc(Display *display, Drawable w, GC gc,
	int x, int y, unsigned int width, unsigned height, int a1, int a2)
{
	XArc arc;
	arc.x = x;
	arc.y = y;
	arc.width = width;
	arc.height = height;
	arc.angle1 = a1;
	arc.angle2 = a2;
	return XFillArcs(display, w, gc, &arc, 1);
}

extern "C" int
XFillArcs(Display *display, Drawable w, GC gc,
	XArc *arc, int n)
{
	// FIXME: Take arc_mode into account!
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for (int i = 0; i < n; i++) {
		view->FillArc(brect_from_xrect(make_xrect(arc[i].x, arc[i].y, arc[i].width, arc[i].height)),
			((float)arc[i].angle1) / 64.0f, ((float)arc[i].angle2) / 64.0f,
			pattern_for(gc));
	}
	view->UnlockLooper();
	return 0;
}

extern "C" int
XFillPolygon(Display *display, Drawable w, GC gc,
	XPoint *points, int npoints, int shape, int mode)
{
	BPolygon polygon;
	switch (mode) {
	case CoordModeOrigin :
		for(int i = 0; i < npoints; i++) {
			BPoint point(points[i].x, points[i].y);
			polygon.AddPoints(&point, 1);
		}
		break;
	case CoordModePrevious: {
		int wx = 0, wy = 0;
		for(int i = 0; i < npoints; i++) {
			wx = wx + points[i].x;
			wy = wy + points[i].y;
			BPoint point(wx, wy);
			polygon.AddPoints(&point, 1);
		}
		break;
	}
	}

	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	view->FillPolygon(&polygon, pattern_for(gc));
	view->UnlockLooper();
	return 0;
}

extern "C" int
XDrawPoint(Display *display, Drawable w, GC gc, int x, int y)
{
	XPoint point;
	point.x = x;
	point.y = y;
	return XDrawPoints(display, w, gc, &point, 1, CoordModeOrigin);
}

extern "C" int
XDrawPoints(Display *display, Drawable w, GC gc,
	XPoint* points, int n, int mode)
{
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	view->PushState();
	view->SetPenSize(1);
	switch (mode) {
	case CoordModeOrigin :
		for (int i = 0; i < n; i++) {
			BPoint point(points[i].x, points[i].y);
			view->StrokeLine(point, point, pattern_for(gc));
		}
		break;
	case CoordModePrevious: {
		short wx = 0, wy = 0;
		for (int i = 0; i < n; i++) {
			wx = wx + points[i].x;
			wy = wy + points[i].y;
			BPoint point( wx, wy );
			view->StrokeLine(point, point, pattern_for(gc));
		}
		break;
	}
	}
	view->PopState();
	view->UnlockLooper();
	return 0;
}

extern "C" int
XCopyArea(Display* display, Drawable src, Drawable dest, GC gc,
	int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y)
{
	XDrawable* src_d = Drawables::get(src);
	XDrawable* dest_d = Drawables::get(dest);
	if (!src_d || !dest_d)
		return BadDrawable;

	const BRect src_rect = brect_from_xrect(make_xrect(src_x, src_y, width, height));
	const BRect dest_rect = brect_from_xrect(make_xrect(dest_x, dest_y, width, height));

	if (src_d == dest_d) {
		src_d->view()->LockLooper();
		bex_check_gc(src_d, gc);
		src_d->view()->CopyBits(src_rect, dest_rect);
		src_d->view()->UnlockLooper();
		return Success;
	}

	XPixmap* src_pxm = dynamic_cast<XPixmap*>(src_d);
	if (src_pxm) {
		dest_d->view()->LockLooper();
		bex_check_gc(dest_d, gc);
		dest_d->view()->DrawBitmap(src_pxm->offscreen(), src_rect, dest_rect);
		dest_d->view()->UnlockLooper();
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

extern "C" int
XPutImage(Display *display, Drawable d, GC gc, XImage* image,
	int src_x, int src_y, int dest_x, int dest_y,
	unsigned int width, unsigned int height)
{
	XDrawable* drawable = Drawables::get(d);
	if (!drawable)
		return BadDrawable;

	BBitmap* bbitmap = (BBitmap*)image->obdata;
	if (image->data != bbitmap->Bits()) {
		// We must import the bits before drawing.
		// TODO: Optimization: Import only the bits we are about to draw!
		bbitmap->ImportBits(image->data, image->height * image->bytes_per_line,
			image->bytes_per_line, image->xoffset, bbitmap->ColorSpace());
	}

	BView* view = drawable->view();
	view->LockLooper();
	bex_check_gc(drawable, gc);
	view->DrawBitmap(bbitmap, brect_from_xrect(make_xrect(src_x, src_y, width, height)),
		brect_from_xrect(make_xrect(dest_x, dest_y, width, height)));
	view->UnlockLooper();
	return Success;
}

extern "C" void
Xutf8DrawString(Display *display, Drawable w, XFontSet set, GC gc, int x, int y, const char* str, int len)
{
	// FIXME: Use provided fonts!
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	view->DrawString(str, len, BPoint(x, y));
	view->UnlockLooper();
}

extern "C" int
XDrawString(Display* display, Drawable w, GC gc, int x, int y, const char* str, int len)
{
	Xutf8DrawString(display, w, NULL, gc, x, y, str, len);
	return 0;
}

extern "C" int
XDrawImageString(Display *display, Drawable w, GC gc, int x, int y, const char* str, int len)
{
	return XDrawString(display, w, gc, x, y, str, len);
}

extern "C" int
XDrawText(Display *display, Drawable w, GC gc, int x, int y, XTextItem* items, int count)
{
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	view->PushState();
	for (int i = 0; i < count; i++) {
		if (items[i].font != None) {
			BFont font = bfont_from_font(items[i].font);
			view->SetFont(&font);
		}
		view->DrawString(items[i].chars, items[i].nchars, BPoint(x, y));
		x += view->StringWidth(items[i].chars, items[i].nchars);
		x += items[i].delta;
	}
	view->PopState();
	view->UnlockLooper();
	return 0;
}
