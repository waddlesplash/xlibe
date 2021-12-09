#ifndef EVENT_H
#define EVENT_H

extern "C" {
#include <X11/Xlib.h>
}

void x_put_event(Display* dpy, const XEvent& event);

#endif
