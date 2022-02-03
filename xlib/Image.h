/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <interface/Bitmap.h>

extern "C" {
#include <X11/Xlib.h>
}

BBitmap* _bbitmap_for_ximage(XImage* image, uint32 flags = 0);
