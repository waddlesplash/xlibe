#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include "XInnerWindow.h"
#include "Color.h"
#include <map>
#include <iostream>

extern "C" GC XCreateGC(Display *display, Window window, unsigned long mask, XGCValues *gc_values) {
  GC gc = new _XGC;
  gc->dirty = True;
  return gc;
}

extern "C" int XSetForeground(Display *display, GC gc, unsigned long color) {
  gc->values.foreground = color;
  gc->dirty = True;
  return 0;
}

int XSetGraphicsExposures(Display *display, GC gc, Bool graphics_exposures) {
  gc->values.graphics_exposures = graphics_exposures;
  return 0;
}

void check_gc(XWindow *window, GC gc) {
  if((window->gc() != gc) || (gc->dirty == True)) {
    window->gc(gc);
    gc->dirty = False;
    window->SetHighColor(create_rgb(gc->values.foreground));
  }
}
