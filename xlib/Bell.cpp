#include <be/support/Beep.h>

extern "C" {
#include <X11/Xlib.h>
}

extern "C" int
XBell(Display* dpy, int percent)
{
	beep();
	return 0;
}
