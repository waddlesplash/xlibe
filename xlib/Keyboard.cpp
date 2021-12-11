#include <interface/InterfaceDefs.h>
#include <cstdlib>

#include <private/shared/Keymap.h>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
}

#include "Debug.h"
#include "keysymlist.h"

extern "C" Display*
XkbOpenDisplay(char *display_name, int *event_rtrn, int *error_rtrn,
	int *major_in_out, int *minor_in_out, int *reason_rtrn)
{
	return XOpenDisplay(display_name);
}

extern "C" int
XLookupString(XKeyEvent* key_event, char* buffer_return, int bytes_buffer,
	KeySym* keysym_return, XComposeStatus *status_in_out)
{
	*keysym_return = XLookupKeysym(key_event, 0);

	int copybytes = strnlen((const char*)&key_event->serial, sizeof(key_event->serial));
	copybytes = min_c(copybytes, bytes_buffer);
	if (copybytes <= 0)
		return 0;
	memcpy(buffer_return, &key_event->serial, copybytes);
	return copybytes;
}

extern "C" KeySym
XkbKeycodeToKeysym(Display* dpy, unsigned int kc, int group, int level)
{
	// TODO: Fill this in!
	switch (kc) {
	case B_BACKSPACE:	return XK_BackSpace;
	case B_RETURN:		return XK_Return;
	case B_SPACE:		return XK_space;
	case B_TAB:			return XK_Tab;
	case B_ESCAPE:		return XK_Escape;
	}
	return NoSymbol;
}

extern "C" KeySym
XLookupKeysym(XKeyEvent* key_event, int index)
{
	return XkbKeycodeToKeysym(key_event->display, key_event->keycode, 0, 0);
}

extern "C" KeyCode
XKeysymToKeycode(Display *display, KeySym keysym)
{
	UNIMPLEMENTED();
	return 0;
}

extern "C" XModifierKeymap*
XGetModifierMapping(Display* display)
{
	XModifierKeymap* map = (XModifierKeymap*)calloc(1, sizeof(XModifierKeymap));
	// Unfortunately B_LEFT_SHIFT_KEY and friends are larger than UCHAR_MAX.
	map->max_keypermod = 1;
	map->modifiermap = (KeyCode*)calloc(16, sizeof(KeyCode));
	map->modifiermap[ShiftMapIndex] = B_SHIFT_KEY;
	map->modifiermap[LockMapIndex] = B_CAPS_LOCK;
	map->modifiermap[ControlMapIndex] = B_COMMAND_KEY;
	map->modifiermap[Mod1MapIndex] = B_CONTROL_KEY;
	map->modifiermap[Mod2MapIndex] = B_NUM_LOCK;
	map->modifiermap[Mod3MapIndex] = B_SCROLL_LOCK;
	return map;
}

extern "C" int
XFreeModifiermap(XModifierKeymap *modmap)
{
	free(modmap->modifiermap);
	free(modmap);
	return Success;
}

extern "C" KeySym
XStringToKeysym(const char* string)
{
	if (string == NULL)
		return NoSymbol;
	if (strlen(string) == 1) {
		char chr = string[0];
		if (chr >= '0' && chr <= '9' && chr >= 'a' && chr <= 'z' && chr >= 'A' && chr <= 'Z')
			return (KeySym) ((long) chr);
	}

	for (int i = 0; i < KEY_SYM_LIST_LENGTH; i++) {
		if (strcmp(KEY_SYM_LIST[i].name, string) == 0) {
			return KEY_SYM_LIST[i].keySym;
		}
	}
	return NoSymbol;
}


extern "C" char*
XKeysymToString(KeySym keysym)
{
	if (keysym >= XK_0 && keysym <= XK_9 && keysym >= XK_a && keysym <= XK_z
			&& keysym >= XK_A && keysym <= XK_Z) {
		// TODO: Return char for keysym...
		return NULL;
	}

	for (int i = 0; i < KEY_SYM_LIST_LENGTH; i++) {
		if (KEY_SYM_LIST[i].keySym == keysym) {
			return (char*) KEY_SYM_LIST[i].name;
		}
	}
	return NULL;
}

// #pragma mark - stubs

extern "C" int
XDisplayKeycodes(Display*, int*, int*)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XGetKeyboardControl(Display* dpy, XKeyboardState* state_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XQueryKeymap(Display* dpy, char keys_return[32])
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" Status
XkbGetState(Display* dpy, unsigned int deviceSpec, XkbStatePtr rtrnState)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" XkbDescPtr
XkbGetKeyboard(Display* display, unsigned int which, unsigned int device_spec)
{
	UNIMPLEMENTED();
	return NULL;
}

extern "C" void
XkbFreeKeyboard(XkbDescPtr xkb, unsigned int which, Bool freeDesc)
{
	UNIMPLEMENTED();
}

extern "C" Bool
XkbLibraryVersion(int *lib_major_in_out, int *lib_minor_in_out)
{
	return True;
}

extern "C" Bool
XkbQueryExtension(Display *dpy, int *opcode_rtrn, int *event_rtrn,
	int *error_rtrn, int *major_in_out, int *minor_in_out)
{
	return True;
}

extern "C" int
XGrabKeyboard(Display *display, Window grab_window, Bool owner_events,
	int pointer_mode, int keyboard_mode, Time time)
{
	UNIMPLEMENTED();
	// TODO?
	return BadImplementation;
}

extern "C" int
XUngrabKeyboard(Display *display, Time time)
{
	UNIMPLEMENTED();
	return Success;
}
