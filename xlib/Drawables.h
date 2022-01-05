/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <interface/View.h>
#include <interface/Window.h>

#include <map>
#include <list>

#include "Event.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

namespace BeXlib {

// Predeclarations
class XDrawable;
class XWindow;
class XPixmap;

class Drawables {
private:
	static pthread_rwlock_t lock;
	static std::map<Window, XDrawable*> drawables;
	static Drawable last;

public:
	static XDrawable* get(Drawable id);
	static XWindow* get_window(Window id);
	static XPixmap* get_pixmap(Pixmap id);

	static XWindow* focused();

private:
	friend class XDrawable;

	static Drawable add(XDrawable* drawable);
	static void erase(Drawable id);
};

class XDrawable : protected BView {
private:
	Display*const display_;
	Drawable id_;

protected:
	BSize base_size_;

public:
	GC gc = NULL;
	GC default_gc = NULL;

public:
	XDrawable(Display* dpy, BRect rect);
	virtual ~XDrawable();

	BView* view() { return this; }

	Display* display() { return display_; }
	Drawable id() { return id_; }

	BSize size() { return base_size_; }
	virtual bool resize(BSize newSize);

	Drawable parent();
	std::list<Drawable> children();
	void contains(const BPoint &point, ::Window& win);

	void remove();
};

class XWindow : public XDrawable {
private:
	rgb_color bg_color_;
	rgb_color border_color_;
	int border_width_;

	long event_mask_ = 0;
	int last_buttons = 0;
	bool current_focus = false;

public:
	BWindow* bwindow = NULL;

public:
	XWindow(Display* dpy, BRect rect);
	virtual ~XWindow() override;

	void create_bwindow();

	virtual bool resize(BSize newSize) override;

	int border_width() { return border_width_; }
	void border_width(int border_width);
	void background_pixel(long bg);
	void border_pixel(long border_color);
	void draw_border(BRect clipRect);

	long event_mask() { return event_mask_; }
	void event_mask(long mask);

	void set_protocols(Atom* protocols, int count);

protected:
	virtual void MessageReceived(BMessage* msg) override;

	virtual void Draw(BRect rect) override;
	virtual void DrawAfterChildren(BRect rect) override;
	void _Expose(BRect rect);

	virtual void FrameMoved(BPoint to) override;
	virtual void FrameResized(float newWidth, float newHeight) override;
	void _Configured();

	virtual void MakeFocus(bool focus) override;
	virtual void WindowActivated(bool active) override;
	void _Focus(bool focus);

	virtual void MouseDown(BPoint point) override;
	virtual void MouseUp(BPoint point) override;
	virtual void MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage) override;
	void _MouseCrossing(int type, BPoint point);
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
	virtual ~XPixmap() override;

	int depth() { return _depth; }
	BBitmap* offscreen() { return offscreen_; }

	void sync();

protected:
	virtual bool resize(BSize newSize) override;
};

} // namespace BeXlib
using namespace BeXlib;
