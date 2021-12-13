#include "Drawables.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include "Debug.h"

extern "C" int
XFindContext(Display* display, XID rid, XContext context, XPointer* data_return)
{
	// We only support attaching contexts to drawables.
	XDrawable* drawable = Drawables::get(rid);
	if (!drawable)
		return BadDrawable;

	const auto& it = drawable->contexts.find(context);
	if (it != drawable->contexts.end()) {
		*data_return = it->second;
		return 0;
	}
	return XCNOENT;
}

extern "C" int
XSaveContext(Display* display, XID rid, XContext context, const char* data)
{
	XDrawable* drawable = Drawables::get(rid);
	if (!drawable)
		return BadDrawable;

	drawable->contexts.insert({context, (XPointer)data});
	return 0;
}

extern "C" int
XDeleteContext(Display* display, XID rid, XContext context)
{
	XDrawable* drawable = Drawables::get(rid);
	if (!drawable)
		return BadDrawable;

	drawable->contexts.erase(context);
	return 0;
}
