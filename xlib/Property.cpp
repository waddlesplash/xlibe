/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#include "Property.h"

#include <cstdio>

#include "Atom.h"
#include "Debug.h"
#include "Drawables.h"
#include "Selection.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
}

static void
unknown_property(const char* format, Atom atom1, Atom atom2 = None)
{
	char* value1 = XGetAtomName(NULL, atom1);
	char* value2 = atom2 != None ? XGetAtomName(NULL, atom2) : NULL;
	if (value2)
		fprintf(stderr, format, value1, value2);
	else
		fprintf(stderr, format, value1);
	free(value1);
	free(value2);
}

static void
_x_property_notify(XWindow* window, Atom property, int state)
{
	if (!window || !(window->event_mask() & PropertyChangeMask))
		return;

	XEvent event = {};
	event.type = PropertyNotify;
	event.xproperty.window = window->id();
	event.xproperty.time = _x_current_time();
	event.xproperty.atom = property;
	event.xproperty.state = state;
	_x_put_event(window->display(), event);
}

extern "C" int
XGetWindowProperty(Display* dpy, Window w, Atom property,
	long long_offset, long long_length, Bool del,
	Atom req_type, Atom* actual_type_return,
	int* actual_format_return, unsigned long* nitems_return,
	unsigned long* bytes_after_return,
	unsigned char** prop_return)
{
	if (property == Atoms::_MOTIF_WM_HINTS) {
		// Hack so GTK does not crash.
		*actual_type_return = Atoms::_MOTIF_WM_HINTS;
		*prop_return = (unsigned char*)calloc(sizeof(long),4);
		*nitems_return = 4;
		return Success;
	}
	unknown_property("libX11: unhandled Property (get): %s<%s>\n", property, req_type);

	*nitems_return = 0;
	*prop_return = NULL;
	return BadImplementation;
}

extern "C" Status
XGetTextProperty(Display *display, Window w,
	XTextProperty* text_prop_return, Atom property)
{
	Atom actual_type;
	int actual_format = 0;
	unsigned long nitems = 0L, leftover = 0L;
	unsigned char* data = NULL;

	if (XGetWindowProperty(display, w, property, 0L, -1, False,
			AnyPropertyType, &actual_type, &actual_format,
			&nitems, &leftover, &data) == Success && actual_type != None) {
		*text_prop_return = make_text_property(actual_type, actual_format, data, nitems);
		return True;
	}

	*text_prop_return = make_text_property(None, 0, NULL, 0);
	return False;
}

extern "C" int
XChangeProperty(Display* dpy, Window w, Atom property, Atom type,
	int format, int mode, const unsigned char* data, int nelements)
{
	// TODO: mode?

	switch (property) {
	case Atoms::WM_PROTOCOLS:
		return XSetWMProtocols(dpy, w, (Atom*)data, nelements);

	case XA_WM_NAME:
	case Atoms::WM_NAME:
	case Atoms::_NET_WM_NAME: {
		XTextProperty tp = make_text_property(type, format, data, nelements);
		XSetWMName(dpy, w, &tp);
		return Success;
	}
	case XA_WM_ICON_NAME:
	case Atoms::WM_ICON_NAME:
	case Atoms::_NET_WM_ICON_NAME: {
		XTextProperty tp = make_text_property(type, format, data, nelements);
		XSetWMIconName(dpy, w, &tp);
		return Success;
	}

	case Atoms::_MOTIF_WM_HINTS: {
		/* The _MOTIF_WM_HINTS property is widely used but not very well documented.
		 * Most producers and consumers of this API seem to copy and paste the definitions
		 * from Motif's headers, but the precise semantics are not very well defined. */
		enum {
			MWM_HINTS_FUNCTIONS		= (1L << 0),
			MWM_HINTS_DECORATIONS	= (1L << 1),
			MWM_HINTS_INPUT_MODE	= (1L << 2),
			MWM_HINTS_STATUS		= (1L << 3),
		};
		enum {
			MWM_FUNC_ALL			= (1L << 0),
			MWM_FUNC_RESIZE			= (1L << 1),
			MWM_FUNC_MOVE			= (1L << 2),
			MWM_FUNC_MINIMIZE		= (1L << 3),
			MWM_FUNC_MAXIMIZE		= (1L << 4),
			MWM_FUNC_CLOSE			= (1L << 5),
		};
		enum {
			MWM_DECOR_ALL			= (1L << 0),
			MWM_DECOR_BORDER		= (1L << 1),
			MWM_DECOR_RESIZEH		= (1L << 2),
			MWM_DECOR_TITLE			= (1L << 3),
			MWM_DECOR_MENU			= (1L << 4),
			MWM_DECOR_MINIMIZE		= (1L << 5),
			MWM_DECOR_MAXIMIZE		= (1L << 6),
		};
		enum {
			MWM_INPUT_MODELESS = 0,
			MWM_INPUT_PRIMARY_APPLICATION_MODAL = 1,
			MWM_INPUT_SYSTEM_MODAL = 2,
			MWM_INPUT_FULL_APPLICATION_MODAL = 3,
		};

		if (type != Atoms::_MOTIF_WM_HINTS || nelements < 1)
			return BadValue;

		XWindow* window = Drawables::get_window(w);
		if (!window || !window->bwindow)
			return BadWindow;
		BWindow* bwindow = window->bwindow;

		long* values = (long*)data;
		const long flags = values[0];
		if (flags & MWM_HINTS_FUNCTIONS) {
			long func = values[1];
			if (func & MWM_FUNC_ALL)
				func = ~0L;

			int32 bflags = 0;
			if (!(func & MWM_FUNC_RESIZE))
				bflags |= B_NOT_RESIZABLE;
			if (!(func & MWM_FUNC_MOVE))
				bflags |= B_NOT_MOVABLE;
			if (!(func & MWM_FUNC_MINIMIZE))
				bflags |= B_NOT_MINIMIZABLE;
			if (!(func & MWM_FUNC_MAXIMIZE))
				bflags |= B_NOT_ZOOMABLE;
			if (!(func & MWM_FUNC_CLOSE))
				bflags |= B_NOT_CLOSABLE;

			int32 setflags = bwindow->Flags();
			setflags &= ~(B_NOT_RESIZABLE | B_NOT_MOVABLE | B_NOT_MINIMIZABLE
				| B_NOT_ZOOMABLE | B_NOT_CLOSABLE);
			setflags |= bflags;
			bwindow->SetFlags(setflags);
		}
		if (flags & MWM_HINTS_DECORATIONS) {
			long decor = values[2];
			if (decor & MWM_DECOR_ALL)
				decor = ~0L;

			// We ignore most of these and rely instead on MWM_HINTS_FUNCTIONS.
			bwindow->SetLook((decor & MWM_DECOR_TITLE) ? B_TITLED_WINDOW_LOOK :
				(decor & MWM_DECOR_BORDER) ? B_BORDERED_WINDOW_LOOK : B_NO_BORDER_WINDOW_LOOK);
		}
		if ((flags & MWM_HINTS_INPUT_MODE) && !window->override_redirect) {
			long input = values[3];
			switch (input) {
			case MWM_INPUT_MODELESS:
				bwindow->SetFeel(B_NORMAL_WINDOW_FEEL);
				break;

			case MWM_INPUT_PRIMARY_APPLICATION_MODAL:
			case MWM_INPUT_FULL_APPLICATION_MODAL:
				bwindow->SetFeel(B_MODAL_APP_WINDOW_FEEL);
				break;

			case MWM_INPUT_SYSTEM_MODAL:
				bwindow->SetFeel(B_MODAL_ALL_WINDOW_FEEL);
				break;
			}
		}
		if (flags & MWM_HINTS_STATUS) {
			// TODO: What is this for?
		}

		return Success;
	}
	case Atoms::_NET_WM_WINDOW_TYPE: {
		if (type != XA_ATOM || nelements != 1)
			return BadValue;
		XWindow* window = Drawables::get_window(w);
		if (!window || !window->bwindow)
			return BadWindow;
		BWindow* bwindow = window->bwindow;

		switch (*(Atom*)data) {
		case Atoms::_NET_WM_WINDOW_TYPE_DROPDOWN_MENU:
		case Atoms::_NET_WM_WINDOW_TYPE_POPUP_MENU:
			bwindow->SetLook(B_NO_BORDER_WINDOW_LOOK);
			if (!window->override_redirect) {
				bwindow->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
				bwindow->SetFlags(bwindow->Flags() | B_AVOID_FOCUS);
			}
			break;

		default:
			unknown_property("libX11: unhandled _NET_WM_WINDOW_TYPE: %s\n", *(Atom*)data);
			// fall through
		case Atoms::_NET_WM_WINDOW_TYPE_NORMAL:
			bwindow->SetLook(B_TITLED_WINDOW_LOOK);
			if (!window->override_redirect) {
				bwindow->SetFeel(B_NORMAL_WINDOW_FEEL);
				bwindow->SetFlags(bwindow->Flags() & ~B_AVOID_FOCUS);
			}
			break;
		}

		return Success;
	}
	case Atoms::_NET_WM_STATE: {
		if (type != XA_ATOM || nelements < 1)
			return BadValue;

		XWindow* window = Drawables::get_window(w);
		if (!window || !window->bwindow)
			return BadWindow;

		Atom* values = (Atom*)data;
		for (int i = 0; i < nelements; i++) {
			switch (values[i]) {
			case Atoms::_NET_WM_STATE_MODAL:
				window->bwindow->SetFeel(window->transient_for ?
					B_MODAL_SUBSET_WINDOW_FEEL : B_MODAL_APP_WINDOW_FEEL);
				window->bwindow->Activate();
				break;

			default:
				unknown_property("libX11: unhandled _NET_WM_STATE: %s\n", values[i]);
				break;
			}
		}

		// Adding a window to a subset only works if it has a SUBSET feel.
		// So we add it (again) after changing feels.
		if (XWindow* transient_for = Drawables::get_window(window->transient_for))
			transient_for->bwindow->AddToSubset(window->bwindow);

		return Success;
	}

	case Atoms::CLIPBOARD:
		return _x_handle_set_clipboard(dpy, w, type, data, nelements);

	default: {
		if (type == XA_ATOM && nelements)
			unknown_property("libX11: unhandled Property: %s = %s\n", property, *(Atom*)data);
		else
			unknown_property("libX11: unhandled Property: %s<%s>\n", property, type);

		// Send a property-change event anyway.
		// (Some applications set a bogus property to get the time field from the reply event.)
		_x_property_notify(Drawables::get_window(w), property, PropertyDelete);
		break;
	}
	}
	return BadImplementation;
}

extern "C" void
XSetTextProperty(Display* dpy, Window w,
	XTextProperty* text_prop, Atom property)
{
	XChangeProperty(dpy, w, property, text_prop->encoding, text_prop->format,
		PropModeReplace, text_prop->value, text_prop->nitems);
}

extern "C" Atom*
XListProperties(Display* dpy, Window w,
	int* num_prop_return)
{
	UNIMPLEMENTED();
	*num_prop_return = 0;
	return NULL;
}

extern "C" int
XDeleteProperty(Display* display, Window w, Atom property)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

void
_x_handle_send_root(Display* dpy, const XEvent& event)
{
	if (event.type != ClientMessage) {
		fprintf(stderr, "libX11: unhandled sent root event, type %d\n", event.type);
		return;
	}

	switch (event.xclient.message_type) {
	case Atoms::_NET_WM_STATE: {
		XWindow* window = Drawables::get_window(event.xclient.window);
		if (!window || !window->bwindow)
			return;

		enum {
			_NET_WM_STATE_REMOVE = 0,
			_NET_WM_STATE_ADD = 1,
			_NET_WM_STATE_TOGGLE = 2,
		};
		const int32 action = event.xclient.data.l[0];

		for (int i = 1; i < 3; i++) {
			const Atom value = event.xclient.data.l[i];
			if (value == None)
				continue;

			if (action == _NET_WM_STATE_ADD) {
				XChangeProperty(dpy, event.xclient.window, Atoms::_NET_WM_STATE, XA_ATOM, -1,
					PropModeAppend, (unsigned char*)&value, 1);
			} else {
				// FIXME: This is not correct at all!
				XDeleteProperty(dpy, event.xclient.window, Atoms::_NET_WM_STATE);
			}
		}
		break;
	}

	default:
		unknown_property("libX11: unhandled sent root message: %s\n", event.xclient.message_type);
		break;
	}
}
