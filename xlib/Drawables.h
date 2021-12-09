#pragma once

#include <interface/View.h>
#include <interface/Window.h>
#include <map>

#include "Event.h"

extern "C" {
#include <X11/Xlib.h>
}

// Predeclarations
class XDrawable;
class XPixmap;

class Drawables {
private:
	static std::map<Window, XDrawable*> drawables;
	static Drawable max;

public:
	static XDrawable* any();

	static XDrawable* get(Drawable id);
	static XPixmap* get_pixmap(Pixmap id);

private:
	friend class XDrawable;

	static Drawable add(XDrawable* drawable);
	static void erase(Drawable id);
};

class XDrawable : protected BView {
private:
	Display*const display_;
	Drawable id_;

	BSize base_size_;
	rgb_color bg_color_;
	rgb_color border_color_;
	int border_width_;

	long event_mask_ = 0;
	int last_buttons = 0;

public:
	BWindow* bwindow = NULL;

	GC gc = NULL;
	GC default_gc = NULL;

public:
	XDrawable(Display* dpy, BRect rect);
	virtual ~XDrawable();

	Display* display() { return display_; }

	BView* view() { return this; }
	void create_bwindow();

	BSize size() { return base_size_; }
	virtual bool resize(int width, int height);

	int border_width() { return border_width_; }
	void border_width(int border_width);
	void background_pixel(long bg);
	void border_pixel(long border_color);
	void draw_border(BRect clipRect);

	void contains(const BPoint &point, ::Window& win);

	long event_mask() {
		return event_mask_;
	}
	void event_mask(long mask);

	Drawable id() {
		return id_;
	}

protected:
	virtual void Draw(BRect rect) override;

	virtual void MessageReceived(BMessage* msg) override;

	virtual void MouseDown(BPoint point) override;
	virtual void MouseUp(BPoint point) override;
	virtual void MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage) override;
	void _MouseEvent(int type, BPoint point, int extraButton = 0);

	virtual	void KeyDown(const char* bytes, int32 numBytes) override;
	virtual	void KeyUp(const char* bytes, int32 numBytes) override;
	void _KeyEvent(int type, const char* bytes, int32 numBytes);
};

class XPixmap : public XDrawable {
private:
	BBitmap* offscreen_ = NULL;
	int _depth;

public:
	XPixmap(Display* dpy, BRect frame, unsigned int depth);
	virtual ~XPixmap();

	BBitmap* offscreen() { return offscreen_; }

protected:
	virtual bool resize(int width, int height) override;
};
