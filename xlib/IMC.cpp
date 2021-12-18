/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

extern "C" {
#include <X11/Xlib.h>
}

extern "C" XIM
XOpenIM(Display* dpy,
	struct _XrmHashBucketRec* rdb, char* res_name, char* res_class)
{
	// Unimplemented.
	return NULL;
}

extern "C" Display*
XDisplayOfIM(XIM im)
{
	return NULL;
}

extern "C" char*
XLocaleOfIM(XIM im)
{
	return NULL;
}

extern "C" char*
XGetIMValues(XIM im, ...)
{
	return NULL;
}

extern "C" char*
XSetIMValues(XIM im, ...)
{
	return NULL;
}

extern "C" Bool
XRegisterIMInstantiateCallback(Display* dpy,
	struct _XrmHashBucketRec* rdb, char* res_name, char* res_class,
	XIDProc callback, XPointer client_data)
{
	return False;
}

extern "C" Bool
XUnregisterIMInstantiateCallback(Display* dpy,
	struct _XrmHashBucketRec* rdb, char* res_name, char* res_class,
	XIDProc callback, XPointer client_data)
{
	return False;
}

extern "C" Status
XCloseIM(XIM xim)
{
	return 0;
}

extern "C" XIC
XCreateIC(XIM xim, ...)
{
	// Unimplemented.
	return NULL;
}

extern "C" XIM
XIMOfIC(XIC ic)
{
	return NULL;
}

extern "C" char*
XmbResetIC(XIC ic)
{
	return NULL;
}

extern "C" void
XDestroyIC(XIC ic)
{
}

extern "C" char*
XSetICValues(XIC ic, ...)
{
	return NULL;
}

extern "C" char*
XGetICValues(XIC ic, ...)
{
	return NULL;
}

extern "C" void
XSetICFocus(XIC ic)
{
}

extern "C" void
XUnsetICFocus(XIC ic)
{
}

extern "C" Bool
XFilterEvent(XEvent *event, Window window)
{
	return 0;
}

extern "C" int
Xutf8LookupString(XIC ic, XKeyPressedEvent* event,
	char* buffer_return, int bytes_buffer, KeySym* keysym_return, Status* status_return)
{
	return BadImplementation;
}

extern "C" XVaNestedList
XVaCreateNestedList(int dummy, ...)
{
	return NULL;
}
