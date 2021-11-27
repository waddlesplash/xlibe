#include <X11/Xlib.h>
#include <X11/Xutil.h>

int
XSendEvent(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send)
{
	return BadImplementation;
}

int
XGrabServer(Display *display)
{
	return Success;
}

int
XUngrabServer(Display *display)
{
	return Success;
}

int
XGrabPointer(Display *display, Window w1, Bool b, unsigned int ui,
	int i1, int i2, Window w2, Cursor c, Time t)
{
	return BadImplementation;
}

int
XUngrabPointer(Display *display, Time time)
{
	return Success;
}

XHostAddress *
XListHosts(Display *display, int *nhosts_return, Bool *state_return)
{
	return NULL;
}

void
XSetWMClientMachine(Display *display, Window w, XTextProperty *text_prop)
{
}

int
XSetWindowColormap(Display *display, Window w, Colormap colormap)
{
	return BadImplementation;
}

Status
XGetWMColormapWindows(Display *display, Window w, Window **windows_return,
			  int *count_return)
{
	return BadImplementation;
}

Status
XSetWMColormapWindows(Display *display, Window w, Window *colormap_windows,
			  int count)
{
	return 0;
}

int
XSetWMHints(Display *display, Window w, XWMHints *wm_hints)
{
	return 0;
}

int
XRefreshKeyboardMapping(XMappingEvent *event_map)
{
	return 0;
}

int
XConvertSelection(Display *display, Atom selection, Atom target,
	Atom property, Window requestor, Time time)
{
	return 0;
}

VisualID
XVisualIDFromVisual(Visual *visual)
{
	return 0;
}

Status
XStringListToTextProperty(char **list, int count,
			  XTextProperty *text_prop_return)
{
	return BadAlloc;
}

int
XSetClassHint(Display *display, Window w, XClassHint *class_hints)
{
	return 0;
}

Window
XGetSelectionOwner(Display* display, Atom selection)
{
	return None;
}

int
XSetSelectionOwner(Display *display, Atom selection, Window owner, Time time)
{
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
	return NULL;
}

void XFreeFontSet(Display* dpy, XFontSet xf)
{
}

int
XSetIconName(Display *display, Window w, const char *icon_name)
{
	return BadImplementation;
}

Status XTextPropertyToStringList (
	XTextProperty *tp,
	char ***list_return,
	int *count_return)
{
	return BadAlloc;
}

void XFreeStringList (char **list)
{
}

int
XSetTransientForHint(Display *display, Window w, Window prop_window)
{
	return BadImplementation;
}

int
XSetInputFocus(Display *display, Window focus, int revert_to, Time time)
{
	return BadImplementation;
}


int
XQueryTree(Display *display, Window w, Window *root_return,
	   Window *parent_return, Window **children_return,
	   unsigned int *nchildren_return)
{
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
	if (nitems_return)
		*nitems_return = 0;
	return BadImplementation;
}

int
XChangeProperty(Display *display, Window w, Atom property, Atom type,
	int format, int mode, _Xconst unsigned char *data, int nelements)
{
	return BadImplementation;
}

int
XDeleteProperty(Display *display, Window w, Atom property)
{
	return BadImplementation;
}

int
(*XSynchronize(Display*, Bool))(Display*)
{
	return NULL;
}

int
XSetCommand(Display *display, Window w, char **argv, int argc)
{
	return 0;
}

XErrorHandler
XSetErrorHandler(XErrorHandler handler)
{
	return NULL;
}

int XResetScreenSaver(Display* display)
{
	return BadImplementation;
}

int XForceScreenSaver(Display* display, int)
{
	return BadImplementation;
}

int
XSetWindowBorderPixmap(Display *display, Window w, Pixmap border_pixmap)
{
	return BadImplementation;
}

int
XSetWindowBackgroundPixmap(Display *display, Window w, Pixmap background_pixmap)
{
	return BadImplementation;
}

int
XWarpPointer(Display *display, Window src_w, Window dest_w,
		 int src_x, int src_y, unsigned int src_width,
		 unsigned int src_height, int dest_x, int dest_y)
{
	return BadImplementation;
}

Status
XReconfigureWMWindow(Display *display, Window w, int screen_number,
	unsigned int mask, XWindowChanges *changes)
{
	return BadImplementation;
}

char* XSetLocaleModifiers(const char *modifier_list)
{
	return NULL;
}
