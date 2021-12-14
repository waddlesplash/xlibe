#include <stdio.h>

#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
}

extern "C" int
XGetWindowProperty(Display *display, Window w, Atom property,
	long long_offset, long long_length, Bool del,
	Atom req_type, Atom* actual_type_return,
	int* actual_format_return, unsigned long* nitems_return,
	unsigned long* bytes_after_return,
	unsigned char** prop_return)
{
	UNIMPLEMENTED();
	*nitems_return = 0;
	*prop_return = NULL;
	return BadImplementation;
}

extern "C" Status
XGetTextProperty(Display *display, Window w,
	XTextProperty* text_prop_return, Atom property)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" void
XSetTextProperty(Display *display, Window w,
	XTextProperty *text_prop, Atom property)
{
	UNIMPLEMENTED();
}

extern "C" int
XChangeProperty(Display *display, Window w, Atom property, Atom type,
	int format, int mode, const unsigned char *data, int nelements)
{
	if (type == XA_ATOM) {
		fprintf(stderr, "UNIMPLEMENTED: XChangeProperty: %s = %s\n", XGetAtomName(display, property),
			XGetAtomName(display, *(Atom*)data));
	} else {
		fprintf(stderr, "UNIMPLEMENTED: XChangeProperty: %s(%s)\n", XGetAtomName(display, property),
			XGetAtomName(display, type));
	}
	return BadImplementation;
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
