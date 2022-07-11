/*
 * Copyright 2003, Shibukawa Yoshiki. All rights reserved.
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include "Drawables.h"

#include <private/interface/WindowPrivate.h>

#include "Atom.h"
#include "Property.h"
#include "Locale.h"
#include "Drawing.h"
#include "Color.h"
#include "Keyboard.h"
#include "Event.h"

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

	BRect frame(brect_from_xrect(make_xrect(x, y, w, h)));
	XWindow* window = new XWindow(display, frame);
	window->border_width(border_width);

	XWindow* parent_window = Drawables::get_window(parent);
	if (!parent_window) {
		window->create_bwindow();
		window->bwindow->MoveTo(x, y);
	} else {
		window->view()->Hide();
		parent_window->view()->AddChild(window->view());

		if (parent_window->event_mask() & SubstructureNotifyMask) {
			XEvent event = {};
			event.type = CreateNotify;
			event.xcreatewindow.parent = parent;
			event.xcreatewindow.window = window->id();
			event.xcreatewindow.x = x;
			event.xcreatewindow.y = y;
			event.xcreatewindow.width = w;
			event.xcreatewindow.height = h;
			event.xcreatewindow.border_width = border_width;
			_x_put_event(display, event);
		}
	}

	XChangeWindowAttributes(display, window->id(), valueMask, &attributes);
	return window->id();
}

extern "C" Status
XChangeWindowAttributes(Display *display, Window w,
	unsigned long vmask, XSetWindowAttributes *attr)
{
	XWindow* window = Drawables::get_window(w);
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

	if (vmask & CWOverrideRedirect) {
		if (window->bwindow && window->override_redirect != attr->override_redirect) {
			// We don't really have a way to do what "override redirect" is designed for,
			// which is mostly to "bypass" normal window manager behavior. The best we can
			// do is set the flags we do here.
			if (attr->override_redirect) {
				// "Menu windows" float above all others, even modals, which is what we need.
				window->bwindow->SetFeel(kMenuWindowFeel);
				window->bwindow->SetLook(B_NO_BORDER_WINDOW_LOOK);
				window->bwindow->SetFlags(window->bwindow->Flags() | B_AVOID_FOCUS);
			} else {
				window->bwindow->SetFeel(B_NORMAL_WINDOW_FEEL);
				window->bwindow->SetLook(B_TITLED_WINDOW_LOOK);
				window->bwindow->SetFlags(window->bwindow->Flags() & ~B_AVOID_FOCUS);
			}
			window->override_redirect = attr->override_redirect;
		}
	}

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

extern "C" int
XGetWindowAttributes(Display* display, Window w,
	XWindowAttributes* window_attributes_return)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return 0;

	memset(window_attributes_return, 0, sizeof(XWindowAttributes));

	window_attributes_return->root = DefaultRootWindow(display);
	window_attributes_return->screen = &display->screens[0];
	// TODO: Not necessarily correct for pixmaps!
	window_attributes_return->visual = window_attributes_return->screen->root_visual;
	window_attributes_return->c_class = window_attributes_return->visual->c_class;
	window_attributes_return->depth = window_attributes_return->screen->depths[0].depth;

	BRect frame;
	bool hidden = true, minimized = false;
	window->view()->LockLooper();
	if (window->bwindow) {
		frame = window->bwindow->Frame();
		hidden = window->bwindow->IsHidden();
		minimized = window->bwindow->IsMinimized();
	} else {
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

	return 1;
}

extern "C" Status
XGetGeometry(Display* display, Drawable d, Window* root_return,
	int* x_return, int* y_return, unsigned int* width_return,
	unsigned int* height_return, unsigned int* border_width_return,
	unsigned int* depth_return)
{
	XWindowAttributes window_attributes;
	Status status = XGetWindowAttributes(display, d, &window_attributes);
	if (status != 1)
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
	return 1;
}

extern "C" int
XIconifyWindow(Display *display, Window w, int screen)
{
	XWindow* window = Drawables::get_window(w);
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
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;

	window->border_width(width);
	return Success;
}

extern "C" int
XConfigureWindow(Display* display, Window w, unsigned int value_mask, XWindowChanges* values)
{
	XWindow* window = Drawables::get_window(w);
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
		window->resize(brect_from_xrect(make_xrect(0, 0, width, height)).Size());

	if (value_mask & CWStackMode) {
		XWindow* sibling = NULL;
		if (value_mask & CWSibling)
			sibling = Drawables::get_window(values->sibling);

		if (window->bwindow) {
			BWindow* siblingWindow = sibling ? sibling->bwindow : NULL;
			switch (values->stack_mode) {
			case Above:
				// TODO: Haiku doesn't really have a way to raise a window above a specific one.
				window->bwindow->Activate();
			break;
			case Below:
				window->bwindow->SendBehind(siblingWindow);
			break;
			default:
				debugger("Unsupported restack mode!");
			}
		} else {
			BView* parent = window->view()->Parent();
			window->view()->RemoveSelf();
			switch (values->stack_mode) {
			case Above:
				if (sibling)
					parent->AddChild(window->view(), sibling->view()->NextSibling());
				else
					parent->AddChild(window->view());
			break;
			case Below:
				if (sibling) {
					parent->AddChild(window->view(), sibling->view());
				} else {
					BView* first = parent->CountChildren() ?
						parent->ChildAt(0) : NULL;
					parent->AddChild(window->view(), first);
				}
			break;
			default:
				debugger("Unsupported restack mode!");
			}
		}
	}

	window->view()->UnlockLooper();
	return Success;
}

extern "C" Status
XReconfigureWMWindow(Display* display, Window w, int screen_number,
	unsigned int value_mask, XWindowChanges* values)
{
	return XConfigureWindow(display, w, value_mask, values);
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
	XWindowChanges changes;
	changes.stack_mode = Above;
	return XConfigureWindow(display, w, CWStackMode, &changes);
}

extern "C" int
XLowerWindow(Display* display, Window w)
{
	XWindowChanges changes;
	changes.stack_mode = Below;
	return XConfigureWindow(display, w, CWStackMode, &changes);
}

extern "C" int
XQueryTree(Display* display, Window w, Window* root_return,
	Window* parent_return, Window** children_return, unsigned int* nchildren_return)
{
	XWindow* window = Drawables::get_window(w);
	if (!window) {
		*children_return = NULL;
		*nchildren_return = 0;
		return BadWindow;
	}

	if (window->view()->Window())
		window->view()->LockLooper();

	*root_return = DefaultRootWindow(display);
	*parent_return = window->parent();

	if (children_return) {
		const std::list<Drawable> children = window->children();
		if (!children.empty()) {
			Window* list;
			*children_return = list = (Window*)malloc(sizeof(Window) * children.size());
			int i = 0;
			for (const auto& child : children)
				list[i++] = child;
			*nchildren_return = i;
		} else {
			*children_return = NULL;
			*nchildren_return = 0;
		}
	}

	if (window->view()->Window())
		window->view()->UnlockLooper();

	return Success;
}

extern "C" Bool
XTranslateCoordinates(Display* display,
	Window src_w, Window dest_w, int src_x, int src_y,
	int* dest_x_return, int* dest_y_return, Window* child_return)
{
	XDrawable* src_window = Drawables::get(src_w),
		*dest_window = Drawables::get(dest_w);

	BPoint coord(src_x, src_y);

	if (src_window) {
		src_window->view()->LockLooper();
		src_window->view()->ConvertToScreen(&coord);
		src_window->view()->UnlockLooper();
	}

	Window child = 0;
	if (dest_window) {
		dest_window->view()->LockLooper();
		dest_window->view()->ConvertFromScreen(&coord);
		dest_window->contains(coord, child);
		dest_window->view()->UnlockLooper();
	}

	if (dest_x_return)
		*dest_x_return = coord.x;
	if (dest_x_return)
		*dest_y_return = coord.y;
	if (child_return)
		*child_return = child;
	return True;
}

extern "C" int
XReparentWindow(Display* dpy, Window w, Window nP, int x, int y)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;
	XWindow* oldParent = Drawables::get_window(window->parent()),
		*newParent = Drawables::get_window(nP);

	const bool mapped = !window->view()->IsHidden() &&
		(window->view()->Window() && !window->view()->Window()->IsHidden());

	if (!newParent) {
		// Making a parented window into a root window.
		if (window->bwindow) {
			XMoveWindow(dpy, w, x, y);
			return Success;
		}

		BWindow* oldparent = window->view()->Window();
		if (oldparent)
			oldparent->LockLooper();
		window->view()->RemoveSelf();
		if (oldparent)
			oldparent->UnlockLooper();

		window->create_bwindow();
	} else if (window->view()->Parent() != newParent->view()) {
		// Adding a window to another window.
		BWindow* parentWindow = window->view()->Window();
		if (parentWindow)
			parentWindow->LockLooper();
		window->view()->RemoveSelf();
		if (parentWindow)
			parentWindow->UnlockLooper();

		if (window->bwindow) {
			window->view()->MoveTo(0, 0);
			delete window->bwindow;
			window->bwindow = NULL;
		}
		window->view()->Hide();
		newParent->view()->AddChild(window->view());
	}

	const bool structureSelf = window->event_mask() & StructureNotifyMask;
	const bool substructureOld = (oldParent && oldParent->event_mask() & SubstructureNotifyMask);
	const bool substructureNew = (newParent && newParent->event_mask() & SubstructureNotifyMask);
	if (structureSelf || substructureOld || substructureNew) {
		XEvent event = {};
		event.type = ReparentNotify;
		event.xreparent.event = structureSelf ? window->id()
			: substructureOld ? oldParent->id() : newParent->id();
		event.xreparent.window = window->id();
		event.xreparent.parent = nP;
		event.xreparent.x = x;
		event.xreparent.y = y;
		_x_put_event(dpy, event);
	}

	XMoveWindow(dpy, w, x, y);
	if (mapped)
		XMapWindow(dpy, w);
	return Success;
}

extern "C" int
XSetTransientForHint(Display* display, Window w, Window prop_w)
{
	XWindow* prop_window = Drawables::get_window(prop_w);
	if (!prop_window)
		return BadWindow;

	XWindow* transient_for = Drawables::get_window(prop_window->transient_for);
	if (transient_for && transient_for->bwindow)
		transient_for->bwindow->RemoveFromSubset(prop_window->bwindow);

	transient_for = Drawables::get_window(w);
	prop_window->transient_for = transient_for ? transient_for->id() : None;

	// Note this may not work if the window does not yet have a SUBSET feel.
	if (transient_for && transient_for->bwindow)
		transient_for->bwindow->AddToSubset(prop_window->bwindow);

	return Success;
}

extern "C" int
XMapWindow(Display* display, Window w)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;

	if (window->bwindow) {
		window->bwindow->Show();
	} else {
		window->view()->LockLooper();
		window->view()->Show();
		window->view()->UnlockLooper();
	}

	XWindow* parent = window->parent_window();
	const bool selfNotify = (window->event_mask() & StructureNotifyMask);
	if (selfNotify || (parent && parent->event_mask() & SubstructureNotifyMask)) {
		XEvent event = {};
		event.type = MapNotify;
		event.xmap.event = selfNotify ? window->id() : parent->id();
		event.xmap.window = window->id();
		_x_put_event(display, event);
	}

	return Success;
}

extern "C" int
XUnmapWindow(Display *display, Window w)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;

	if (window->bwindow) {
		window->bwindow->Hide();
	} else {
		window->view()->LockLooper();
		window->view()->Hide();
		window->view()->UnlockLooper();
	}

	XWindow* parent = window->parent_window();
	const bool selfNotify = (window->event_mask() & StructureNotifyMask);
	if (selfNotify || (parent && parent->event_mask() & SubstructureNotifyMask)) {
		XEvent event = {};
		event.type = UnmapNotify;
		event.xunmap.event = selfNotify ? window->id() : parent->id();
		event.xunmap.window = window->id();
		_x_put_event(display, event);
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
XMapSubwindows(Display* display, Window w)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;

	for (const Window& child : window->children())
		XMapWindow(display, child);
	return 1;
}

extern "C" int
XMapRaised(Display* display, Window w)
{
	XMapWindow(display, w);
	return XRaiseWindow(display, w);
}

extern "C" Status
XGetWMName(Display* display, Window w, XTextProperty* name_return)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;

	// TODO: Make this work for parented windows!
	const char* name = NULL;
	if (window->bwindow)
		name = window->bwindow->Title();

	*name_return = {};
	if (name && name[0] != '\0')
		*name_return = make_text_property(Atoms::UTF8_STRING, 8, name, -1, true);
	return Success;
}

extern "C" void
XSetWMName(Display* display, Window w, XTextProperty* name)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return;

	const BString nameStr = _x_text_decode(name);

	// TODO: Make this work for parented windows!
	if (window->bwindow)
		window->bwindow->SetTitle(nameStr.String());
}

extern "C" int
XFetchName(Display* display, Window w, char** window_name_return)
{
	XTextProperty prop = {};
	XGetWMName(display, w, &prop);
	*window_name_return = (char*)prop.value;
	return (prop.value != NULL) ? Success : BadWindow;
}

extern "C" int
XStoreName(Display* display, Window w, const char* wname)
{
	XTextProperty property = make_text_property(XA_STRING, 8, wname, -1);
	XSetWMName(display, w, &property);
	return 0;
}

extern "C" int
XSetWindowBackground(Display *display, Window w, unsigned long bg)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;
	window->background_pixel(bg);
	return Success;
}

extern "C" int
XSetWindowBorder(Display* display, Window w, unsigned long border_pixel)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;
	window->border_pixel(border_pixel);
	return Success;
}

extern "C" int
XClearWindow(Display *display, Window w)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;
	window->draw_border(BRect());
	return Success;
}

extern "C" int
XClearArea(Display *display, Window w,
	int x, int y, unsigned int width, unsigned int height, Bool exposures)
{
	XWindow* window = Drawables::get_window(w);
	if (!window)
		return BadWindow;

	BRect rect(brect_from_xrect(make_xrect(x, y, width, height)));
	window->draw_border(rect);
	if (exposures && window->view()->Window()) {
		window->view()->LockLooper();
		window->view()->Invalidate(rect);
		window->view()->UnlockLooper();
	}
	return Success;
}

extern "C" int
XGetInputFocus(Display* display, Window* focus_return, int* revert_to_return)
{
	XWindow* focus = Drawables::focused();
	*focus_return = focus ? focus->id() : None;
	*revert_to_return = RevertToNone;
	return Success;
}

extern "C" int
XSetInputFocus(Display* display, Window focus, int revert_to, Time time)
{
	XWindow* window = Drawables::get_window(focus);
	if (focus == PointerRoot)
		window = NULL;
	if (window && !window->view()->Window())
		return BadWindow;

	if (time > _x_current_time())
		return Success;
	if (window == Drawables::focused())
		return Success;

	const bool defocus = !window;
	if (defocus)
		window = Drawables::focused();

	window->view()->LockLooper();
	if (!defocus)
		window->view()->Window()->Activate();
	window->view()->MakeFocus(!defocus);
	window->view()->UnlockLooper();
	return Success;
}

extern "C" Bool
XQueryPointer(Display *display, Window w, Window* root_return,
	Window* child_return, int* root_x_return, int* root_y_return,
	int* win_x_return, int *win_y_return,
	unsigned int* mask_return)
{
	XDrawable* window = Drawables::get(w);

	BPoint location, rootLocation;
	uint32 buttons;
	if (window) {
		window->view()->LockLooper();
		window->view()->GetMouse(&location, &buttons, false);
		rootLocation = window->view()->ConvertToScreen(location);
		window->view()->UnlockLooper();
	} else {
		get_mouse(&rootLocation, &buttons);
	}

	*root_x_return = abs(rootLocation.x);
	*root_y_return = abs(rootLocation.y);
	if (!window) {
		*win_x_return = *root_x_return;
		*win_y_return = *root_y_return;

		XWindow* pointer = Drawables::pointer();
		*child_return = pointer ? pointer->id() : None;
	} else {
		*win_x_return = abs(location.x);
		*win_y_return = abs(location.y);

		Window child = None;
		window->contains(location, child);
		if (child != w)
			*child_return = child;
		else
			*child_return = None;
	}

	*root_return = DefaultRootWindow(display);
	*mask_return = _x_get_button_state(-1, buttons);

	return True;
}

extern "C" int
XGrabPointer(Display* display, Window grab_w, Bool owner_events, unsigned int event_mask,
	int pointer_mode, int keyboard_mode, Window confine_to_w, Cursor cursor, Time time)
{
	if (Drawables::pointer_grab()) {
		if (Drawables::pointer_grab()->display() != display)
			return AlreadyGrabbed;
		Drawables::pointer_grab()->ungrab_pointer();
	}

	if (owner_events)
		return GrabSuccess; // No "grabbing" needed.

	XWindow* grab_window = Drawables::get_window(grab_w);
	if (!grab_window || grab_window->view()->Window()->IsHidden())
		return GrabNotViewable;

	// TODO: confine_to_w?

	grab_window->grab_pointer(event_mask);
	return XChangeActivePointerGrab(display, event_mask, cursor, time);
}

extern "C" int
XChangeActivePointerGrab(Display* display, unsigned int event_mask, Cursor cursor, Time time)
{
	XWindow* grab_window = Drawables::pointer_grab();
	grab_window->grab_event_mask(event_mask);
	// TODO: cursor?
	return GrabSuccess;
}

extern "C" int
XUngrabPointer(Display* display, Time time)
{
	XWindow* grab_window = Drawables::pointer_grab();
	if (!grab_window)
		return GrabNotViewable;

	grab_window->ungrab_pointer();
	return GrabSuccess;
}

extern "C" int
XDefineCursor(Display *display, Window w, Cursor cursor)
{
	if (cursor == None)
		return XUndefineCursor(display, w);

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

extern "C" int
XGetNormalHints(Display* display, Window w, XSizeHints* hints)
{
	XWindow* window = Drawables::get_window(w);
	if (!window || !window->bwindow)
		return BadWindow;
	if (!hints)
		return BadValue;

	if (hints->flags & PBaseSize) {
		// Not supported.
	}
	if (hints->flags & PMinSize) {
		float minWidth, minHeight;
		window->bwindow->GetSizeLimits(&minWidth, NULL, &minHeight, NULL);
		hints->min_width = minWidth;
		hints->min_height = minHeight;
	}
	if (hints->flags & PMaxSize) {
		float maxWidth, maxHeight;
		window->bwindow->GetSizeLimits(NULL, &maxWidth, NULL, &maxHeight);
		hints->max_width = maxWidth;
		hints->max_height = maxHeight;
	}
	if (hints->flags & PResizeInc) {
		// Not supported.
	}
	return Success;
}

extern "C" int
XSetNormalHints(Display* display, Window w, XSizeHints* hints)
{
	XWindow* window = Drawables::get_window(w);
	if (!window || !window->bwindow)
		return BadWindow;
	if (!hints)
		return BadValue;

	if (hints->flags & PBaseSize) {
		// Not supported.
	}

	if ((hints->flags & PMinSize) || (hints->flags & PMaxSize)) {
		BSize minSize, maxSize;
		window->bwindow->GetSizeLimits(&minSize.width, &maxSize.width, &minSize.height, &maxSize.height);
		if (hints->flags & PMinSize)
			minSize = brect_from_xrect(make_xrect(0, 0, hints->min_width, hints->min_height)).Size();
		if (hints->flags & PMaxSize)
			maxSize = brect_from_xrect(make_xrect(0, 0, hints->max_width, hints->max_height)).Size();
		window->bwindow->SetSizeLimits(minSize.width, maxSize.width, minSize.height, maxSize.height);
	}

	if (hints->flags & PResizeInc) {
		// Not supported.
	}

	// TODO: Flags?
	return Success;
}

extern "C" Status
XSetWMProtocols(Display* display, Window w, Atom* protocols, int count)
{
	XWindow* window = Drawables::get_window(w);
	if (!window || !window->bwindow)
		return BadWindow;

	window->set_protocols(protocols, count);
	return Success;
}

extern "C" Status
XGetWMNormalHints(Display* display, Window w, XSizeHints* hints_return, long* supplied_return)
{
	Status status = XGetNormalHints(display, w, hints_return);
	*supplied_return = hints_return->flags;
	return status;
}

extern "C" void
XSetWMNormalHints(Display* display, Window w, XSizeHints* hints)
{
	XSetNormalHints(display, w, hints);
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
XSetWMProperties(Display* display, Window w, XTextProperty* window_name, XTextProperty* icon_name,
	char** argv, int argc, XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints)
{
	if (window_name)
		XSetWMName(display, w, window_name);
	if (icon_name)
		XSetWMIconName(display, w, icon_name);
	if (normal_hints)
		XSetWMNormalHints(display, w, normal_hints);
	if (wm_hints)
		XSetWMHints(display, w, wm_hints);
	if (class_hints)
		XSetClassHint(display, w, class_hints);
}

extern "C" Status
XGetSizeHints(Display* display, Window w,
	XSizeHints* hints, Atom property)
{
	switch (property) {
	case XA_WM_NORMAL_HINTS:
		return XGetNormalHints(display, w, hints);

	default: break;
	}

	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" XWMHints*
XAllocWMHints(void)
{
	return (XWMHints*)malloc(sizeof(XWMHints));
}

extern "C" XClassHint*
XAllocClassHint(void)
{
	return (XClassHint*)malloc(sizeof(XClassHint));
}
