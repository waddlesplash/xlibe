#include <deque>
#include <X11/Xlib.h>
#include "XInnerWindow.h"
#include <kernel/OS.h>

deque<XEvent*> gevents;
sem_id gcounter = 0;

void add_event(XEvent* event) {
  if(gcounter == 0)
    gcounter = create_sem(0, "Event Counter");
  gevents.push_back(event);
  release_sem(gcounter);
}

extern "C" int XSelectInput(Display *display, Window window, long mask) {
  XWindow* xwindow = Windows::get_xwindow(window);
  if(xwindow == 0)
    return 1;
  xwindow->event_mask(xwindow->event_mask() ^ mask);
  return 0;
}

extern "C" int XNextEvent(Display *display, XEvent *event) {
  XFlush(display);
  if(gcounter == 0)
    gcounter = create_sem(0, "Event Counter");
  acquire_sem(gcounter);
  *event = *(gevents.front());
  gevents.pop_front();
  return 0;
}
