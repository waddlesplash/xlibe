/*
 * Copyright 2003, Shibukawa Yoshiki. All rights reserved.
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#include <support/Beep.h>

extern "C" {
#include <X11/Xlib.h>
}

extern "C" int
XBell(Display* dpy, int percent)
{
	beep();
	return 0;
}
