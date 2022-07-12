/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <interface/Font.h>

extern "C" {
#include <X11/Xlib.h>
}

void _x_init_font();
void _x_finalize_font();

BFont _bfont_from_font(Font font);
Font _font_from_fontset(XFontSet set);
