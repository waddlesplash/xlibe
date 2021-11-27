#include "XInnerWindow.h"
#include "Color.h"
#include "Event.h"
#include <map>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
}

extern "C" Window
XCreateSimpleWindow(Display* display, Window parent, int x, int y, unsigned int width,
	unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
	XSetWindowAttributes attributes;
	attributes.border_pixel = border;
	attributes.background_pixel = background;
	return XCreateWindow(display, parent, x, y, width, height, border_width,
		CopyFromParent, CopyFromParent, CopyFromParent,
		CWBackPixel | CWBorderPixel, &attributes);
}

extern "C" Window
XCreateWindow(Display* display, Window parent, int x, int y, unsigned int w,
	unsigned int h, unsigned int border_width, int depth, unsigned int clazz,
	Visual* visual, unsigned long valueMask, XSetWindowAttributes* _attributes)
{
	XSetWindowAttributes attributes = {};
	if (_attributes)
		attributes = *_attributes;
	// TODO: valueMask

	// DEBUG
	if (x<=10)
		x=40;
	if (y<=10)
		y=40;
	if (w<=100)
		w=100;
	if (h<=100)
		h=100;

	WinHandle* handle;
	if (parent == 0) {
		BRect rect(x, y, x + w, y + h);
		Windows::add(handle = new XWindowFrame(rect, create_rgb(attributes.background_pixel)));
	} else {
		BRect rect(x, y, x + w + border_width*2, y + h + border_width *2);
		WinHandle* parent_window = Windows::get(parent);
		XWindow* window = new XWindow(rect, border_width, create_rgb(attributes.border_pixel),
			create_rgb(attributes.background_pixel));
		handle = window;
		parent_window->get_window()->AddChild(window);
		window->offscreen(parent_window->get_window()->offscreen());
		window->draw_border();
		Windows::add(window);
	}

	if (valueMask & CWEventMask)
		handle->get_window()->event_mask(attributes.event_mask);

	// TODO: just use XSetWindowAttributes

	return Windows::last_id();
}

int
XDestroyWindow(Display *display, Window w)
{
	// FIXME
	return 0;
}

Status XGetWindowAttributes(Display* display, Window window,
	XWindowAttributes* window_attributes_return)
{
	WinHandle* handle = Windows::get(window);
	if (!handle)
		return BadValue;

	memset(window_attributes_return, 0, sizeof(XWindowAttributes));

	// TODO: Not necessarily correct for pixmaps!
	window_attributes_return->visual = display->screens[0].root_visual;

	BRect frame;
	bool hidden = true, minimized = false;
	if (handle->is_root()) {
		window_attributes_return->root = window;
		XWindowFrame* w = dynamic_cast<XWindowFrame*>(handle);
		w->Lock();

		frame = w->Frame();
		hidden = w->IsHidden();
		minimized = w->IsMinimized();

		w->Unlock();
	} else {
		XWindow* w = dynamic_cast<XWindow*>(handle);
		if (w->Window())
			w->Window()->Lock();

		frame = w->Frame();
		hidden = w->IsHidden();

		if (w->Window())
			w->Window()->Unlock();
	}

	window_attributes_return->x = frame.LeftTop().x;
	window_attributes_return->y = frame.LeftTop().y;
	window_attributes_return->width = frame.IntegerWidth();
	window_attributes_return->height = frame.IntegerHeight();
	window_attributes_return->map_state =
		minimized ? IsUnviewable : (hidden ? IsUnmapped : IsViewable);

	return Success;
}

Status
XGetGeometry(Display *display, Drawable d, Window *root_return,
	int *x_return, int *y_return, unsigned int *width_return,
	unsigned int *height_return, unsigned int *border_width_return,
	unsigned int *depth_return)
{
	XWindowAttributes window_attributes;
	Status status = XGetWindowAttributes(display, d, &window_attributes);
	if (status != 0)
		return status;
	if (root_return)
		*root_return = window_attributes.root;
	if (x_return)
		*x_return = window_attributes.x;
	if (y_return)
		*y_return = window_attributes.y;
	if (width_return)
		*width_return = window_attributes.width;
	if (height_return)
		*height_return = window_attributes.height;
	if (border_width_return)
		*border_width_return = window_attributes.border_width;
	if (depth_return)
		*depth_return = window_attributes.depth;
	return Success;
}

extern "C" int
XIconifyWindow(Display *display, Window w, int screen)
{
	if (Windows::is_bewindow(w)) {
		XWindowFrame* window = Windows::get_bewindow(w);
		window->Minimize(true);
	}
	return Success;
}

extern "C" int
XSetWindowBorderWidth(Display *display, Window w, unsigned int width)
{
	XWindow* window = Windows::get_xwindow(w);
	if (window) {
		// TODO: add 2*new width to parentw/h>
		window->border_width(width);
		return Success;
	}
	return BadWindow;
}

Status XChangeWindowAttributes(Display *display, Window w,
	unsigned long vmask, XSetWindowAttributes *attr)
{
	// TODO!
	return Success;
}

int
XConfigureWindow(Display* display, Window window, unsigned int value_mask, XWindowChanges* values)
{
	// FIXME: Need also to resize the root window() if it is set.
	XWindow* w = Windows::get(window)->get_window();
	w->LockLooper();
	if ((value_mask & CWX) && (value_mask & CWY)) {
		w->MoveTo(values->x, values->y);
	}
	if ((value_mask & CWWidth) && (value_mask & CWHeight)) {
		w->ResizeTo(values->width, values->height);
	}
	w->UnlockLooper();
	return Success;
}

extern "C" int
XMoveWindow(Display* display, Window window, int x, int y)
{
	XWindowChanges changes;
	changes.x = x;
	changes.y = y;
	return XConfigureWindow(display, window, CWX | CWY, &changes);
}

extern "C" int
XResizeWindow(Display* display, Window window, unsigned int width, unsigned int height)
{
	XWindowChanges changes;
	changes.width = width;
	changes.height = height;
	return XConfigureWindow(display, window, CWWidth | CWHeight, &changes);
}

extern "C" int
XMoveResizeWindow(Display *display, Window w,
	int x, int y, unsigned int width, unsigned int height)
{
	XWindowChanges changes;
	changes.x = x;
	changes.y = y;
	changes.width = width;
	changes.height = height;
	return XConfigureWindow(display, w, CWX | CWY | CWWidth | CWHeight, &changes);
}

extern "C" int
XRaiseWindow(Display* display, Window w)
{
	// TODO: Also raise child views?
	if (Windows::is_bewindow(w)) {
		XWindowFrame* window = Windows::get_bewindow(w);
		window->Show();
	}
	return Success;
}

extern "C" Bool
XTranslateCoordinates(Display *display,
	Window src_w, Window dest_w,
	int src_x, int src_y, int *dest_x_return,
	int *dest_y_return, Window *child_return)
{
	// TODO: Implement (after drawable refactor.)
	debugger("XTranslateCoordinates");
	return False;
}

extern "C" int
XReparentWindow(Display* display, Window window, Window parent, int x, int y)
{
	// FIXME: This function is a hack and does do what it claims!
	WinHandle* wh = Windows::get(window);
	XWindowFrame* wf = dynamic_cast<XWindowFrame*>(wh);
	if (wf) {
		wf->Show();
	} else {
		dynamic_cast<XWindow*>(wh)->Show();
	}
	return Success;
}

extern "C" int XMapWindow(Display *display, Window window)
{
	WinHandle* handle = Windows::get(window);
	if(handle->is_root()) {
		dynamic_cast<XWindowFrame*>(handle)->Show();
	} else {
		dynamic_cast<XWindow*>(handle)->Show();
	}
	return 0;
}

extern "C" int
XUnmapWindow(Display *display, Window window)
{
	WinHandle* handle = Windows::get(window);
	if(handle->is_root()) {
		dynamic_cast<XWindowFrame*>(handle)->Hide();
	} else {
		dynamic_cast<XWindow*>(handle)->Hide();
	}
	return 0;
}

extern "C" int
XWithdrawWindow(Display *display, Window w, int screen)
{
	XUnmapWindow(display, w);
	return 1;
}

extern "C" int
XMapSubwindows(Display *display, Window window)
{
	XMapWindow(display, window);
	return 0;
}

extern "C" int
XMapRaised(Display* display, Window w)
{
	if(w == 0)
		return 0;
	if(Windows::is_bewindow(w)) {
		XWindowFrame* window = Windows::get_bewindow(w);
		window->Show();
		window->Activate();
	} else
		Windows::get_xwindow(w)->Show();
	return 0;
}

extern "C" int
XStoreName(Display *display, Window w, const char *wname)
{
	if(w == 0)
		return 0;
	if(Windows::is_bewindow(w))
		Windows::get_bewindow(w)->SetTitle(wname);
	Windows::get_xwindow(w)->SetName(wname);
	return 0;
}

extern "C" int
XFlush(Display *display)
{
	Windows::flush();
	return 0;
}

extern "C" int
XGetInputFocus(Display* display, Window* focus_return, int* revert_to_return)
{
	// TODO: Find the actually focused window.
	*focus_return = 0;
	*revert_to_return = RevertToParent;
	return 0;
}

extern "C" int
XSetWindowBackground(Display *display, Window w, unsigned long bg)
{
	XWindow* window = Windows::get_xwindow(w);
	window->bg_color(create_rgb(bg));
	return 0;
}

extern "C" int
XSetWindowBorder(Display* display, Window w, unsigned long border_pixel)
{
	if(w == 0)
		return 0;
	if(!Windows::is_bewindow(w)) {
		XWindow* window = Windows::get_xwindow(w);
		window->border_color(create_rgb(border_pixel));
		window->draw_border();
	}
	return 0;
}

extern "C" int
XClearWindow(Display *display, Window w)
{
	XWindow* window = Windows::get_xwindow(w);
	window->draw_border();
	return 0;
}

int XSetStandardProperties(Display* display, Window w,
	const char* window_name, const char* icon_name, Pixmap icon_pixmap,
	char** argv, int argc, XSizeHints* hints)
{
	XStoreName(display, w, window_name);
}

extern "C" Bool
XQueryPointer(Display *display, Window w, Window *root_return,
	Window *child_return, int *root_x_return,
	int *root_y_return, int *win_x_return, int *win_y_return,
	unsigned int *mask_return)
{
	if (!Windows::is_bewindow(w))
		return BadWindow;

	BWindow* window = Windows::get_bewindow(w);
	BPoint location;
	uint32 buttons;
	window->ChildAt(0)->GetMouse(&location, &buttons, false);
	BPoint rootLocation = window->ChildAt(0)->ConvertToScreen(location);

	if (root_x_return)
		*root_x_return = abs(rootLocation.x);
	if (root_y_return)
		*root_y_return = abs(rootLocation.y);
	if (win_x_return)
		*win_x_return = abs(location.x);
	if (win_y_return)
		*win_y_return = abs(location.y);

	int mask = 0;
	if (buttons & B_MOUSE_BUTTON(1))
		mask |= Button1Mask;
	if (buttons & B_MOUSE_BUTTON(2))
		mask |= Button2Mask;
	if (buttons & B_MOUSE_BUTTON(3))
		mask |= Button3Mask;

	if (mask_return)
		*mask_return = mask;

	return True;
}

extern "C" int
XDefineCursor(Display *display, Window w, Cursor cursor)
{
	XWindow* window = Windows::get_xwindow(w);
	window->SetViewCursor((BCursor*)cursor);
}

extern "C" int
XUndefineCursor(Display *display, Window w)
{
	XWindow* window = Windows::get_xwindow(w);
	window->SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);
}

XSizeHints*
XAllocSizeHints(void)
{
	return (XSizeHints*)malloc(sizeof(XSizeHints));
}

void
XSetWMNormalHints(Display *display, Window w, XSizeHints *hints)
{
	BWindow* window = NULL;
	if (Windows::is_bewindow(w))
		window = Windows::get_bewindow(w);
	if (!window)
		return;

	if (hints->flags & PBaseSize) {
		// Not supported.
	}
	if (hints->flags & PMinSize) {
		float maxWidth, maxHeight;
		window->GetSizeLimits(NULL, &maxWidth, NULL, &maxHeight);
		window->SetSizeLimits(hints->min_width, maxWidth, hints->min_height, maxHeight);
	}
	if (hints->flags & PMaxSize) {
		float minWidth, minHeight;
		window->GetSizeLimits(&minWidth, NULL, &minHeight, NULL);
		window->SetSizeLimits(minWidth, hints->max_width, minHeight, hints->max_height);
	}
	if (hints->flags & PResizeInc) {
		// Not supported.
	}
	// TODO: Flags?
}

XClassHint*
XAllocClassHint(void)
{
	return (XClassHint*)malloc(sizeof(XClassHint));
}
