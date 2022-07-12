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

#ifndef XLIBE_DRAWABLES_PROTECTED
#define XLIBE_DRAWABLES_PROTECTED protected
#endif

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
	static XWindow* pointer();
	static XWindow* pointer_grab();

private:
	friend class XDrawable;

	static Drawable add(XDrawable* drawable);
	static void erase(Drawable id);
};

/* We do not want the BView members of this class accessible except through view();
 * but on the other hand, we also need to be able to dynamic_cast<> from BView,
 * which is only possible if the class inherits publicly. Ugh. */
class XDrawable : XLIBE_DRAWABLES_PROTECTED BView {
private:
	Display*const _display;
	Drawable _id;

protected:
	BSize _base_size;

public:
	BBitmap* scratch_bitmap = NULL;

public:
	XDrawable(Display* dpy, BRect rect);
	virtual ~XDrawable() override;

	BView* view() { return this; }
	virtual color_space colorspace() = 0;

	Display* display() const { return _display; }
	Drawable id() const { return _id; }

	BSize size() { return _base_size; }
	virtual bool resize(BSize newSize);

	Drawable parent() const;
	std::list<Drawable> children() const;
	void contains(const BPoint &point, ::Window& win);

	void remove();
};

class XWindow : public XDrawable {
private:
	rgb_color _bg_color;
	rgb_color _border_color;
	int _border_width;

	long _prior_event_mask = 0;
	long _event_mask = 0;
	int last_buttons = 0;
	bool current_focus = false;

public:
	BWindow* bwindow = NULL;

	bool override_redirect = false;
	::Window transient_for = None;

public:
	XWindow(Display* dpy, BRect rect);
	virtual ~XWindow() override;

	std::list<XWindow*> child_windows();
	XWindow* parent_window();

	void create_bwindow();

	virtual color_space colorspace() override;

	virtual bool resize(BSize newSize) override;

	int border_width() { return _border_width; }
	void border_width(int border_width);
	void background_pixel(long bg);
	void border_pixel(long border_color);
	void draw_border(BRect clipRect);

	long event_mask() { return _event_mask; }
	void event_mask(long mask);

	void set_protocols(Atom* protocols, int count);

	void grab_pointer(long mask);
	void grab_event_mask(long mask);
	void ungrab_pointer();

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
	void _MouseCrossing(int type, BPoint point, int mode = NotifyNormal);
	void _MouseEvent(int type, BPoint point, int extraButton = 0);

	virtual	void KeyDown(const char* bytes, int32 numBytes) override;
	virtual	void KeyUp(const char* bytes, int32 numBytes) override;
	void _KeyEvent(int type, const char* bytes, int32 numBytes);
};

class XPixmap : public XDrawable {
private:
	BBitmap* _offscreen = NULL;
	int _depth;

public:
	XPixmap(Display* dpy, BRect frame, unsigned int depth);
	virtual ~XPixmap() override;

	virtual color_space colorspace() override;

	int depth() { return _depth; }
	BBitmap* offscreen() { return _offscreen; }

	void sync();

protected:
	virtual bool resize(BSize newSize) override;
};

} // namespace BeXlib
using namespace BeXlib;
