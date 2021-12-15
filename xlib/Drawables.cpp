#include "Drawables.h"

#include <interface/Bitmap.h>
#include <app/Message.h>

#include "Color.h"
#include "Event.h"
#include "Drawing.h"

namespace BeXlib {

// statics
std::map<Drawable, XDrawable*> Drawables::drawables;
Drawable Drawables::last = 100000;
XWindow* Drawables::_focused = NULL;

Drawable
Drawables::add(XDrawable* drawable)
{
	last++;
	drawables[last] = drawable;
	return last;
}

void
Drawables::erase(Drawable id)
{
	drawables.erase(id);
}

XDrawable*
Drawables::any()
{
	return drawables.begin()->second;
}

XDrawable*
Drawables::get(Drawable id)
{
	if (id == 0)
		return NULL;
	return drawables[id];
}

XWindow*
Drawables::get_window(Drawable id)
{
	if (drawables.find(id) == drawables.end())
		return NULL;
	return dynamic_cast<XWindow*>(drawables[id]);
}

XPixmap*
Drawables::get_pixmap(Drawable id)
{
	if (drawables.find(id) == drawables.end())
		return NULL;
	return dynamic_cast<XPixmap*>(drawables[id]);
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
	XWindow* _window;

public:
	RootWindow(BRect frame, XWindow* window)
		: BWindow(frame, "*****", B_TITLED_WINDOW, 0, B_CURRENT_WORKSPACE)
		, _window(window)
	{
	}

	virtual void Show() override;
	virtual void Hide() override;

protected:
	virtual void FrameResized(float newWidth, float newHeight) override;
	virtual bool QuitRequested() override;
};

void
RootWindow::Show()
{
	if (!IsHidden())
		return;
	BWindow::Show();

	if (!CurrentFocus()) {
		LockLooper();
		_window->view()->MakeFocus(true);
		UnlockLooper();
	}

	if (!(_window->event_mask() & StructureNotifyMask))
		return;

	XEvent event = {};
	event.type = MapNotify;
	event.xmap.event = _window->id();
	event.xmap.window = _window->id();
	_x_put_event(_window->display(), event);

	// FIXME: Generate MapNotify also for children!
}

void
RootWindow::Hide()
{
	if (IsHidden())
		return;
	BWindow::Hide();

	if (!(_window->event_mask() & StructureNotifyMask))
		return;

	XEvent event = {};
	event.type = UnmapNotify;
	event.xunmap.event = _window->id();
	event.xunmap.window = _window->id();
	_x_put_event(_window->display(), event);

	// FIXME: Generate UnmapNotify also for children!
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
	remove();

	if (bwindow) {
		bwindow->LockLooper();
		bwindow->Quit();
	}
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
	// FIXME: Coordinates for drawing do not take border_width into account?
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
	PushState();
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
XWindow::FrameResized(float newWidth, float newHeight)
{
	base_size_ = BSize(newWidth - (border_width_ * 2), newHeight - (border_width_ * 2));

	if (!(event_mask() & StructureNotifyMask))
		return;

	int x = Frame().LeftTop().x, y = Frame().LeftTop().y;
	if (bwindow) {
		x = bwindow->Frame().LeftTop().x;
		y = bwindow->Frame().LeftTop().y;
	}

	XEvent event = {};
	XRectangle xrect = xrect_from_brect(BRect(BPoint(x, y), base_size_));
	event.type = ConfigureNotify;
	event.xconfigure.event = id();
	event.xconfigure.window = id();
	event.xconfigure.x = xrect.x;
	event.xconfigure.y = xrect.y;
	event.xconfigure.width = xrect.width;
	event.xconfigure.height = xrect.height;
	event.xconfigure.border_width = border_width();
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
		Drawables::focused(NULL);
	else if (focus)
		Drawables::focused(this);
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
XWindow::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
	_MouseEvent(MotionNotify, where);
}

void
XWindow::_MouseEvent(int type, BPoint point, int extraButton)
{
	// TODO: Is this logic correct for child windows?

	int32 buttons = 0;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	BPoint screenPt = ConvertToScreen(point);

	if (type == ButtonRelease)
		buttons = last_buttons & ~buttons;

	XEvent event;
	event.type = type;
	event.xany.window = id();
	event.xbutton.x = (int)point.x;
	event.xbutton.y = (int)point.y;
	event.xbutton.x_root = (int)screenPt.x;
	event.xbutton.y_root = (int)screenPt.y;
	if (buttons & B_MOUSE_BUTTON(1)) {
		event.xbutton.state |= Button1Mask;
		event.xbutton.button = 1;
	}
	if (buttons & B_MOUSE_BUTTON(2)) {
		event.xbutton.state |= Button2Mask;
		event.xbutton.button = 2;
	}
	if (buttons & B_MOUSE_BUTTON(3)) {
		event.xbutton.state |= Button3Mask;
		event.xbutton.button = 3;
	}
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
	int32 keycode = 0, modifiers = 0;
	message->FindInt32("raw_char", &keycode);
	message->FindInt32("modifiers", &modifiers);

	XEvent event = {};
	event.type = type;
	event.xkey.window = id();
	event.xkey.keycode = keycode;

	// abuse the "serial" field to store the bytes
	event.xkey.serial = 0;
	memcpy(&event.xany.serial, bytes,
		min_c(numBytes, sizeof(event.xany.serial)));

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
