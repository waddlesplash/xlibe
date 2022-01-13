/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

extern "C" {
#include <X11/Xlib.h>
}

void _x_handle_send_root_selection(Display* dpy, const XEvent& event);
Status _x_handle_set_clipboard(Display* dpy, Window w, Atom type, const unsigned char* data, int nelements);
