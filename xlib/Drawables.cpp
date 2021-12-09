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

XDrawable::XDrawable(BRect rect)
	: BView(rect, "XDrawable", 0, B_WILL_DRAW)
	, id_(Drawables::add(this))
	, event_mask_(0)
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

	// FIXME: bwindow deletes on close!
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

	BWindow* rootWindow = new BWindow(Frame(),
		"*****", B_TITLED_WINDOW, 0, B_CURRENT_WORKSPACE);
	bwindow = rootWindow;
	MoveTo(0, 0);
	rootWindow->AddChild(this);
}

void
XDrawable::resize(int width, int height)
{
	if (Window())
		LockLooper();
	base_size_ = BSize(width+100, height+100);
	ResizeTo(base_size_.Width() + border_width_ * 2,
		base_size_.Height() + border_width_ * 2);
	if (bwindow)
		bwindow->ResizeTo(Bounds().Width(), Bounds().Height());
	if (Window())
		UnlockLooper();
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

	SetMouseEventMask((event_mask() &
			(PointerMotionMask | Button1MotionMask | Button2MotionMask)) ?
		B_POINTER_EVENTS : 0);
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
	Events::instance().add(event);
}

void
XDrawable::MouseDown(BPoint point)
{
	if (!(event_mask() & ButtonPressMask))
		return;

	_MouseEvent(ButtonPress, point);
}

void
XDrawable::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
	_MouseEvent(MotionNotify, where);
}

void
XDrawable::_MouseEvent(int type, BPoint point)
{
	// TODO: Is this logic correct for child windows?

	int32 buttons = 0;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	XEvent event;
	event.type = type;
	event.xany.window = id();
	event.xbutton.x = (int)point.x;
	event.xbutton.y = (int)point.y;
	if (buttons & B_MOUSE_BUTTON(1)) {
		event.xbutton.state |= Button1Mask;
		event.xbutton.button = 2;
	}
	if (buttons & B_MOUSE_BUTTON(2)) {
		event.xbutton.state |= Button2Mask;
		event.xbutton.button = 2;
	}
	if (buttons & B_MOUSE_BUTTON(3)) {
		event.xbutton.state |= Button3Mask;
		event.xbutton.button = 3;
	}
	Events::instance().add(event);
}

XPixmap::XPixmap(BRect frame, unsigned int depth)
	: XDrawable(frame)
{
	// FIXME: take "depth" into account!
	resize(frame.IntegerWidth(), frame.IntegerHeight());
}

XPixmap::~XPixmap()
{
	LockLooper();
	RemoveSelf();
	UnlockLooper();

	delete offscreen_;
}

void
XPixmap::resize(int width, int height)
{
	XDrawable::resize(width, height);

	if (offscreen_) {
		RemoveSelf();
		delete offscreen_;
	}

	offscreen_ = new BBitmap(Frame(), B_RGB32, true);
	offscreen_->AddChild(this);
}
