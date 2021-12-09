#include "Drawables.h"

#include <support/Autolock.h>
#include <sys/ioctl.h>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

Events::Events()
	: lock_("XEvents")
{
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

void Events::add(XEvent event)
{
	BAutolock evl(lock_);
	event.xany.display = dpy_;
	dpy_->qlen++;
	list_.push_back(event);
	evl.Unlock();

	char dummy[1];
	write(dpy_->conn_checker, dummy, 1);
}

void
Events::wait_for_more()
{
	char dummy[1];
	read(dpy_->fd, dummy, 1);
}

void
Events::wait_for_next(Display* dpy, XEvent* event)
{
	if (!dpy->qlen)
		wait_for_more();

	BAutolock evl(lock_);
	*event = list_.front();
	list_.pop_front();
	dpy->qlen--;
}

void
Events::wait_event(XEvent* event, long event_mask)
{
	BAutolock evl(lock_);
	for (auto i = list_.begin(); i != list_.end(); i++) {
		if (!is_match(event_mask, i->type))
			continue;

		*event = (*i);
		list_.erase(i);
		dpy_->qlen--;
		return;
	}
	int end = list_.size();
	evl.Unlock();

	while (true) {
		wait_for_more();

		evl.Lock();
		auto i = list_.begin();
		for (int j = 0; j < end; j++)
			i++;
		if (is_match(event_mask, i->type)) {
			*event = list_.back();
			list_.pop_back();
			dpy_->qlen--;
			return;
		}
		end++;
		evl.Unlock();
	}
}

extern "C" int
XSelectInput(Display* display, Window w, long mask)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;
	window->event_mask(mask);
	return Success;
}

extern "C" int
XNextEvent(Display* display, XEvent *event)
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

extern "C" int
XFlush(Display* dpy)
{
	int nbytes;
	ioctl(dpy->fd, FIONREAD, &nbytes);

	while (nbytes) {
		char dummy[16];
		int rd = read(dpy->fd, dummy, min_c(nbytes, sizeof(dummy)));
		if (rd > 0)
			nbytes -= rd;
	}

	return Success;
}

extern "C" int
XSync(Display* display, Bool discard)
{
	XFlush(display);
	if (discard) {
		XEvent dummy;
		while (QLength(display))
			XNextEvent(display, &dummy);
	}
	return Success;
}

extern "C" int
XEventsQueued(Display* display, int mode)
{
	if (mode != QueuedAlready && !QLength(display))
		XFlush(display);
	return QLength(display);
}

extern "C" int
XPending(Display* display)
{
	return XEventsQueued(display, QueuedAfterFlush);
}
