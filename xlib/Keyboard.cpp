#include <interface/InterfaceDefs.h>
#include <cstdlib>

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
XLookupString(XKeyEvent* event_struct, char* buffer_return, int bytes_buffer,
		KeySym* keysym_return, XComposeStatus *status_in_out)
{
	UNIMPLEMENTED();
	// FIXME: Implement!
	*buffer_return = event_struct->keycode;
	*keysym_return = -1;
	return 1;
}

extern "C" KeySym
XLookupKeysym(XKeyEvent* key_event, int index)
{
	// FIXME: Implement!
	return 0;
}

extern "C" KeyCode
XKeysymToKeycode(Display *display, KeySym keysym)
{
	UNIMPLEMENTED();
	// FIXME: Implement!
	return 0;
}

extern "C" KeySym
XkbKeycodeToKeysym(Display* dpy, unsigned int kc, int group, int level)
{
	UNIMPLEMENTED();
	return NoSymbol;
}

extern "C" XModifierKeymap*
XGetModifierMapping(Display* display)
{
	XModifierKeymap* map = (XModifierKeymap*)calloc(1, sizeof(XModifierKeymap));
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
	free((char *) modmap->modifiermap);
	free((char *) modmap);
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

extern "C" int
XDisplayKeycodes(Display*, int*, int*)
{
	UNIMPLEMENTED();
	return BadImplementation;
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
