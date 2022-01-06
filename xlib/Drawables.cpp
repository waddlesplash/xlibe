/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#include "Drawables.h"

#include <interface/Bitmap.h>

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

Drawable
Drawables::add(XDrawable* drawable)
{
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

static void
Drawables_defocus(XWindow* window)
{
	sFocusedWindow.compare_exchange_weak(window, nullptr);
}

// #pragma mark - XDrawable

XDrawable::XDrawable(Display* dpy, BRect rect)
	: BView(rect, "XDrawable", 0, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
	, display_(dpy)
	, id_(Drawables::add(this))
	, base_size_(rect.Size())
{
	resize(rect.Size());
}

XDrawable::~XDrawable()
{
	XFreeGC(display_, default_gc);
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
	base_size_ = newSize;
	ResizeTo(base_size_);

	if (Window())
		UnlockLooper();
	return true;
}

Drawable
XDrawable::parent()
{
	XDrawable* parent = dynamic_cast<XDrawable*>(Parent());
	if (parent)
		return parent->id();
	return None;
}

std::list<Drawable>
XDrawable::children()
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
	, bg_color_(_x_pixel_to_rgb(0))
	, border_color_(_x_pixel_to_rgb(0))
	, border_width_(0)
{
	resize(rect.Size());
}

XWindow::~XWindow()
{
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
	border_width_ = border_width;
	resize(base_size_);
}

void
XWindow::background_pixel(long bg)
{
	LockLooper();
	bg_color_ = _x_pixel_to_rgb(bg);
	Invalidate();
	UnlockLooper();
}

void
XWindow::border_pixel(long border_color)
{
	LockLooper();
	border_color_ = _x_pixel_to_rgb(border_color);
	Invalidate();
	UnlockLooper();
}

bool
XWindow::resize(BSize newSize)
{
	// We intentionally do not invoke the base implementation at all here.

	if (Window())
		LockLooper();

	BSize borderedSize = newSize;
	borderedSize.width += border_width_ * 2;
	borderedSize.height += border_width_ * 2;
	if (Bounds().Size() == borderedSize) {
		if (Window())
			UnlockLooper();
		return false; // Nothing to do.
	}
	base_size_ = newSize;

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
	const BPoint baseOrigin(border_width_, border_width_);
	SetOrigin(baseOrigin);
	PushState();
	SetOrigin(-baseOrigin);
	SetDrawingMode(B_OP_COPY);

	ClipToRect(clipRect);
	SetHighColor(bg_color_);
	if (border_width_ != 0) {
		SetPenSize(border_width_);
		float w = border_width_ / 2;
		BRect frame = Frame();
		BRect drawframe(w, w, frame.Width() - w, frame.Height() - w);
		FillRect(drawframe);
		SetHighColor(border_color_);
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
	event_mask_ = mask;
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
	base_size_ = BSize(Frame().Width() - (border_width_ * 2),
		Frame().Height() - (border_width_ * 2));

	const bool selfNotify = (event_mask() & StructureNotifyMask);
	if (!selfNotify && !(parent_window() && (parent_window()->event_mask() & SubstructureNotifyMask)))
		return;

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
	XRectangle xrect = xrect_from_brect(BRect(BPoint(x, y), base_size_));
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
		_Focus(IsFocus());
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
		if (!(event_mask() & PointerMotionMask))
			return;

		_MouseEvent(MotionNotify, where);
	}
}

void
XWindow::_MouseCrossing(int type, BPoint point)
{
	// TODO: Is this logic correct for child windows?

	BPoint screenPt = ConvertToScreen(point);

	XEvent event = {};
	event.type = type;
	event.xany.window = id();
	event.xcrossing.time = _x_current_time();
	event.xcrossing.x = (int)point.x;
	event.xcrossing.y = (int)point.y;
	event.xcrossing.x_root = (int)screenPt.x;
	event.xcrossing.y_root = (int)screenPt.y;
	event.xcrossing.mode = NotifyNormal;
	event.xcrossing.focus = current_focus;
	_x_put_event(display(), event);
}

void
XWindow::_MouseEvent(int type, BPoint point, int extraButton)
{
	// TODO: Is this logic correct for child windows?

	BMessage* message = Window()->CurrentMessage();
	int32 buttons = 0;
	message->FindInt32("buttons", &buttons);
	BPoint screenPt = ConvertToScreen(point);

	if (type == ButtonRelease)
		buttons = last_buttons & ~buttons;

	XEvent event = {};
	event.type = type;
	event.xany.window = id();
	event.xbutton.time = _x_current_time();
	event.xbutton.x = (int)point.x;
	event.xbutton.y = (int)point.y;
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
	offscreen_->Lock();
	RemoveSelf();
	offscreen_->Unlock();

	delete offscreen_;
}

bool
XPixmap::resize(BSize newSize)
{
	if (!XDrawable::resize(newSize) && offscreen_ != NULL)
		return false;

	if (offscreen_) {
		RemoveSelf();
		delete offscreen_;
	}

	offscreen_ = new BBitmap(Frame(), _x_color_space(NULL, _depth), true);
	memset(offscreen_->Bits(), 0, offscreen_->BitsLength());
	offscreen_->AddChild(this);
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
