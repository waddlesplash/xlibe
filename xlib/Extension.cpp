#include <map>

#include "Extension.h"
#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

static int sLastExtension = 1;

static std::map<int, _XExtension*> sExtensions;

extern "C" XExtCodes*
XAddExtension(Display* dpy)
{
	const int id = sLastExtension++;
	_XExtension* extension = new _XExtension;
	extension->codes.extension = id;

	// arbitrary
	extension->codes.major_opcode = id;
	extension->codes.first_event = id * 100000;
	extension->codes.first_error = id * 100000;
	sExtensions.insert({id, extension});
	return &extension->codes;
}

extern "C" CloseDisplayType
XESetCloseDisplay(Display* dpy, int extension_id, CloseDisplayType proc)
{
	const auto& it = sExtensions.find(extension_id);
	if (it == sExtensions.end())
		return NULL;

	CloseDisplayType last = it->second->close_display;
	it->second->close_display = proc;
	return last;
}

void
_x_extensions_close(Display* dpy)
{
	for (const auto& it : sExtensions) {
		if (it.second->close_display)
			it.second->close_display(dpy, &it.second->codes);
	}
	sExtensions.clear();
}

extern "C" char**
XListExtensions(Display* dpy, int* nextensions_return)
{
	UNIMPLEMENTED();
	return NULL;
}

extern "C" int
XFreeExtensionList(char** list)
{
	return Success;
}
