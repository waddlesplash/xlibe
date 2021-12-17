#include "Property.h"

#include <cstdio>

#include "Atom.h"
#include "Debug.h"
#include "Drawables.h"

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

extern "C" int
XGetWindowProperty(Display* dpy, Window w, Atom property,
	long long_offset, long long_length, Bool del,
	Atom req_type, Atom* actual_type_return,
	int* actual_format_return, unsigned long* nitems_return,
	unsigned long* bytes_after_return,
	unsigned char** prop_return)
{
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

	case Atoms::_NET_WM_NAME: {
		XTextProperty tp = make_text_property(type, format, data, nelements);
		XSetWMName(dpy, w, &tp);
		return Success;
	}
	case Atoms::_NET_WM_ICON_NAME: {
		XTextProperty tp = make_text_property(type, format, data, nelements);
		XSetWMIconName(dpy, w, &tp);
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
			bwindow->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
			bwindow->SetFlags(bwindow->Flags() | B_AVOID_FOCUS);
			break;

		default:
			unknown_property("libX11: unhandled _NET_WM_WINDOW_TYPE: %s\n", *(Atom*)data);
			// fall through
		case Atoms::_NET_WM_WINDOW_TYPE_NORMAL:
			bwindow->SetLook(B_TITLED_WINDOW_LOOK);
			bwindow->SetFeel(B_NORMAL_WINDOW_FEEL);
			bwindow->SetFlags(bwindow->Flags() & ~(B_AVOID_FOCUS));
			break;
		}

		return Success;
	}

	default: {
		if (type == XA_ATOM && nelements)
			unknown_property("libX11: unhandled Property: %s = %s\n", property, *(Atom*)data);
		else
			unknown_property("libX11: unhandled Property: %s<%s>\n", property, type);
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

extern "C" int
XDeleteProperty(Display *display, Window w, Atom property)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" Status
Xutf8TextPropertyToTextList(Display* display, const XTextProperty* tp,
	char*** list_return, int* count_return)
{
	return XTextPropertyToStringList((XTextProperty*)tp, list_return, count_return);
}
