#include "Drawables.h"
#include "Color.h"
#include "Event.h"
#include <map>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
}

#include "Debug.h"

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

	BRect frame(BPoint(x, y), BSize(w, h));
	XDrawable* window = new XDrawable(display, frame);
	window->border_width(border_width);
	if (parent == 0) {
		window->create_bwindow();
	} else {
		XDrawable* parent_window = Drawables::get(parent);
		parent_window->view()->AddChild(window->view());

		if (parent_window->event_mask() & SubstructureNotifyMask) {
			XEvent event;
			event.type = CreateNotify;
			event.xcreatewindow.parent = parent;
			event.xcreatewindow.window = window->id();
			event.xcreatewindow.x = x;
			event.xcreatewindow.y = y;
			event.xcreatewindow.width = w;
			event.xcreatewindow.height = h;
			event.xcreatewindow.border_width = border_width;
			x_put_event(display, event);
		}
	}

	XChangeWindowAttributes(display, window->id(), valueMask, &attributes);
	return window->id();
}

extern "C" Status
XChangeWindowAttributes(Display *display, Window w,
	unsigned long vmask, XSetWindowAttributes *attr)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	if (vmask & CWBorderPixel)
		window->border_pixel(attr->border_pixel);
	if (vmask & CWBackPixel)
		window->background_pixel(attr->background_pixel);
	if (vmask & CWEventMask)
		window->event_mask(attr->event_mask);
	if (vmask & CWCursor)
		XDefineCursor(display, w, attr->cursor);

	return Success;
}

extern "C" int
XDestroyWindow(Display *display, Window w)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	delete window;
	return Success;
}

extern "C" Status
XGetWindowAttributes(Display* display, Window w,
	XWindowAttributes* window_attributes_return)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	memset(window_attributes_return, 0, sizeof(XWindowAttributes));

	window_attributes_return->screen = &display->screens[0];
	// TODO: Not necessarily correct for pixmaps!
	window_attributes_return->visual = window_attributes_return->screen->root_visual;
	window_attributes_return->c_class = window_attributes_return->visual->c_class;
	window_attributes_return->depth = window_attributes_return->screen->depths[0].depth;

	BRect frame;
	bool hidden = true, minimized = false;
	window->view()->LockLooper();
	if (window->bwindow) {
		window_attributes_return->root = window->id();
		frame = window->bwindow->Frame();
		hidden = window->bwindow->IsHidden();
		minimized = window->bwindow->IsMinimized();
	} else {
		// TODO: ->root
		frame = window->view()->Frame();
		hidden = window->view()->IsHidden();
	}
	window->view()->UnlockLooper();

	window_attributes_return->x = frame.LeftTop().x;
	window_attributes_return->y = frame.LeftTop().y;
	window_attributes_return->width = frame.IntegerWidth();
	window_attributes_return->height = frame.IntegerHeight();
	window_attributes_return->border_width = window->border_width();
	window_attributes_return->your_event_mask = window->event_mask();
	window_attributes_return->all_event_masks = window->event_mask();
	window_attributes_return->map_state =
		minimized ? IsUnviewable : (hidden ? IsUnmapped : IsViewable);

	return Success;
}

extern "C" Status
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
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	if (window->bwindow) {
		window->bwindow->Minimize(true);
		return Success;
	}
	return BadValue;
}

extern "C" int
XSetWindowBorderWidth(Display *display, Window w, unsigned int width)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	window->border_width(width);
	return Success;
}

extern "C" int
XConfigureWindow(Display* display, Window w, unsigned int value_mask, XWindowChanges* values)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;
	window->view()->LockLooper();

	if ((value_mask & CWX) && (value_mask & CWY)) {
		// TODO: Allow changing just X or just Y.
		if (window->bwindow)
			window->bwindow->MoveTo(values->x, values->y);
		else
			window->view()->MoveTo(values->x, values->y);
	}

	if (value_mask & CWBorderWidth)
		window->border_width(values->border_width);

	int width = window->size().IntegerWidth(),
		height = window->size().IntegerHeight();
	if (value_mask & CWWidth)
		width = values->width;
	if (value_mask & CWHeight)
		height = values->height;
	if ((value_mask & CWWidth) || (value_mask & CWHeight))
		window->resize(width, height);

	window->view()->UnlockLooper();
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
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	if (window->bwindow) {
		window->bwindow->Show();
		window->bwindow->Activate();
	} else {
		// TODO: raise?
		window->view()->Show();
	}
	return Success;
}

extern "C" Bool
XTranslateCoordinates(Display *display,
	Window src_w, Window dest_w,
	int src_x, int src_y, int *dest_x_return,
	int *dest_y_return, Window *child_return)
{
	// TODO: Implement.
	debugger("XTranslateCoordinates");
	return False;
}

extern "C" int
XReparentWindow(Display* display, Window w, Window p, int x, int y)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	if (p == 0) {
		// Making a parented window into a root window.
		if (window->bwindow)
			return Success;

		BWindow* oldparent = window->view()->Window();
		if (oldparent)
			oldparent->LockLooper();
		window->view()->RemoveSelf();
		if (oldparent)
			oldparent->UnlockLooper();

		window->create_bwindow();
		return Success;
	}

	XDrawable* parent = Drawables::get(p);
	if (window->view()->Parent() == parent->view()) {
		// Nothing to do.
		return Success;
	}

	// Adding a window to another window.
	window->view()->RemoveSelf();
	if (window->bwindow) {
		window->view()->MoveTo(window->bwindow->Bounds().LeftTop());
		delete window->bwindow;
		window->bwindow = NULL;
	}
	parent->view()->AddChild(window->view());
	return Success;
}

extern "C" int
XMapWindow(Display *display, Window w)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	if (window->bwindow)
		window->bwindow->Show();
	else
		window->view()->Show();
	return Success;
}

extern "C" int
XUnmapWindow(Display *display, Window w)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	if (!window->view()->Window())
		return Success;

	if (window->bwindow) {
		window->bwindow->Hide();
	} else {
		window->view()->LockLooper();
		window->view()->Hide();
		window->view()->UnlockLooper();
	}
	return Success;
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
	return Success;
}

extern "C" int
XMapRaised(Display* display, Window w)
{
	XMapWindow(display, w);
	return XRaiseWindow(display, w);
}

extern "C" int
XStoreName(Display *display, Window w, const char *wname)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	// TODO: Make this work for parented windows!
	if (window->bwindow)
		window->bwindow->SetTitle(wname);
	return Success;
}

extern "C" int
XGetInputFocus(Display* display, Window* focus_return, int* revert_to_return)
{
	UNIMPLEMENTED();
	// TODO: Find the actually focused window!
	*focus_return = 0;
	*revert_to_return = RevertToParent;
	return Success;
}

extern "C" int
XSetWindowBackground(Display *display, Window w, unsigned long bg)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;
	window->background_pixel(bg);
	return Success;
}

extern "C" int
XSetWindowBorder(Display* display, Window w, unsigned long border_pixel)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;
	window->border_pixel(border_pixel);
	return Success;
}

extern "C" int
XClearWindow(Display *display, Window w)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;
	window->draw_border(BRect());
	return Success;
}

extern "C" Bool
XQueryPointer(Display *display, Window w, Window *root_return,
	Window *child_return, int *root_x_return,
	int *root_y_return, int *win_x_return, int *win_y_return,
	unsigned int *mask_return)
{
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;

	BPoint location;
	uint32 buttons;
	window->view()->LockLooper();
	window->view()->GetMouse(&location, &buttons, false);
	BPoint rootLocation = window->view()->ConvertToScreen(location);
	window->view()->UnlockLooper();

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
	XDrawable* window = Drawables::get(w);
	if (!window)
		return BadWindow;
	if (!window->view()->Window())
		return BadValue;

	window->view()->LockLooper();
	window->view()->SetViewCursor((BCursor*)cursor);
	window->view()->UnlockLooper();
	return Success;
}

extern "C" int
XUndefineCursor(Display *display, Window w)
{
	return XDefineCursor(display, w, (Cursor)static_cast<const BCursor*>(B_CURSOR_SYSTEM_DEFAULT));
}

extern "C" XSizeHints*
XAllocSizeHints()
{
	return (XSizeHints*)malloc(sizeof(XSizeHints));
}

extern "C" void
XSetWMNormalHints(Display *display, Window w, XSizeHints *hints)
{
	XDrawable* window = Drawables::get(w);
	if (!window || !window->bwindow)
		return;
	if (!hints)
		return;

	if (hints->flags & PBaseSize) {
		// Not supported.
	}
	if (hints->flags & PMinSize) {
		float maxWidth, maxHeight;
		window->bwindow->GetSizeLimits(NULL, &maxWidth, NULL, &maxHeight);
		window->bwindow->SetSizeLimits(hints->min_width, maxWidth, hints->min_height, maxHeight);
	}
	if (hints->flags & PMaxSize) {
		float minWidth, minHeight;
		window->bwindow->GetSizeLimits(&minWidth, NULL, &minHeight, NULL);
		window->bwindow->SetSizeLimits(minWidth, hints->max_width, minHeight, hints->max_height);
	}
	if (hints->flags & PResizeInc) {
		// Not supported.
	}
	// TODO: Flags?
}

extern "C" int
XSetStandardProperties(Display* display, Window w,
	const char* window_name, const char* icon_name, Pixmap icon_pixmap,
	char** argv, int argc, XSizeHints* hints)
{
	XStoreName(display, w, window_name);
	XSetWMNormalHints(display, w, hints);
	return Success;
}

extern "C" void
XmbSetWMProperties(Display* display, Window w,
	const char* window_name, const char* icon_name, char** argv, int argc,
	XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints)
{
	XSetStandardProperties(display, w, window_name, icon_name, None, argv, argc, normal_hints);
}

extern "C" XClassHint*
XAllocClassHint(void)
{
	return (XClassHint*)malloc(sizeof(XClassHint));
}
