#ifndef EVENT_H
#define EVENT_H

#include <kernel/OS.h>
#include <support/Locker.h>
#include <list>

extern "C" {
#include <X11/Xlib.h>
}

class Events {
private:
	BLocker lock_;
	std::list<XEvent> list_;

	Events();
	void wait_for_more();
	static bool is_match(long mask, long event);

public:
	Display* dpy_ = NULL;
	static Events& instance();

	void add(XEvent event);
	void wait_for_next(Display* dpy, XEvent* event);
	void wait_event(XEvent* event, long event_mask);
};

#endif
