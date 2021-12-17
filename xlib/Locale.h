#pragma once

#include <support/String.h>

extern "C" {
#include <X11/Xutil.h>
}

BString _x_text_decode(const XTextProperty* prop);
