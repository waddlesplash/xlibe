#include <X11/Xlib.h>
#include "GC.h"
#include "XInnerWindow.h"

extern "C" int XDrawLine(Display *display, Drawable w, GC gc,
						 int x1,int y1, int x2,int y2) {
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	window->StrokeLine(BPoint(x1, y1), BPoint(x2, y2));
	window->unlock();
	return 0;
}

extern "C" int XDrawSegments(Display *display, Drawable w, GC gc,
							 XSegment *segments, int ns) {
	int	i;
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	for( i=0; i<ns; i++ ) {
		BPoint point1(segments[i].x1, segments[i].y1);
		BPoint point2(segments[i].x2, segments[i].y2);
		window->StrokeLine(point1, point2);
	}
	window->unlock();
	return 0;
}

extern "C" int XDrawLines(Display *display, Drawable w, GC gc,
						  XPoint *points, int np, int mode) {
	int	i;
	short	wx, wy;
	wx = 0;
	wy = 0;
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	switch( mode ) {
	case CoordModeOrigin :
		for( i=0; i<(np-1); i++ ) {
			BPoint point1(points[i].x, points[i].y);
			BPoint point2(points[i+1].x, points[i+1].y);
			window->StrokeLine(point1, point2);
		}
		break;
	case CoordModePrevious:
		for( i=0; i<np; i++ ) {
			if ( i==0 ) {
				wx = wx + points[i].x;
				wy = wy + points[i].y;
				BPoint point1( wx, wy );
				BPoint point2( wx, wy );
				window->StrokeLine(point1, point2);
			}
			else {
				BPoint point3( wx, wy );
				wx = wx + points[i].x;
				wy = wy + points[i].y;
				BPoint point4( wx, wy );
				window->StrokeLine(point3, point4);
			}
		}
		break;
	}
	window->unlock();
	return 0;
}

extern "C" int XDrawRectangle(Display *display, Drawable w, GC gc,
							  int x,int y, unsigned int width, unsigned int height) {
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	window->StrokeRect(BRect(x, y, x+width-1, y+height-1));
	window->unlock();
	return 0;
}

extern "C" int XDrawRectangles(Display *display, Drawable w, GC gc,
							   XRectangle *rect, int n) {
	int i;
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	for ( i=0; i<n; i++ ) {
		window->StrokeRect(BRect(rect[i].x, rect[i].y,
								 rect[i].x + rect[i].width -1, rect[i].y + rect[i].height -1));
	}
	window->unlock();
	return 0;
}

extern "C" int XFillRectangle(Display *display, Drawable win, GC gc,
							  int x, int y, unsigned int w, unsigned int h) {
	XWindow* window = Windows::get_xwindow(win);
	window->lock();
	bex_check_gc(window, gc);
	window->FillRect(BRect(x, y, x+w-1, y+h-1));
	window->unlock();
	return 0;
}

extern "C" int XFillRectangles(Display *display, Drawable w, GC gc,
							   XRectangle *rect, int n) {
	int i;
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	for ( i=0; i<n; i++ ) {
		window->FillRect(BRect(rect[i].x, rect[i].y,
							   rect[i].x + rect[i].width -1, rect[i].y + rect[i].height -1));
	}
	window->unlock();
	return 0;
}

extern "C" int XDrawArc(Display *display, Drawable w, GC gc,
						int x, int y, unsigned int width,unsigned height, int a1, int a2) {
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	window->StrokeArc(BRect(x, y, x+width-1, y+height-1),
					  ((float)a1)/64, ((float)a2)/64);
	window->unlock();
	return 0;
}

extern "C" int XDrawArcs(Display *display, Drawable w, GC gc, XArc *arc, int n) {
	int	i;
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	for( i=0; i<n; i++ ) {
		window->StrokeArc(BRect(arc[i].x, arc[i].y,
								arc[i].x+arc[i].width-1, arc[i].y+arc[i].height-1),
						  ((float)arc[i].angle1)/64, ((float)arc[i].angle2)/64);
	}
	window->unlock();
	return 0;
}


extern "C" int XFillArc(Display *display, Drawable w, GC gc,
						int x, int y, unsigned int width,unsigned height, int a1, int a2) {
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	window->FillArc(BRect(x, y, x+width-1, y+height-1),
					((float)a1)/64, ((float)a2)/64);
	window->unlock();
	return 0;
}

extern "C" int XFillArcs(Display *display, Drawable w, GC gc, XArc *arc, int n) {
	int	i;
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	for( i=0; i<n; i++ ) {
		window->FillArc(BRect(arc[i].x, arc[i].y,
							  arc[i].x+arc[i].width-1, arc[i].y+arc[i].height-1),
						((float)arc[i].angle1)/64, ((float)arc[i].angle2)/64);
	}
	window->unlock();
	return 0;
}

extern "C" int XDrawPoint(Display *display, Drawable w, GC gc, int x, int y) {
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	BPoint point(x, y);
	window->SetPenSize(1);
	window->StrokeLine(point, point);
	window->unlock();
	return 0;
}

extern "C" int XDrawPoints(Display *display, Drawable w, GC gc,
						   XPoint *points, int n, int mode) {
	int	i;
	short	wx, wy;
	wx = 0;
	wy = 0;
	XWindow* window = Windows::get_xwindow(w);
	window->lock();
	bex_check_gc(window, gc);
	switch( mode ) {
	case CoordModeOrigin :
		for( i=0; i<n; i++ ) {
			BPoint point(points[i].x, points[i].y);
			window->SetPenSize(1);
			window->StrokeLine(point, point);
		}
		break;
	case CoordModePrevious:
		for( i=0; i<n; i++ ) {
			wx = wx + points[i].x;
			wy = wy + points[i].y;
			BPoint point( wx, wy );
			window->SetPenSize(1);
			window->StrokeLine(point, point);
		}
		break;
	}
	window->unlock();
	return 0;
}
