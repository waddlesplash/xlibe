/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

extern "C" {
#include <X11/Xlib.h>
}

int _x_handle_get_settings(Display* dpy, Window w,
	Atom* actual_type_return, int* actual_format_return, unsigned long* nitems_return,
	unsigned char** prop_return);
