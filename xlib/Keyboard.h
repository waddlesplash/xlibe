#pragma once

#include <app/Message.h>

extern "C" {
#include <X11/Xlib.h>
}

int _x_get_button_state(BMessage* message);
int _x_get_button_state(int32 modifiers = -1, int32 mouseButtons = -1);

void _x_fill_key_event(XEvent* event, BMessage* message, const char* bytes, int32 numBytes);
