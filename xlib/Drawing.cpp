#include <interface/Polygon.h>

#include "Drawables.h"
#include "FontList.h"
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
	switch(gc->values.fill_style) {
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
	int i;
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for ( i=0; i<n; i++ ) {
		view->StrokeRect(BRect(rect[i].x, rect[i].y,
			rect[i].x + rect[i].width -1, rect[i].y + rect[i].height -1),
			pattern_for(gc));
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
	int i;
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for ( i=0; i<n; i++ ) {
		view->FillRect(BRect(rect[i].x, rect[i].y,
			rect[i].x + rect[i].width -1, rect[i].y + rect[i].height -1),
			pattern_for(gc));
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
	int	i;
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for( i=0; i<n; i++ ) {
		view->StrokeArc(BRect(arc[i].x, arc[i].y,
				arc[i].x+arc[i].width-1, arc[i].y+arc[i].height-1),
			((float)arc[i].angle1)/64, ((float)arc[i].angle2)/64,
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
	int	i;
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	for( i=0; i<n; i++ ) {
		view->FillArc(BRect(arc[i].x, arc[i].y,
				arc[i].x+arc[i].width-1, arc[i].y+arc[i].height-1),
			((float)arc[i].angle1)/64, ((float)arc[i].angle2)/64,
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
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	BPoint point(x, y);
	view->SetPenSize(1);
	view->StrokeLine(point, point, pattern_for(gc));
	view->UnlockLooper();
	return 0;
}

extern "C" int
XDrawPoints(Display *display, Drawable w, GC gc,
	XPoint *points, int n, int mode)
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
		for( i=0; i<n; i++ ) {
			BPoint point(points[i].x, points[i].y);
			view->SetPenSize(1);
			view->StrokeLine(point, point, pattern_for(gc));
		}
		break;
	case CoordModePrevious:
		for( i=0; i<n; i++ ) {
			wx = wx + points[i].x;
			wy = wy + points[i].y;
			BPoint point( wx, wy );
			view->SetPenSize(1);
			view->StrokeLine(point, point, pattern_for(gc));
		}
		break;
	}
	view->UnlockLooper();
	return 0;
}

extern "C" int
XTextWidth(XFontStruct* font_struct, const char *string, int count)
{
	BFont* bfont = bfont_from_font(font_struct->fid);
	return bfont->StringWidth(string, count);
}

extern "C" int
XTextWidth16(XFontStruct *font_struct, const XChar2b *string, int count)
{
	return XTextWidth(font_struct, (const char*)string, count);
}

extern "C" int
XDrawString(Display *display, Drawable w, GC gc, int x, int y, const char* str, int len)
{
	XDrawable* window = Drawables::get(w);
	BView* view = window->view();
	view->LockLooper();
	bex_check_gc(window, gc);
	view->DrawString(str, len, BPoint(x, y));
	view->UnlockLooper();
	return 0;
}

extern "C" int
XDrawString16(Display *display, Drawable w, GC gc, int x, int y, const XChar2b* str, int len)
{
	return XDrawString(display, w, gc, x, y, (char*)str, len);
}

extern "C" int
XDrawImageString(Display *display, Drawable w, GC gc, int x, int y, const char* str, int len)
{
	return XDrawString(display, w, gc, x, y, str, len);
}

extern "C" void
XmbDrawString(Display *display, Drawable w, XFontSet font_set,
	GC gc, int x, int y, const char* str, int len)
{
	XDrawString(display, w, gc, x, y, str, len);
}
