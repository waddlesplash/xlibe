#include <X11/Xlib.h>
#include "XInnerWindow.h"

Events::Events() {
	counter_ = create_sem(0, "counter");
	wait_ = create_sem(1, "wait flag");
	lock_ = create_sem(1, "lock");
	waiting_thread_ = -1;
}

bool Events::is_match(long mask, long event) const {
	switch(event) {
	case KeyPress:
		return (mask & KeyPressMask);
	case KeyRelease:
		return (mask & KeyReleaseMask);
	case ButtonPress:
		return (mask & ButtonPressMask);
	case ButtonRelease:
		return (mask & ButtonReleaseMask);
	case MotionNotify:
	case EnterNotify:
	case LeaveNotify:
	case FocusIn:
	case FocusOut:
	case KeymapNotify:
		return false;
	case Expose:
		return (mask & ExposureMask);
	case GraphicsExpose:
	case NoExpose:
	case VisibilityNotify:
	case CreateNotify:
	case DestroyNotify:
	case UnmapNotify:
	case MapNotify:
	case MapRequest:
	case ReparentNotify:
	case ConfigureNotify:
	case ConfigureRequest:
	case GravityNotify:
	case ResizeRequest:
	case CirculateNotify:
	case CirculateRequest:
	case PropertyNotify:
	case SelectionClear:
	case SelectionRequest:
	case SelectionNotify:
	case ColormapNotify:
	case ClientMessage:
	case MappingNotify:
	default:
		return false;
	}
}

Events& Events::instance() {
	static Events events;
	return events;
}

void Events::add(XEvent* event) {
	acquire_sem(lock_);
	list_.push_back(event);
	release_sem(lock_);
	if(waiting_thread_ == -1) {
		resume_thread(waiting_thread_);
	}
	release_sem(counter_);
}

void Events::wait_for_next(XEvent* event) {
	acquire_sem(counter_);
	acquire_sem(lock_);
	*event = *(list_.front());
	list_.pop_front();
	release_sem(lock_);
}

void Events::wait_for_coming() {
	acquire_sem(wait_);
	waiting_thread_ = find_thread(NULL);
	suspend_thread(waiting_thread_);
	waiting_thread_ = -1;
	release_sem(wait_);
}

void Events::wait_event(XEvent* event, long event_mask) {
	acquire_sem(lock_);
	std::list<XEvent*>::iterator i;
	for(i=list_.begin();i!=list_.end();++i) {
		if(is_match(event_mask, (*i)->type)) {
			*event = *(*i);
			list_.erase(i);
			acquire_sem(counter_);
			release_sem(lock_);
			return;
		}
	}
	release_sem(lock_);
	for(;;) {
		wait_for_coming();
		acquire_sem(lock_);
		if(list_.back()->type == event_mask) {
			*event = *(*i);
			list_.erase(i);
			acquire_sem(counter_);
			release_sem(lock_);
			return;
		}
		release_sem(lock_);
	}
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
	Events::instance().wait_for_next(event);
	return 0;
}

extern "C" int XMaskEvent(Display* display, long event_mask, XEvent* event_return) {
	XFlush(display);
	Events::instance().wait_event(event_return, event_mask);
	return 0;
}

