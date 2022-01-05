/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

extern "C" {
#include <X11/Xlib.h>
}

#define _x_current_time() (Time(system_time() / 1000))

void _x_init_events(Display* dpy);
void _x_finalize_events(Display* dpy);

void _x_put_event(Display* dpy, const XEvent& event);
