#pragma once

extern "C" {
#include <X11/Xlib.h>
}

void x_extensions_close(Display *dpy);
