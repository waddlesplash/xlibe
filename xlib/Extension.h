/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

extern "C" {
#include <X11/Xlib.h>
}

void _x_extensions_close(Display *dpy);
