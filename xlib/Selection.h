/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

extern "C" {
#include <X11/Xlib.h>
}

void _x_handle_send_root_selection(Display* dpy, const XEvent& event);
bool _x_handle_get_clipboard(Display* dpy, Window w, Atom property,
	Atom* actual_type_return, int* actual_format_return, unsigned long* nitems_return,
	unsigned char** prop_return);
Status _x_handle_set_clipboard(Display* dpy, Window w, Atom type, const unsigned char* data, int nelements);
