#include <X11/Xlib.h>
#include "GC.h"
#include "XInnerWindow.h"

extern "C" int XDrawLine(Display *display, Drawable w, GC gc, int x1,int y1, int x2,int y2) {
  XWindow* window = Windows::get_xwindow(w);
  window->lock();
  check_gc(window, gc);
  window->StrokeLine(BPoint(x1, y1), BPoint(x2, y2));
  window->unlock();
  return 0;
}

extern "C" int XDrawRectangle(Display *display, Drawable w, GC gc, int x,int y, unsigned int w, unsigned int h) {
  XWindow* window = Windows::get_xwindow(w);
  window->lock();
  check_gc(window, gc);
  window->StrokeRect(BRect(x, y, x+w, y+h));
  window->unlock();
  return 0;
}

extern "C" int XFillRectangle(Display *display, Drawable win, GC gc, int x, int y, unsigned int w, unsigned int h) {
  XWindow* window = Windows::get_xwindow(win);
  window->lock();
  check_gc(window, gc);
  window->FillRect(BRect(x, y, x+w, y+h));
  window->unlock();
  return 0;
}

extern "C" int XDrawArc(Display *display, Drawable w, GC gc,
		 int x, int y, unsigned int width,unsigned height, int a1, int a2) {
  XWindow* window = Windows::get_xwindow(w);
  window->lock();
  check_gc(window, gc);
  window->StrokeArc(BRect(x, y, x+width, y+height), ((float)a1)/64, ((float)a2)/64);
  window->unlock();
  return 0;
}

extern "C" int XFillArc(Display *display, Drawable w, GC gc, int x, int y, unsigned int width,unsigned height, int a1, int a2) {
  XWindow* window = Windows::get_xwindow(w);
  window->lock();
  check_gc(window, gc);
  window->FillArc(BRect(x, y, x+width, y+height), ((float)a1)/64, ((float)a2)/64);
  window->unlock();
  return 0;
}

extern "C" int XDrawPoint(Display *display, Drawable w, GC gc, int x, int y) {
  XWindow* window = Windows::get_xwindow(w);
  window->lock();
  check_gc(window, gc);
  BPoint point(x, y);
  window->SetPenSize(1);
  window->StrokeLine(point, point);
  window->unlock();
  return 0;
}

extern "C" int XDrawPoints(Display *display, Drawable w, GC gc, XPoint *points, int n, int mode) {
  int i;
  short wx, wy;
  wx = 0;
  wy = 0;
  XWindow* window = Windows::get_xwindow(w);
  window->lock();
  check_gc(window, gc);
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
