/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#include "Drawables.h"

#include <support/Autolock.h>
#include <support/Locker.h>
#include <sys/ioctl.h>
#include <list>
#include <functional>

#include "Property.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

namespace {
class Events {
private:
	Display* _display;
	BLocker lock_;
	std::list<XEvent> list_;

	Events(Display* display);
	void wait_for_more();

public:
	static bool is_match(long mask, long event);

	static void init_for(Display* display);
	static Events& instance_for(Display* display);

	void add(XEvent event, bool front = false);
	void wait_for_next(XEvent* event_return, bool dequeue = true);

	/* if and only if 'wait' is false can this function return false, i.e. no event found */
	bool query(std::function<bool(const XEvent&)> condition,
		XEvent* event_return, bool wait, bool dequeue = true);
};
}

Events&
Events::instance_for(Display* display)
{
	return *(Events*)display->trans_conn;
}

void
_x_init_events(Display* dpy)
{
	Events::init_for(dpy);
}

void
_x_finalize_events(Display* dpy)
{
	delete (Events*)dpy->trans_conn;
}

void
Events::init_for(Display* display)
{
	if (!display->trans_conn)
		display->trans_conn = (typeof(display->trans_conn))new Events(display);
}

Events::Events(Display* display)
	: _display(display)
	, lock_("XEvents")
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
	case ConfigureNotify:
	case DestroyNotify:
	case UnmapNotify:
	case MapNotify:
	case ReparentNotify:
	case GravityNotify:
	case CirculateNotify:
		return (mask & StructureNotifyMask);
	case CreateNotify:
		return (mask & SubstructureNotifyMask);
	case MapRequest:
	case ConfigureRequest:
	case CirculateRequest:
		return (mask & SubstructureRedirectMask);
	case ResizeRequest:
		return (mask & ResizeRedirectMask);
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

void
Events::add(XEvent event, bool front)
{
	event.xany.display = _display;

	BAutolock evl(lock_);
	_display->last_request_read = _display->request;
	event.xany.serial = _display->request++;
	_display->qlen++;
	if (front)
		list_.push_front(event);
	else
		list_.push_back(event);
	evl.Unlock();

	char dummy[1];
	write(_display->conn_checker, dummy, 1);
}

void
_x_put_event(Display* display, const XEvent& event)
{
	Events::instance_for(display).add(event);
}

void
Events::wait_for_more()
{
	char dummy[1];
	read(_display->fd, dummy, 1);
}

void
Events::wait_for_next(XEvent* event_return, bool dequeue)
{
	if (!_display->qlen)
		wait_for_more();

	BAutolock evl(lock_);
	*event_return = list_.front();
	if (dequeue) {
		list_.pop_front();
		_display->qlen--;
	}
}

bool
Events::query(std::function<bool(const XEvent&)> condition, XEvent* event,
	bool wait, bool dequeue)
{
	while (true) {
		BAutolock evl(lock_);
		for (auto i = list_.begin(); i != list_.end(); i++) {
			if (!condition(*i))
				continue;

			*event = (*i);
			if (dequeue) {
				list_.erase(i);
				_display->qlen--;
			}
			return true;
		}
		evl.Unlock();

		if (!wait)
			return false;
		wait_for_more();
	}
	return false;
}

extern "C" int
XSelectInput(Display* display, Window w, long mask)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;
	window->event_mask(mask);
	return Success;
}

extern "C" int
XPeekEvent(Display* display, XEvent* event)
{
	XFlush(display);
	Events::instance_for(display).wait_for_next(event, false);
	return Success;
}

extern "C" int
XNextEvent(Display* display, XEvent* event)
{
	XFlush(display);
	Events::instance_for(display).wait_for_next(event);
	return Success;
}

extern "C" int
XMaskEvent(Display* display, long event_mask, XEvent* event_return)
{
	XFlush(display);
	Events::instance_for(display).query([event_mask](const XEvent& event) {
		return Events::is_match(event_mask, event.type);
	}, event_return, true);
	return Success;
}

extern "C" int
XSendEvent(Display* display, Window w, Bool propagate, long event_mask, XEvent* event_send)
{
	if (w == DefaultRootWindow(display)) {
		_x_handle_send_root(display, *event_send);
		return 0;
	}

	XWindow* window = Drawables::get_window(w);
	if (w == PointerWindow)
		window = Drawables::pointer();
	else if (w == InputFocus)
		window = Drawables::focused();
	if (!window)
		return 0;

	if (!Events::is_match(window->event_mask(), event_send->type))
		return 1;

	event_send->xany.display = display;
	event_send->xany.window = w;
	event_send->xany.send_event = True;
	_x_put_event(display, *event_send);
	return 1;
}

extern "C" int
XPutBackEvent(Display* display, XEvent* event)
{
	Events::instance_for(display).add(*event, true);
	return Success;
}

extern "C" Bool
XIfEvent(Display* display, XEvent* event_return,
	Bool (*predicate)(Display*, XEvent*, XPointer), XPointer arg)
{
	XFlush(display);
	Events::instance_for(display).query([predicate, arg](const XEvent& event) {
		return predicate(event.xany.display, (XEvent*)&event, arg);
	}, event_return, true);
	return Success;
}

extern "C" Bool
XPeekIfEvent(Display* display, XEvent* event_return,
	Bool (*predicate)(Display*, XEvent*, XPointer), XPointer arg)
{
	XFlush(display);
	Events::instance_for(display).query([predicate, arg](const XEvent& event) {
		return predicate(event.xany.display, (XEvent*)&event, arg);
	}, event_return, true, false);
	return Success;
}

extern "C" Bool
XWindowEvent(Display* display, Window w, long event_mask, XEvent* event_return)
{
	XFlush(display);
	Events::instance_for(display).query([w, event_mask](const XEvent& event) {
		return (event.xany.window == w && Events::is_match(event_mask, event.type));
	}, event_return, true);
	return Success;
}

extern "C" Bool
XCheckMaskEvent(Display* display, long event_mask, XEvent* event_return)
{
	XFlush(display);
	bool found = Events::instance_for(display).query([event_mask](const XEvent& event) {
		return Events::is_match(event_mask, event.type);
	}, event_return, false);
	return found ? True : False;
}

extern "C" Bool
XCheckTypedWindowEvent(Display* display, Window w, int event_type, XEvent* event_return)
{
	XFlush(display);
	bool found = Events::instance_for(display).query([w, event_type](const XEvent& event) {
		return (event.type == event_type && (w == ~((Window)0) || event.xany.window == w));
	}, event_return, false);
	return found ? True : False;
}

extern "C" Bool
XCheckTypedEvent(Display* display, int event_type, XEvent* event_return)
{
	return XCheckTypedWindowEvent(display, ~((Window)0), event_type, event_return);
}

extern "C" Bool
XCheckWindowEvent(Display* display, Window w, long event_mask, XEvent* event_return)
{
	XFlush(display);
	bool found = Events::instance_for(display).query([w, event_mask](const XEvent& event) {
		return (event.xany.window == w && Events::is_match(event_mask, event.type));
	}, event_return, false);
	return found ? True : False;
}

extern "C" Bool
XCheckIfEvent(Display* display, XEvent* event_return,
	Bool (*predicate)(Display*, XEvent*, XPointer), XPointer arg)
{
	XFlush(display);
	bool found = Events::instance_for(display).query([predicate, arg](const XEvent& event) {
		return predicate(event.xany.display, (XEvent*)&event, arg);
	}, event_return, false);
	return found ? True : False;
}

extern "C" int
XFlush(Display* dpy)
{
	// We only have the "input buffer" to flush.
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
