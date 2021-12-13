#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include <string.h>
#include <stdio.h>

#include "Debug.h"

int
XGrabServer(Display *display)
{
	UNIMPLEMENTED();
	return Success;
}

int
XUngrabServer(Display *display)
{
	UNIMPLEMENTED();
	return Success;
}

int
XGrabPointer(Display *display, Window w1, Bool b, unsigned int ui,
	int i1, int i2, Window w2, Cursor c, Time t)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XUngrabPointer(Display *display, Time time)
{
	UNIMPLEMENTED();
	return Success;
}

Status
XAddConnectionWatch(Display *display, XConnectionWatchProc procedure, XPointer client_data)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

void
XRemoveConnectionWatch(Display *display, XConnectionWatchProc procedure, XPointer client_data)
{
	UNIMPLEMENTED();
}

void
XProcessInternalConnection(Display *display, int fd)
{
	UNIMPLEMENTED();
}

XHostAddress *
XListHosts(Display *display, int *nhosts_return, Bool *state_return)
{
	UNIMPLEMENTED();
	return NULL;
}

Status
XSetWMProtocols(Display *display, Window w, Atom* protocols, int count)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

void
XSetWMClientMachine(Display *display, Window w, XTextProperty *text_prop)
{
	UNIMPLEMENTED();
}

void XSetWMSizeHints(Display* display, Window w, XSizeHints* hints, Atom property)
{
	UNIMPLEMENTED();
}

int
XSetWindowColormap(Display *display, Window w, Colormap colormap)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

Status
XGetWMColormapWindows(Display *display, Window w, Window **windows_return,
			  int *count_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

Status
XSetWMColormapWindows(Display *display, Window w,
	Window *colormap_windows, int count)
{
	UNIMPLEMENTED();
	return 0;
}

int
XSetWMHints(Display *display, Window w, XWMHints *wm_hints)
{
	UNIMPLEMENTED();
	return 0;
}

int
XParseGeometry(const char *parsestring, int *x_return, int *y_return, unsigned int *width_return, unsigned int *height_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XWMGeometry(Display *display, int screen, const char *user_geom, const char *def_geom, unsigned int bwidth, XSizeHints *hints, int *x_return, int *y_return, int *width_return, int *height_return, int *gravity_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XRefreshKeyboardMapping(XMappingEvent *event_map)
{
	UNIMPLEMENTED();
	return 0;
}

int
XConvertSelection(Display *display, Atom selection, Atom target,
	Atom property, Window requestor, Time time)
{
	UNIMPLEMENTED();
	return 0;
}

VisualID
XVisualIDFromVisual(Visual *visual)
{
	UNIMPLEMENTED();
	return 0;
}

Status
XStringListToTextProperty(char **list, int count,
			  XTextProperty *text_prop_return)
{
	UNIMPLEMENTED();
	return BadAlloc;
}

int
XSetClassHint(Display *display, Window w, XClassHint *class_hints)
{
	UNIMPLEMENTED();
	return 0;
}

Window
XGetSelectionOwner(Display* display, Atom selection)
{
	UNIMPLEMENTED();
	return None;
}

int
XSetSelectionOwner(Display *display, Atom selection, Window owner, Time time)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XSetStipple(Display *display, GC gc, Pixmap stipple)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XSetTile(Display *display, GC gc, Pixmap tile)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XSetTSOrigin(Display *display, GC gc, int x, int y)
{
	return BadImplementation;
}

XFontSet XCreateFontSet(Display *display, const char *base_font_name_list,
	char ***missing_charset_list_return, int *missing_charset_count_return, char **def_string_return)
{
	UNIMPLEMENTED();
	return NULL;
}

void XFreeFontSet(Display* dpy, XFontSet xf)
{
	UNIMPLEMENTED();
}

int
XSetIconName(Display *display, Window w, const char *icon_name)
{
	return BadImplementation;
}

void
XSetWMIconName(Display* display, Window w, XTextProperty* icon_name)
{
}

Status XTextPropertyToStringList(
	XTextProperty *tp,
	char ***list_return,
	int *count_return)
{
	UNIMPLEMENTED();
	return BadAlloc;
}

void XFreeStringList(char **list)
{
	UNIMPLEMENTED();
}

int XmbTextListToTextProperty(Display* display,
	char** list, int count,
	XICCEncodingStyle style,
	XTextProperty* text_prop_return)
{
	UNIMPLEMENTED();
	text_prop_return->value = NULL;
	return BadAlloc;
}

int XwcTextListToTextProperty(Display* display,
	wchar_t** list, int count,
	XICCEncodingStyle style,
	XTextProperty* text_prop_return)
{
	UNIMPLEMENTED();
	text_prop_return->value = NULL;
	return BadAlloc;
}

Status Xutf8TextPropertyToTextList(
	Display*		display,
	const XTextProperty* text_prop,
	char***		list_return,
	int*		count_return)
{
	UNIMPLEMENTED();
	return BadAlloc;
}

Status XwcTextPropertyToTextList(
	Display*		display,
	const XTextProperty* text_prop,
	wchar_t***		list_return,
	int*		count_return)
{
	UNIMPLEMENTED();
	return BadAlloc;
}

void XwcFreeStringList(wchar_t **list)
{
	UNIMPLEMENTED();
}

int
XSetTransientForHint(Display *display, Window w, Window prop_window)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XSetInputFocus(Display *display, Window focus, int revert_to, Time time)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XGetWindowProperty(Display *display, Window w, Atom property,
	long long_offset, long long_length, Bool delete,
	Atom req_type, Atom *actual_type_return,
	int *actual_format_return, unsigned long *nitems_return,
	unsigned long *bytes_after_return,
	unsigned char **prop_return)
{
	UNIMPLEMENTED();
	*nitems_return = 0;
	*prop_return = NULL;
	return BadImplementation;
}

Status
XGetTextProperty(Display *display, Window w,
	XTextProperty *text_prop_return, Atom property)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

void
XSetTextProperty(Display *display, Window w,
	XTextProperty *text_prop, Atom property)
{
	UNIMPLEMENTED();
}

int
XChangeProperty(Display *display, Window w, Atom property, Atom type,
	int format, int mode, const unsigned char *data, int nelements)
{
	if (type == XA_ATOM) {
		fprintf(stderr, "UNIMPLEMENTED: XChangeProperty: %s = %s\n", XGetAtomName(display, property),
			XGetAtomName(display, *(Atom*)data));
	} else {
		fprintf(stderr, "UNIMPLEMENTED: XChangeProperty: %s(%s)\n", XGetAtomName(display, property),
			XGetAtomName(display, type));
	}
	return BadImplementation;
}

int
XDeleteProperty(Display *display, Window w, Atom property)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
(*XSynchronize(Display*, Bool))(Display*)
{
	UNIMPLEMENTED();
	return NULL;
}

int
XSetCommand(Display *display, Window w, char **argv, int argc)
{
	UNIMPLEMENTED();
	return 0;
}

XErrorHandler
XSetErrorHandler(XErrorHandler handler)
{
	return NULL;
}

XIOErrorHandler
XSetIOErrorHandler(XIOErrorHandler handler)
{
	return NULL;
}

int XResetScreenSaver(Display* display)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int XForceScreenSaver(Display* display, int)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XSetWindowBorderPixmap(Display *display, Window w, Pixmap border_pixmap)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XSetWindowBackgroundPixmap(Display *display, Window w, Pixmap background_pixmap)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XWarpPointer(Display *display, Window src_w, Window dest_w,
		 int src_x, int src_y, unsigned int src_width,
		 unsigned int src_height, int dest_x, int dest_y)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XGetPointerControl(Display* display,
	int* accel_numerator_return, int* accel_denominator_return, int* threshold_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

Status
XReconfigureWMWindow(Display *display, Window w, int screen_number,
	unsigned int mask, XWindowChanges *changes)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

Bool
XSupportsLocale()
{
	return True;
}

char*
XSetLocaleModifiers(const char *modifier_list)
{
	UNIMPLEMENTED();
	return NULL;
}

int
XGetErrorDatabaseText(Display* dpy, const char* name,
	const char* message, const char* default_string, char* buffer_return, int length)
{
	UNIMPLEMENTED();
	strlcpy(buffer_return, default_string, length);
	return 0;
}

int
XGetErrorText(Display* dpy, int code, char* buffer_return, int length)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

char*
XGetDefault(Display* display, const char* program, const char* option)
{
	UNIMPLEMENTED();
	return NULL;
}

int
XGrabButton(Display *display, unsigned int button, unsigned int modifiers, Window grab_window,
	Bool owner_events, unsigned int event_mask, int pointer_mode, int keyboard_mode, Window confine_to, Cursor cursor)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XUngrabButton(Display *display, unsigned int button, unsigned int modifiers, Window grab_window)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

Status
XGetRGBColormaps(Display *display, Window w, XStandardColormap **std_colormap_return, int *count_return, Atom property)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

void
XSetRGBColormaps(Display *display, Window w, XStandardColormap *std_colormap, int count, Atom property)
{
	UNIMPLEMENTED();
}

int
XKillClient(Display *display, XID resource)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

int
XSetCloseDownMode(Display* display, int close_mode)
{
	return BadImplementation;
}
