#ifndef EVENT_H
#define EVENT_H

union _XEvent;
typedef union _XEvent XEvent;

void add_event(XEvent* event);

#endif
