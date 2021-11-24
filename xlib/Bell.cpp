#include "X11/Xlib.h"
#include "be/support/Beep.h"

int XBell(Display *dpy, int percent) {
	beep();
	return 0;
}
