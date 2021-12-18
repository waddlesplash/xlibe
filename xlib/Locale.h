/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <support/String.h>

extern "C" {
#include <X11/Xutil.h>
}

BString _x_text_decode(const XTextProperty* prop);
