#include <stdio.h>
#include "Drawables.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

Events::Events()
{
	counter_ = create_sem(0, "counter");
	wait_ = create_sem(1, "wait flag");
	lock_ = create_sem(1, "lock");
	waiting_thread_ = -1;
}

bool Events::is_match(long mask, long event)
{
	if (mask == (~NoEventMask))
		return true;

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
		return (mask & PointerMotionMask);
	case EnterNotify:
		return (mask & EnterWindowMask);
	case LeaveNotify:
		return (mask & LeaveWindowMask);
	case FocusIn:
	case FocusOut:
		return (mask & FocusChangeMask);
	case KeymapNotify:
		return (mask & KeymapStateMask);
	case Expose:
	case GraphicsExpose:
	case NoExpose:
		return (mask & ExposureMask);
	case VisibilityNotify:
		return (mask & VisibilityChangeMask);
	case CreateNotify:
		return (mask & SubstructureNotifyMask);
	case DestroyNotify:
	case UnmapNotify:
	case MapNotify:
	case MapRequest:
	case ReparentNotify:
	case ConfigureNotify:
	case ConfigureRequest:
	case GravityNotify:
		break;
	case ResizeRequest:
		return (mask & ResizeRedirectMask);
	case CirculateNotify:
	case CirculateRequest:
		break;
	case PropertyNotify:
		return (mask & PropertyChangeMask);
	case SelectionClear:
	case SelectionRequest:
	case SelectionNotify:
	case ColormapNotify:
	case ClientMessage:
	case MappingNotify:
	default:
		break;
	}
	return false;
}

Events& Events::instance()
{
	static Events events;
	return events;
}

void Events::add(XEvent* event)
{
	acquire_sem(lock_);
	event->xany.display = dpy_;
	dpy_->qlen++;
	list_.push_back(event);
	release_sem(lock_);
	if (waiting_thread_ == -1) {
		resume_thread(waiting_thread_);
	}
	release_sem(counter_);

	char dummy[1];
	write(dpy_->conn_checker, dummy, 1);
}

void Events::wait_for_next(Display* dpy, XEvent* event)
{
	acquire_sem(counter_);
	acquire_sem(lock_);
	char dummy[1];
	read(dpy->fd, dummy, 1);
	*event = *(list_.front());
	dpy_->qlen--;
	list_.pop_front();
	release_sem(lock_);
}

void Events::wait_for_coming()
{
	acquire_sem(wait_);
	waiting_thread_ = find_thread(NULL);
	suspend_thread(waiting_thread_);
	waiting_thread_ = -1;
	release_sem(wait_);
}

void Events::wait_event(XEvent* event, long event_mask)
{
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

extern "C" int
XSelectInput(Display *display, Window w, long mask)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;
	window->event_mask(mask);
	return Success;
}

extern "C" int
XEventsQueued(Display *display, int mode)
{
	return QLength(display);
}

extern "C" int
XNextEvent(Display *display, XEvent *event)
{
	XFlush(display);
	Events::instance().wait_for_next(display, event);
	return 0;
}

extern "C" int
XMaskEvent(Display* display, long event_mask, XEvent* event_return)
{
	XFlush(display);
	Events::instance().wait_event(event_return, event_mask);
	return 0;
}
