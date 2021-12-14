#include <stdio.h>

#include "Atom.h"
#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
}

extern "C" int
XGetWindowProperty(Display* dpy, Window w, Atom property,
	long long_offset, long long_length, Bool del,
	Atom req_type, Atom* actual_type_return,
	int* actual_format_return, unsigned long* nitems_return,
	unsigned long* bytes_after_return,
	unsigned char** prop_return)
{
	char* propertyName = XGetAtomName(dpy, property), *reqTypeName = XGetAtomName(dpy, req_type);
	fprintf(stderr, "UNIMPLEMENTED: XGetWindowProperty: %s(%s)\n", propertyName, reqTypeName);
	free(propertyName);
	free(reqTypeName);

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
XChangeProperty(Display* dpy, Window w, Atom property, Atom type,
	int format, int mode, const unsigned char *data, int nelements)
{
	char* propertyName = XGetAtomName(dpy, property);
	if (type == XA_ATOM) {
		char* value = XGetAtomName(dpy, *(Atom*)data);
		fprintf(stderr, "UNIMPLEMENTED: XChangeProperty: %s = %s\n", propertyName, value);
		free(value);
	} else {
		char* typeName = XGetAtomName(dpy, type);
		fprintf(stderr, "UNIMPLEMENTED: XChangeProperty: %s(%s)\n", propertyName, typeName);
		free(typeName);
	}
	free(propertyName);
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
