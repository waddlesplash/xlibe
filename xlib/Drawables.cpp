#include "Drawables.h"

#include <interface/Bitmap.h>
#include <app/Message.h>

#include "Color.h"
#include "Event.h"

// statics
std::map<Drawable, XDrawable*> Drawables::drawables;
Drawable Drawables::max = 0;

Drawable
Drawables::add(XDrawable* drawable)
{
	max++;
	drawables[max] = drawable;
	return max;
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

XPixmap*
Drawables::get_pixmap(Drawable id)
{
	if (drawables.find(id) == drawables.end())
		return NULL;
	return dynamic_cast<XPixmap*>(drawables[id]);
}

class XBWindow : public BWindow {
	XDrawable* _drawable;

public:
	XBWindow(BRect frame, XDrawable* drawable)
		: BWindow(frame, "*****", B_TITLED_WINDOW, 0, B_CURRENT_WORKSPACE)
		, _drawable(drawable)
	{
	}

	virtual void Show() override;
	virtual void Hide() override;

protected:
	virtual void FrameResized(float newWidth, float newHeight) override;
	virtual bool QuitRequested() override;
};

void
XBWindow::Show()
{
	if (!IsHidden())
		return;
	BWindow::Show();

	if (!(_drawable->event_mask() & StructureNotifyMask))
		return;

	XEvent event = {};
	event.type = MapNotify;
	event.xmap.event = _drawable->id();
	event.xmap.window = _drawable->id();
	x_put_event(_drawable->display(), event);

	// FIXME: Generate MapNotify also for children!
}

void
XBWindow::Hide()
{
	if (IsHidden())
		return;
	BWindow::Hide();

	if (!(_drawable->event_mask() & StructureNotifyMask))
		return;

	XEvent event = {};
	event.type = UnmapNotify;
	event.xunmap.event = _drawable->id();
	event.xunmap.window = _drawable->id();
	x_put_event(_drawable->display(), event);

	// FIXME: Generate UnmapNotify also for children!
}

void
XBWindow::FrameResized(float newWidth, float newHeight)
{
	if (!(_drawable->event_mask() & StructureNotifyMask))
		return;

	_drawable->resize(ceilf(newWidth), ceilf(newHeight));

	XEvent event = {};
	event.type = ConfigureNotify;
	event.xconfigure.event = _drawable->id();
	event.xconfigure.window = _drawable->id();
	event.xconfigure.x = Frame().LeftTop().x;
	event.xconfigure.y = Frame().LeftTop().y;
	event.xconfigure.width = newWidth;
	event.xconfigure.height = newHeight;
	event.xconfigure.border_width = _drawable->border_width();
	x_put_event(_drawable->display(), event);
}

bool
XBWindow::QuitRequested()
{
	Hide();
	return false;
}

XDrawable::XDrawable(Display* dpy, BRect rect)
	: BView(rect, "XDrawable", 0, B_WILL_DRAW)
	, display_(dpy)
	, id_(Drawables::add(this))
	, base_size_(rect.Size())
	, bg_color_(create_rgb(0))
	, border_color_(create_rgb(0))
	, border_width_(0)
{
	resize(rect.IntegerWidth(), rect.IntegerHeight());
}

XDrawable::~XDrawable()
{
	Drawables::erase(id());

	if (Window() || Parent()) {
		BWindow* window = Window();
		if (window)
			window->LockLooper();
		RemoveSelf();
		if (window)
			window->UnlockLooper();
	}

	if (bwindow) {
		bwindow->LockLooper();
		bwindow->Quit();
	}
}

void
XDrawable::create_bwindow()
{
	if (bwindow) {
		debugger("Already have a BWindow.");
		return;
	}

	BWindow* rootWindow = new XBWindow(Frame(), this);
	bwindow = rootWindow;
	MoveTo(0, 0);
	rootWindow->AddChild(this);
}

bool
XDrawable::resize(int width, int height)
{
	if (Window())
		LockLooper();

	BSize borderedSize = BSize(width, height);
	borderedSize.width += border_width_ * 2;
	borderedSize.height += border_width_ * 2;
	if (Bounds().Size() == borderedSize) {
		if (Window())
			UnlockLooper();
		return false; // Nothing to do.
	}
	base_size_ = BSize(width, height);

	ResizeTo(borderedSize);
	if (bwindow)
		bwindow->ResizeTo(borderedSize.Width(), borderedSize.Height());

	if (Window())
		UnlockLooper();
	return true;
}

void
XDrawable::border_width(int border_width)
{
	border_width_ = border_width;
	resize(base_size_.Width(), base_size_.Height());
}

void
XDrawable::background_pixel(long bg)
{
	LockLooper();
	bg_color_ = create_rgb(bg);
	Invalidate();
	UnlockLooper();
}

void
XDrawable::border_pixel(long border_color)
{
	LockLooper();
	border_color_ = create_rgb(border_color);
	Invalidate();
	UnlockLooper();
}

void
XDrawable::draw_border(BRect clipRect)
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
XDrawable::event_mask(long mask)
{
	event_mask_ = mask;
}

void
XDrawable::Draw(BRect rect)
{
	if (!(event_mask() & ExposureMask))
		return;

	draw_border(rect);

	XEvent event;
	event.type = Expose;
	event.xany.window = id();
	event.xexpose.x = rect.LeftTop().x;
	event.xexpose.y = rect.LeftTop().y;
	event.xexpose.width = rect.IntegerWidth();
	event.xexpose.height = rect.IntegerHeight();
	event.xexpose.count = 0;
	x_put_event(display_, event);
}

void
XDrawable::MessageReceived(BMessage* message)
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
XDrawable::MouseDown(BPoint point)
{
	if (!(event_mask() & ButtonPressMask))
		return;

	_MouseEvent(ButtonPress, point);
	MakeFocus(); // FIXME proper focus handling!
}

void
XDrawable::MouseUp(BPoint point)
{
	if (!(event_mask() & ButtonReleaseMask))
		return;

	_MouseEvent(ButtonRelease, point);
}

void
XDrawable::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
	_MouseEvent(MotionNotify, where);
}

void
XDrawable::_MouseEvent(int type, BPoint point, int extraButton)
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
	x_put_event(display_, event);
	last_buttons = buttons;
}

void
XDrawable::KeyDown(const char* bytes, int32 numBytes)
{
	if (!(event_mask() & KeyPressMask))
		return;

	_KeyEvent(KeyPress, bytes, numBytes);
}

void
XDrawable::KeyUp(const char* bytes, int32 numBytes)
{
	if (!(event_mask() & KeyPressMask))
		return;

	_KeyEvent(KeyRelease, bytes, numBytes);
}

void
XDrawable::_KeyEvent(int type, const char* bytes, int32 numBytes)
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

	x_put_event(display_, event);
}

XPixmap::XPixmap(Display* dpy, BRect frame, unsigned int depth)
	: XDrawable(dpy, frame)
	, _depth(depth)
{
	// We store depth, but we ignore it, because ImportBits()
	// only works properly with B_RGB[A]32/
	resize(frame.IntegerWidth(), frame.IntegerHeight());
}

XPixmap::~XPixmap()
{
	offscreen_->Lock();
	RemoveSelf();
	offscreen_->Unlock();

	delete offscreen_;
}

bool
XPixmap::resize(int width, int height)
{
	if (!XDrawable::resize(width, height) && offscreen_)
		return false;

	if (offscreen_) {
		RemoveSelf();
		delete offscreen_;
	}

	offscreen_ = new BBitmap(Frame(), B_RGB32, true);
	offscreen_->AddChild(this);
	return true;
}
