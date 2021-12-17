#include "Keyboard.h"

#include <cstdlib>
#include <interface/InterfaceDefs.h>
#include <interface/View.h>

#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include "tables/keysymlist.h"
}

// Unfortunately we cannot store an XK_* inside the KeyEvent, because
// despite xkey.keycode being a 32-bit-integer, if it is any larger than
// 255, real applications will assume invalidity and discard the event (ugh.)
// We also cannot use the Haiku-native keycode definitions, as these do not
// contain direct mappings for the modifier and function keys. So, we must
// create a local keycode mapping scheme.
enum class LocalKeyCode {
	Unknown = 1,

	// TODO: These are not actually generated at present!
	LeftShift, RightShift,
	LeftControl, RightControl,
	LeftAlt, RightAlt,
	CapsLock,
	NumLock,
	ScrollLock,

	Backspace,
	Return,
	Space,
	Tab,
	Escape,

	LeftArrow, RightArrow, UpArrow, DownArrow,

	Insert, Delete,
	Home, End,
	PageUp, PageDown,

	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

	count,
};
static_assert((int)LocalKeyCode::count < '0');

static inline LocalKeyCode
map_local_from_be(int32 rawChar, int32 key)
{
	switch (rawChar) {
	case B_BACKSPACE:	return LocalKeyCode::Backspace;
	case B_RETURN:		return LocalKeyCode::Return;
	case B_SPACE:		return LocalKeyCode::Space;
	case B_TAB:			return LocalKeyCode::Tab;
	case B_ESCAPE:		return LocalKeyCode::Escape;

	case B_LEFT_ARROW:	return LocalKeyCode::LeftArrow;
	case B_RIGHT_ARROW:	return LocalKeyCode::RightArrow;
	case B_UP_ARROW:	return LocalKeyCode::UpArrow;
	case B_DOWN_ARROW:	return LocalKeyCode::DownArrow;

	case B_INSERT:		return LocalKeyCode::Insert;
	case B_DELETE:		return LocalKeyCode::Delete;
	case B_HOME:		return LocalKeyCode::Home;
	case B_END:			return LocalKeyCode::End;
	case B_PAGE_UP:		return LocalKeyCode::PageUp;
	case B_PAGE_DOWN:	return LocalKeyCode::PageDown;

	case B_FUNCTION_KEY:
		switch (key) {
		case B_F1_KEY:	return LocalKeyCode::F1;
		case B_F2_KEY:	return LocalKeyCode::F2;
		case B_F3_KEY:	return LocalKeyCode::F3;
		case B_F4_KEY:	return LocalKeyCode::F4;
		case B_F5_KEY:	return LocalKeyCode::F5;
		case B_F6_KEY:	return LocalKeyCode::F6;
		case B_F7_KEY:	return LocalKeyCode::F7;
		case B_F8_KEY:	return LocalKeyCode::F8;
		case B_F9_KEY:	return LocalKeyCode::F9;
		case B_F10_KEY:	return LocalKeyCode::F10;
		case B_F11_KEY:	return LocalKeyCode::F11;
		case B_F12_KEY:	return LocalKeyCode::F12;
		}
		break;

	default: break;
	}

	if ((rawChar >= '0' && rawChar <= '9') || (rawChar >= 'A' && rawChar <= 'Z')
			|| (rawChar >= 'a' && rawChar <= 'z'))
		return (LocalKeyCode)rawChar;

	return LocalKeyCode::Unknown;
}

static inline KeySym
map_x_from_local(LocalKeyCode code)
{
	if ((int)code >= '0')
		return (KeySym)code; // Presume ASCII.

	switch (code) {
	default:
	case LocalKeyCode::Unknown:			return NoSymbol;

	case LocalKeyCode::LeftShift:		return XK_Shift_L;
	case LocalKeyCode::RightShift:		return XK_Shift_R;

	case LocalKeyCode::LeftControl:		return XK_Control_L;
	case LocalKeyCode::RightControl:	return XK_Control_R;

	case LocalKeyCode::LeftAlt:			return XK_Alt_L;
	case LocalKeyCode::RightAlt:		return XK_Alt_R;

	case LocalKeyCode::CapsLock:		return XK_Caps_Lock;
	case LocalKeyCode::NumLock:			return XK_Num_Lock;
	case LocalKeyCode::ScrollLock:		return XK_Scroll_Lock;

	case LocalKeyCode::Backspace:		return XK_BackSpace;
	case LocalKeyCode::Return:			return XK_Return;
	case LocalKeyCode::Space:			return XK_space;
	case LocalKeyCode::Tab:				return XK_Tab;
	case LocalKeyCode::Escape:			return XK_Escape;

	case LocalKeyCode::LeftArrow:		return XK_Left;
	case LocalKeyCode::RightArrow:		return XK_Right;
	case LocalKeyCode::UpArrow:			return XK_Up;
	case LocalKeyCode::DownArrow:		return XK_Down;

	case LocalKeyCode::Insert:			return XK_Insert;
	case LocalKeyCode::Delete:			return XK_Delete;
	case LocalKeyCode::Home:			return XK_Home;
	case LocalKeyCode::End:				return XK_End;
	case LocalKeyCode::PageUp:			return XK_Page_Up;
	case LocalKeyCode::PageDown:		return XK_Page_Down;

	case LocalKeyCode::F1:				return XK_F1;
	case LocalKeyCode::F2:				return XK_F2;
	case LocalKeyCode::F3:				return XK_F3;
	case LocalKeyCode::F4:				return XK_F4;
	case LocalKeyCode::F5:				return XK_F5;
	case LocalKeyCode::F6:				return XK_F6;
	case LocalKeyCode::F7:				return XK_F7;
	case LocalKeyCode::F8:				return XK_F8;
	case LocalKeyCode::F9:				return XK_F9;
	case LocalKeyCode::F10:				return XK_F10;
	case LocalKeyCode::F11:				return XK_F11;
	case LocalKeyCode::F12:				return XK_F12;
	}
}

int
_x_get_button_state(BMessage* message)
{
	int32 modifiers = 0, mouseButtons = 0;
	if (message->FindInt32("modifiers", &modifiers) != B_OK)
		modifiers = -1;
	if (message->FindInt32("buttons", &mouseButtons) != B_OK)
		mouseButtons = -1;
	return _x_get_button_state(modifiers, mouseButtons);
}

int
_x_get_button_state(int32 modifiers, int32 mouseButtons)
{
	if (mouseButtons < 0) {
		uint32 mouseBtns;
		get_mouse(NULL, &mouseBtns);
		mouseButtons = mouseBtns;
	}
	if (modifiers < 0) {
		if (mouseButtons) {
			modifiers = ::modifiers();
		} else {
			// Don't bother fetching modifiers if no mouse buttons are pressed.
			modifiers = 0;
		}
	}

	int xmod = 0;

	if (modifiers & B_SHIFT_KEY)
		xmod |= ShiftMask;
	if (modifiers & B_COMMAND_KEY)
		xmod |= ControlMask;
	if (modifiers & B_CAPS_LOCK)
		xmod |= LockMask;
	if (modifiers & B_CONTROL_KEY)
		xmod |= Mod1Mask;
	if (modifiers & B_NUM_LOCK)
		xmod |= Mod2Mask;

	if (mouseButtons & B_MOUSE_BUTTON(1))
		xmod |= Button1Mask;
	if (mouseButtons & B_MOUSE_BUTTON(3))
		xmod |= Button2Mask;
	if (mouseButtons & B_MOUSE_BUTTON(2))
		xmod |= Button3Mask;

	return xmod;
}

void
_x_fill_key_event(XEvent* event, BMessage* message, const char* bytes, int32 numBytes)
{
	int32 rawChar = 0, key = 0;
	message->FindInt32("raw_char", &rawChar);
	message->FindInt32("key", &key);

	event->xkey.keycode = (int)map_local_from_be(rawChar, key);
	event->xkey.state = _x_get_button_state(message);

	// abuse the "same_screen" field to store the bytes.
	event->xkey.same_screen = 0;
	memcpy(&event->xkey.same_screen, bytes,
		min_c(numBytes, sizeof(event->xkey.same_screen)));
}

extern "C" KeySym
XkbKeycodeToKeysym(Display* dpy, unsigned int kc, int group, int level)
{
	return map_x_from_local((LocalKeyCode)kc);
}

extern "C" KeySym
XKeycodeToKeysym(Display* dpy, unsigned int kc, int index)
{
	return XkbKeycodeToKeysym(dpy, kc, 0, 0);
}

extern "C" KeySym
XLookupKeysym(XKeyEvent* key_event, int index)
{
	return XkbKeycodeToKeysym(key_event->display, key_event->keycode, 0, 0);
}

extern "C" Bool
XkbLookupKeySym(Display* dpy, KeyCode keycode,
	unsigned int modifiers, unsigned int* modifiers_return, KeySym* keysym_return)
{
	*keysym_return = map_x_from_local((LocalKeyCode)keycode);
	if (modifiers_return)
		*modifiers_return = modifiers;
	return (*keysym_return != NoSymbol);
}

extern "C" int
XLookupString(XKeyEvent* key_event, char* buffer_return, int bytes_buffer,
	KeySym* keysym_return, XComposeStatus* status_in_out)
{
	*keysym_return = XLookupKeysym(key_event, 0);

	int copybytes = strnlen((const char*)&key_event->same_screen, sizeof(key_event->same_screen));
	copybytes = min_c(copybytes, bytes_buffer);
	if (copybytes <= 0)
		return 0;
	memcpy(buffer_return, &key_event->same_screen, copybytes);
	return copybytes;
}

extern "C" XModifierKeymap*
XGetModifierMapping(Display* display)
{
	XModifierKeymap* map = (XModifierKeymap*)calloc(sizeof(XModifierKeymap), 1);
	map->max_keypermod = 2;
	map->modifiermap = (KeyCode*)calloc(sizeof(KeyCode), 16);
	memset(map->modifiermap, 0, sizeof(KeyCode) * 16);
	map->modifiermap[ShiftMapIndex * 2 + 0] = (uchar)LocalKeyCode::LeftShift;
	map->modifiermap[ShiftMapIndex * 2 + 1] = (uchar)LocalKeyCode::RightShift;
	map->modifiermap[LockMapIndex * 2 + 0] = (uchar)LocalKeyCode::CapsLock;
	map->modifiermap[ControlMapIndex * 2 + 0] = (uchar)LocalKeyCode::LeftControl;
	map->modifiermap[ControlMapIndex * 2 + 1] = (uchar)LocalKeyCode::RightControl;
	map->modifiermap[Mod1MapIndex * 2 + 0] = (uchar)LocalKeyCode::LeftAlt;
	map->modifiermap[Mod2MapIndex * 2 + 0] = (uchar)LocalKeyCode::NumLock;
	map->modifiermap[Mod3MapIndex * 2 + 0] = (uchar)LocalKeyCode::ScrollLock;
	map->modifiermap[Mod4MapIndex * 2 + 0] = (uchar)LocalKeyCode::RightAlt;
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

	for (int i = 0; i < KEY_SYM_LIST_LENGTH; i++) {
		if (strcmp(KEY_SYM_LIST[i].name, string) == 0)
			return KEY_SYM_LIST[i].keySym;
	}
	return NoSymbol;
}

extern "C" char*
XKeysymToString(KeySym keysym)
{
	for (int i = 0; i < KEY_SYM_LIST_LENGTH; i++) {
		if (KEY_SYM_LIST[i].keySym == keysym)
			return (char*)KEY_SYM_LIST[i].name;
	}
	return NULL;
}

// #pragma mark - minor functions

extern "C" int
XDisplayKeycodes(Display* dpy, int* min_keycodes_return, int* max_keycodes_return)
{
	// The documentation specifies that min >= 8 and max <= 255.
	if (min_keycodes_return)
		*min_keycodes_return = 8;
	if (max_keycodes_return)
		*max_keycodes_return = 255;
	return 0;
}

extern "C" Display*
XkbOpenDisplay(char *display_name, int *event_rtrn, int *error_rtrn,
	int *major_in_out, int *minor_in_out, int *reason_rtrn)
{
	return XOpenDisplay(display_name);
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

// #pragma mark - stubs

extern "C" KeyCode
XKeysymToKeycode(Display* display, KeySym keysym)
{
	UNIMPLEMENTED();
	return 0;
}

extern "C" void
XConvertCase(KeySym keysym, KeySym *lower_return, KeySym *upper_return)
{
	UNIMPLEMENTED();
}

extern "C" KeySym*
XGetKeyboardMapping(Display *display, unsigned int first_keycode, int keycode_count, int *keysyms_per_keycode_return)
{
	UNIMPLEMENTED();
	return NULL;
}

extern "C" int
XGetKeyboardControl(Display* dpy, XKeyboardState* state_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XChangeKeyboardControl(Display* dpy, unsigned long value_mask, XKeyboardControl* control)
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

extern "C" Bool
XkbGetNamedIndicator(Display* dpy, Atom name,
	int* index_return, Bool* state_return, XkbIndicatorMapPtr map_return, Bool* real_return)
{
	UNIMPLEMENTED();
	return False;
}

extern "C" Bool
XkbSetNamedIndicator(Display* dpy, Atom name,
	Bool change_state, Bool state, Bool create_new, XkbIndicatorMapPtr map)
{
	UNIMPLEMENTED();
	return False;
}

extern "C" void
XkbFreeKeyboard(XkbDescPtr xkb, unsigned int which, Bool freeDesc)
{
	UNIMPLEMENTED();
}

extern "C" int
XGrabKey(Display *display, int keycode, unsigned int modifiers, Window grab_window,
	Bool owner_events, int pointer_mode, int keyboard_mode)
{
	UNIMPLEMENTED();
	// TODO?
	return BadImplementation;
}

extern "C" int
XUngrabKey(Display *display, int keycode, unsigned int modifiers, Window grab_window)
{
	UNIMPLEMENTED();
	return Success;
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
