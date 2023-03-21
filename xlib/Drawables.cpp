/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#define XLIBE_DRAWABLES_PROTECTED public
#include "Drawables.h"

#include <interface/Bitmap.h>
#include <interface/Screen.h>

#include <set>
#include <atomic>

#include "Atom.h"
#include "Color.h"
#include "Keyboard.h"
#include "Event.h"
#include "Drawing.h"
#include "Locking.h"

namespace BeXlib {

// statics
pthread_rwlock_t Drawables::lock = PTHREAD_RWLOCK_INITIALIZER;
std::map<Drawable, XDrawable*> Drawables::drawables;
Drawable Drawables::last = 100000;

static std::atomic<XWindow*> sFocusedWindow, sPointerWindow;
static XWindow* sPointerGrabWindow = NULL;

Drawable
Drawables::add(XDrawable* drawable)
{
	if (last <= DefaultRootWindow(drawable->display()))
		debugger("IDs wrapped?!");

	PthreadWriteLocker wrlock(lock);
	last++;
	drawables[last] = drawable;
	return last;
}

void
Drawables::erase(Drawable id)
{
	PthreadWriteLocker wrlock(lock);
	drawables.erase(id);
}

XDrawable*
Drawables::get(Drawable id)
{
	if (id == None)
		return NULL;

	PthreadReadLocker rdlock(lock);
	auto drawable = drawables.find(id);
	if (drawable == drawables.end())
		return NULL;
	return drawable->second;
}

XWindow*
Drawables::get_window(Drawable id)
{
	return dynamic_cast<XWindow*>(get(id));
}

XPixmap*
Drawables::get_pixmap(Drawable id)
{
	return dynamic_cast<XPixmap*>(get(id));
}

XWindow*
Drawables::focused()
{
	return sFocusedWindow;
}

XWindow*
Drawables::pointer()
{
	return sPointerWindow;
}

XWindow*
Drawables::pointer_grab()
{
	return sPointerGrabWindow;
}

static void
Drawables_defocus(XWindow* window)
{
	sFocusedWindow.compare_exchange_weak(window, nullptr);
}

// #pragma mark - XDrawable

XDrawable::XDrawable(Display* dpy, BRect rect)
	: BView(rect, "XDrawable", 0, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
	, _display(dpy)
	, _id(Drawables::add(this))
	, _base_size(rect.Size())
{
	resize(rect.Size());
}

XDrawable::~XDrawable()
{
	delete scratch_bitmap;
	Drawables::erase(id());
	remove();
}

bool
XDrawable::resize(BSize newSize)
{
	if (Window())
		LockLooper();

	if (Bounds().Size() == newSize) {
		if (Window())
			UnlockLooper();
		return false; // Nothing to do.
	}
	_base_size = newSize;
	ResizeTo(_base_size);

	if (Window())
		UnlockLooper();
	return true;
}

Drawable
XDrawable::parent() const
{
	XDrawable* parent = dynamic_cast<XDrawable*>(Parent());
	if (parent)
		return parent->id();

	// Check if we are a "root window."
	const XWindow* self = dynamic_cast<const XWindow*>(this);
	if (self && self->bwindow)
		return DefaultRootWindow(display());

	return None;
}

std::list<Drawable>
XDrawable::children() const
{
	std::list<Drawable> ret;
	for (int i = 0; i < CountChildren(); i++) {
		XDrawable* child = dynamic_cast<XDrawable*>(ChildAt(i));
		if (child)
			ret.push_back(child->id());
	}
	return ret;
}

void
XDrawable::contains(const BPoint& point, Drawable& win)
{
	LockLooper();
	int max = CountChildren();
	if (Frame().Contains(point))
		win = id();
	for (int i = 0; i != max; i++) {
		XDrawable* drawable = dynamic_cast<XDrawable*>(ChildAt(i));
		if (drawable)
			drawable->contains(point, win);
	}
	UnlockLooper();
}

void
XDrawable::remove()
{
	if (Window() || Parent()) {
		BWindow* window = Window();
		if (window)
			window->LockLooper();
		RemoveSelf();
		if (window)
			window->UnlockLooper();
	}
}

// #pragma mark - XWindow

namespace {

#undef RootWindow
class RootWindow : public BWindow {
	friend class ::BeXlib::XWindow;

	XWindow* _window;
	std::set<Atom> _protocols;

public:
	RootWindow(BRect frame, XWindow* window)
		: BWindow(frame, "*****", B_TITLED_WINDOW, 0, B_CURRENT_WORKSPACE)
		, _window(window)
	{
	}

	virtual void Show() override;
	virtual void Hide() override;

protected:
	virtual void DispatchMessage(BMessage* message, BHandler* target) override;

	virtual void FrameMoved(BPoint to) override;
	virtual void FrameResized(float newWidth, float newHeight) override;

	virtual bool QuitRequested() override;
};

void
RootWindow::Show()
{
	if (!IsHidden())
		return;
	BWindow::Show();

	LockLooper();
	if (!CurrentFocus())
		_window->view()->MakeFocus(true);
	UnlockLooper();

	// Adjust so that the window border is not offscreen, if necessary.
	if (Frame().LeftTop() == BPoint(0, 0) && Look() != B_NO_BORDER_WINDOW_LOOK)
		MoveOnScreen(B_MOVE_IF_PARTIALLY_OFFSCREEN | B_DO_NOT_RESIZE_TO_FIT);
}

void
RootWindow::Hide()
{
	if (IsHidden())
		return;
	BWindow::Hide();
}

void
RootWindow::DispatchMessage(BMessage* message, BHandler* handler)
{
	switch (message->what) {
	case B_KEY_DOWN:
	case B_KEY_UP: {
		// Bypass all of BWindow's special key event handling.
		uint32 raw_char = message->FindInt32("raw_char");
		uint32 key = message->FindInt32("key");
		uint32 modifiers = message->FindInt32("modifiers");

		// Special case: Don't bypass on CNTRL+Tab.
		if (raw_char == B_TAB && (modifiers & B_CONTROL_KEY))
			break;
		// Special case: Don't bypass on PrntScrn.
		if (raw_char == B_FUNCTION_KEY && key == B_PRINT_KEY)
			break;

		handler->MessageReceived(message);
		return;
	}
	}

	BWindow::DispatchMessage(message, handler);
}

void
RootWindow::FrameMoved(BPoint to)
{
	BWindow::FrameMoved(to);

	_window->view()->FrameMoved(_window->view()->Frame().LeftTop());
}

void
RootWindow::FrameResized(float newWidth, float newHeight)
{
	BWindow::FrameResized(newWidth, newHeight);

	_window->view()->ResizeTo(newWidth, newHeight);
}

bool
RootWindow::QuitRequested()
{
	if (_protocols.count(Atoms::WM_DELETE_WINDOW)) {
		// Do not hide at all, but send a delete-window event.
		XEvent event = {};
		event.type = ClientMessage;
		event.xclient.window = _window->id();
		event.xclient.message_type = Atoms::WM_PROTOCOLS;
		event.xclient.format = 32;
		event.xclient.data.l[0] = Atoms::WM_DELETE_WINDOW;
		event.xclient.data.l[1] = _x_current_time();
		_x_put_event(_window->display(), event);
		return false;
	}

	Hide();
	return false;
}

}

XWindow::XWindow(Display* dpy, BRect rect)
	: XDrawable(dpy, rect)
	, _bg_color(_x_pixel_to_rgb(0))
	, _border_color(_x_pixel_to_rgb(0))
	, _border_width(0)
{
	resize(rect.Size());
}

XWindow::~XWindow()
{
	if (sPointerGrabWindow == this)
		ungrab_pointer();

	// Delete all children before sending our own DestroyNotify.
	LockLooper();
	while (CountChildren())
		delete ChildAt(0);
	UnlockLooper();

	const bool selfNotify = (event_mask() & StructureNotifyMask);
	if (selfNotify || (parent_window() && (parent_window()->event_mask() & SubstructureNotifyMask))) {
		XEvent event = {};
		event.type = DestroyNotify;
		event.xdestroywindow.event = selfNotify ? id() : parent();
		event.xdestroywindow.window = id();
		_x_put_event(display(), event);
	}

	Drawables_defocus(this);
	remove();

	if (bwindow) {
		bwindow->LockLooper();
		bwindow->Quit();
	}
}

std::list<XWindow*>
XWindow::child_windows()
{
	std::list<XWindow*> windows;
	for (int i = 0; i < CountChildren(); i++) {
		XWindow* child = dynamic_cast<XWindow*>(ChildAt(i));
		if (!child)
			continue;
		windows.push_back(child);
	}
	return windows;
}

XWindow*
XWindow::parent_window()
{
	return dynamic_cast<XWindow*>(Parent());
}

void
XWindow::create_bwindow()
{
	if (bwindow) {
		debugger("Already have a BWindow.");
		return;
	}

	BWindow* rootWindow = new RootWindow(Frame(), this);
	bwindow = rootWindow;
	MoveTo(0, 0);
	rootWindow->AddChild(this);
}

void
XWindow::border_width(int border_width)
{
	_border_width = border_width;
	resize(_base_size);
}

void
XWindow::background_pixel(long bg)
{
	_bg_color = _x_pixel_to_rgb(bg);
}

void
XWindow::border_pixel(long border_color)
{
	LockLooper();
	_border_color = _x_pixel_to_rgb(border_color);
	Invalidate();
	UnlockLooper();
}

color_space
XWindow::colorspace()
{
	return BScreen(Window()).ColorSpace();
}

void
XWindow::minimum_size(int width, int height)
{
	_min_size = brect_from_xrect(make_xrect(0, 0, width, height)).Size();
	resize(_base_size);
}

void
XWindow::maximum_size(int width, int height)
{
	_max_size = brect_from_xrect(make_xrect(0, 0, width, height)).Size();
	resize(_base_size);
}

bool
XWindow::resize(BSize newSize)
{
	// We intentionally do not invoke the base implementation at all here.

	if (Window())
		LockLooper();

	if (bwindow) {
		BSize borderedMinSize(B_SIZE_UNSET, B_SIZE_UNSET),
			borderedMaxSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED);
		if (_min_size != BSize()) {
			borderedMinSize = _min_size;
			borderedMinSize.width += _border_width * 2;
			borderedMinSize.height += _border_width * 2;
		}
		if (_max_size != BSize()) {
			borderedMaxSize = _max_size;
			borderedMaxSize.width += _border_width * 2;
			borderedMaxSize.height += _border_width * 2;
		}
		bwindow->SetSizeLimits(borderedMinSize.width, borderedMaxSize.width,
			borderedMinSize.height, borderedMaxSize.height);
	}

	BSize borderedSize = newSize;
	borderedSize.width += _border_width * 2;
	borderedSize.height += _border_width * 2;
	if (Bounds().Size() == borderedSize) {
		if (Window())
			UnlockLooper();
		return false; // Nothing to do.
	}
	_base_size = newSize;

	ResizeTo(borderedSize);
	if (bwindow)
		bwindow->ResizeTo(borderedSize.Width(), borderedSize.Height());

	if (Window())
		UnlockLooper();
	return true;
}

void
XWindow::draw_border(BRect clipRect)
{
	LockLooper();
	const BPoint baseOrigin(_border_width, _border_width);
	SetOrigin(baseOrigin);

	PushState();
	SetOrigin(-baseOrigin);
	SetDrawingMode(B_OP_COPY);

	ClipToRect(clipRect);
	SetHighColor(_bg_color);
	if (_border_width != 0) {
		SetPenSize(_border_width);
		float w = _border_width / 2;
		BRect frame = Frame();
		BRect drawframe(w, w, frame.Width() - w, frame.Height() - w);
		FillRect(drawframe);
		SetHighColor(_border_color);
		StrokeRect(drawframe);
	} else {
		FillRect(Frame());
	}

	PopState();
	UnlockLooper();
}

void
XWindow::event_mask(long mask)
{
	_event_mask = mask;
}

void
XWindow::set_protocols(Atom* protocols, int count)
{
	RootWindow* window = static_cast<RootWindow*>(bwindow);
	window->_protocols.clear();
	for (int i = 0; i < count; i++)
		window->_protocols.insert(protocols[i]);
}

void
XWindow::grab_pointer(long mask)
{
	if (sPointerGrabWindow != NULL)
		debugger("pointer already grabbed");

	sPointerGrabWindow = this;

	LockLooper();
	SetEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);

	_prior_event_mask = _event_mask;
	grab_event_mask(mask);

	BPoint location;
	GetMouse(&location, NULL, false);
	_MouseCrossing(EnterNotify, location, NotifyGrab);
	UnlockLooper();
}

void
XWindow::grab_event_mask(long mask)
{
	if (sPointerGrabWindow != this)
		debugger("Not the grab window!");

	_event_mask = mask;
}

void
XWindow::ungrab_pointer()
{
	if (sPointerGrabWindow != this)
		return;

	LockLooper();

	// We have to call twice: first to unset the mask, then to add the option.
	SetEventMask(0, 0);
	SetEventMask(0, B_NO_POINTER_HISTORY);

	_event_mask = _prior_event_mask;
	sPointerGrabWindow = NULL;

	BPoint location;
	GetMouse(&location, NULL, false);
	_MouseCrossing(LeaveNotify, location, NotifyUngrab);
	UnlockLooper();
}

void
XWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
	case B_MOUSE_WHEEL_CHANGED: {
		float deltaY = 0.0f;
		message->FindFloat("be:wheel_delta_y", &deltaY);
		if (deltaY == 0)
			break;

		BPoint where;
		GetMouse(&where, NULL, false);
		int button = deltaY < 0 ? 4 : 5;
		_MouseEvent(ButtonPress, where, button);
		_MouseEvent(ButtonRelease, where, button);
	} break;
	}

	BView::MessageReceived(message);
}

void
XWindow::Draw(BRect rect)
{
	if (Flags() & B_DRAW_ON_CHILDREN)
		return;

	_Expose(rect);
}

void
XWindow::DrawAfterChildren(BRect rect)
{
	if (!(Flags() & B_DRAW_ON_CHILDREN))
		return;

	_Expose(rect);
}

void
XWindow::_Expose(BRect rect)
{
	if (!(event_mask() & ExposureMask))
		return;

	draw_border(rect);

	// Translate the rectangle into X11 coordinates.
	rect.SetLeftTop(rect.LeftTop() - Origin());
	if (rect.top < 0) {
		rect.bottom -= rect.top;
		rect.top = 0;
	}
	if (rect.left < 0) {
		rect.right -= rect.left;
		rect.left = 0;
	}
	if (rect.Width() < 0 || rect.Height() < 0)
		return;

	XEvent event;
	XRectangle exposed = xrect_from_brect(rect);
	event.type = Expose;
	event.xany.window = id();
	event.xexpose.x = exposed.x;
	event.xexpose.y = exposed.y;
	event.xexpose.width = exposed.width;
	event.xexpose.height = exposed.height;
	event.xexpose.count = 0;
	_x_put_event(display(), event);
}

void
XWindow::FrameMoved(BPoint)
{
	_Configured();
}

void
XWindow::FrameResized(float, float)
{
	_Configured();
}

void
XWindow::_Configured()
{
	_base_size = BSize(Frame().Width() - (_border_width * 2),
		Frame().Height() - (_border_width * 2));

	const bool selfNotify = (event_mask() & StructureNotifyMask);
	if (!selfNotify && !(parent_window() && (parent_window()->event_mask() & SubstructureNotifyMask))) {
		_Visibility();
		return;
	}

	int x = Frame().LeftTop().x, y = Frame().LeftTop().y;
	if (bwindow) {
		x = bwindow->Frame().LeftTop().x;
		y = bwindow->Frame().LeftTop().y;
	}

	Drawable above = None;
	if (PreviousSibling()) {
		XDrawable* previous = dynamic_cast<XDrawable*>(PreviousSibling());
		if (previous)
			above = previous->id();
	}

	XEvent event = {};
	XRectangle xrect = xrect_from_brect(BRect(BPoint(x, y), _base_size));
	event.type = ConfigureNotify;
	event.xconfigure.event = selfNotify ? id() : parent();
	event.xconfigure.window = id();
	event.xconfigure.x = xrect.x;
	event.xconfigure.y = xrect.y;
	event.xconfigure.width = xrect.width;
	event.xconfigure.height = xrect.height;
	event.xconfigure.border_width = border_width();
	event.xconfigure.above = above;
	event.xconfigure.override_redirect = override_redirect;
	_x_put_event(display(), event);

	_Visibility();
}

void
XWindow::_Visibility()
{
	// TODO: Do we actually send this event sufficiently often?
	if (!(event_mask() & VisibilityChangeMask))
		return;

	int state = -1;
	if (IsHidden() || !Window() || (Window() && Window()->IsMinimized())) {
		state = VisibilityFullyObscured;
	} else {
		// TODO: This is not very accurate.
		if (NextSibling() || !Window()->IsActive())
			state = VisibilityPartiallyObscured;
		else
			state = VisibilityUnobscured;
	}

	XEvent event = {};
	event.type = VisibilityNotify;
	event.xvisibility.window = id();
	event.xvisibility.state = state;
	_x_put_event(display(), event);
}

void
XWindow::MakeFocus(bool focus)
{
	if (focus == IsFocus())
		return;
	BView::MakeFocus(focus);

	if (!focus || Window()->IsActive())
		_Focus(focus);
}

void
XWindow::WindowActivated(bool active)
{
	if (!active || (active && (IsFocus() != current_focus)))
		_Focus(active && IsFocus());
	_Visibility();
}

void
XWindow::_Focus(bool focus)
{
	if (focus == current_focus)
		return;

	if (!focus && Drawables::focused() == this)
		Drawables_defocus(this);
	else if (focus)
		sFocusedWindow = this;
	current_focus = focus;

	if (!(event_mask() & FocusChangeMask))
		return;

	XEvent event = {};
	event.type = focus ? FocusIn : FocusOut;
	event.xfocus.window = id();
	event.xfocus.mode = NotifyNormal;
	event.xfocus.detail = NotifyDetailNone;
	_x_put_event(display(), event);
}

void
XWindow::MouseDown(BPoint point)
{
	MakeFocus(true);

	if (!(event_mask() & ButtonPressMask))
		return;

	_MouseEvent(ButtonPress, point);
}

void
XWindow::MouseUp(BPoint point)
{
	if (!(event_mask() & ButtonReleaseMask))
		return;

	_MouseEvent(ButtonRelease, point);
}

void
XWindow::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (transit == B_ENTERED_VIEW || transit == B_EXITED_VIEW) {
		if (transit == B_ENTERED_VIEW) {
			sPointerWindow = this;
		} else if (transit == B_EXITED_VIEW) {
			XWindow* compare = this;
			sPointerWindow.compare_exchange_weak(compare, nullptr);
		}

		if (transit == B_ENTERED_VIEW && !(event_mask() & EnterWindowMask))
			return;
		if (transit == B_EXITED_VIEW && !(event_mask() & LeaveWindowMask))
			return;

		_MouseCrossing(transit == B_ENTERED_VIEW ? EnterNotify : LeaveNotify, where);
	} else {
		if (!(event_mask() & PointerMotionMask)) {
			if ((event_mask() & ButtonMotionMask)) {
				int32 buttons = 0;
				Window()->CurrentMessage()->FindInt32("buttons", &buttons);
				if (!buttons)
					return;
			} else
				return;
		}

		_MouseEvent(MotionNotify, where);
	}
}

void
XWindow::_MouseCrossing(int type, BPoint point, int mode)
{
	if (sPointerGrabWindow && sPointerGrabWindow != this)
		return;

	// TODO: Is this logic correct for child windows?

	const BPoint screenPt = ConvertToScreen(point);
	const BPoint xPoint = point - Origin();

	XEvent event = {};
	event.type = type;
	event.xany.window = id();
	event.xcrossing.root = DefaultRootWindow(display());
	event.xcrossing.time = _x_current_time();
	event.xcrossing.x = (int)xPoint.x;
	event.xcrossing.y = (int)xPoint.y;
	event.xcrossing.x_root = (int)screenPt.x;
	event.xcrossing.y_root = (int)screenPt.y;
	event.xcrossing.mode = mode;
	event.xcrossing.focus = current_focus;
	_x_put_event(display(), event);
}

void
XWindow::_MouseEvent(int type, BPoint point, int extraButton)
{
	if (sPointerGrabWindow && sPointerGrabWindow != this)
		return;

	// TODO: Is this logic correct for child windows?

	BMessage* message = Window()->CurrentMessage();
	int32 buttons = 0;
	message->FindInt32("buttons", &buttons);
	const BPoint screenPt = ConvertToScreen(point);
	const BPoint xPoint = point - Origin();

	if (type == ButtonRelease)
		buttons = last_buttons & ~buttons;

	XEvent event = {};
	event.type = type;
	event.xbutton.window = id();
	event.xbutton.root = DefaultRootWindow(display());
	contains(point, event.xbutton.subwindow);
	event.xbutton.time = _x_current_time();
	event.xbutton.x = (int)xPoint.x;
	event.xbutton.y = (int)xPoint.y;
	event.xbutton.x_root = (int)screenPt.x;
	event.xbutton.y_root = (int)screenPt.y;
	event.xbutton.state = _x_get_button_state(message);
	if (buttons & B_MOUSE_BUTTON(2))
		event.xbutton.button = 3;
	if (buttons & B_MOUSE_BUTTON(3))
		event.xbutton.button = 2;
	if (buttons & B_MOUSE_BUTTON(1))
		event.xbutton.button = 1;
	if (extraButton)
		event.xbutton.button = extraButton;
	_x_put_event(display(), event);
	last_buttons = buttons;
}

void
XWindow::KeyDown(const char* bytes, int32 numBytes)
{
	if (!(event_mask() & KeyPressMask))
		return;

	_KeyEvent(KeyPress, bytes, numBytes);
}

void
XWindow::KeyUp(const char* bytes, int32 numBytes)
{
	if (!(event_mask() & KeyPressMask))
		return;

	_KeyEvent(KeyRelease, bytes, numBytes);
}

void
XWindow::_KeyEvent(int type, const char* bytes, int32 numBytes)
{
	BMessage* message = Looper()->CurrentMessage();

	XEvent event = {};
	event.type = type;
	event.xkey.window = id();
	event.xkey.time = _x_current_time();
	_x_fill_key_event(&event, message, bytes, numBytes);
	_x_put_event(display(), event);
}

// #pragma mark - XPixmap

XPixmap::XPixmap(Display* dpy, BRect frame, unsigned int depth)
	: XDrawable(dpy, frame)
	, _depth((depth < 8) ? 8 : depth)
{
	resize(frame.Size());
}

XPixmap::~XPixmap()
{
	_offscreen->Lock();
	RemoveSelf();
	_offscreen->Unlock();

	delete _offscreen;
}

color_space
XPixmap::colorspace()
{
	return _offscreen->ColorSpace();
}

bool
XPixmap::resize(BSize newSize)
{
	if (!XDrawable::resize(newSize) && _offscreen != NULL)
		return false;

	if (_offscreen) {
		RemoveSelf();
		delete _offscreen;
	}

	_offscreen = new BBitmap(Frame(), _x_color_space_for(NULL, _depth), true);
	memset(_offscreen->Bits(), 0, _offscreen->BitsLength());
	_offscreen->AddChild(this);
	return true;
}

void
XPixmap::sync()
{
	LockLooper();
	Sync();
	UnlockLooper();
}

} // namespace BeXlib
