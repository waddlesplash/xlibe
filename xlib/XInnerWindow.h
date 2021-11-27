#ifndef X_INNER_WINDOW_H
#define X_INNER_WINDOW_H

#include <View.h>
#include <Window.h>
#include <map>

extern "C" {
#include <X11/Xlib.h>
}
#include "Event.h"

class XWindow;
class XWindowFrame;
class WinHandle;
class BBitmap;
class XFrameView;
class XPixmap;

class Windows {
private:
	static std::map<Window, WinHandle*> windows_;
	static Window max_;
public:
	static void add(WinHandle* window);
	static bool is_bewindow(Window id);
	static XWindowFrame* get_bewindow(Window id);
	static XWindow* get_xwindow(Window id);
	static XPixmap* get_pixmap(Window id);
	static WinHandle* get(Window id);
	static Window last_id();
	static void flush();
	static void erase(Window id);
};

class WinHandle {
public:
	virtual XWindow* get_window() {return 0;}
	virtual ~WinHandle() {}
	virtual bool is_root() const {
		return false;
	}
};

class XWindow : public BView, public WinHandle {
private:
	rgb_color bg_color_;
	rgb_color border_color_;
	int border_width_;
	BBitmap* offscreen_;
	long event_mask_;
	::Window id_;
public:
	GC gc = NULL;
	GC default_gc = NULL;
public:
	XWindow(BRect rect, int border, rgb_color bg, rgb_color border_color);
	void lock();
	void unlock();
	virtual XWindow* get_window() {
		return this;
	}
	void draw_border();
	void bg_color(rgb_color bg_color);
	void contains(const BPoint &point, ::Window &win);
	void expose();
	BBitmap* offscreen() {
		return offscreen_;
	}
	void offscreen(BBitmap* offscreen) {
		offscreen_ = offscreen;
	}
	void border_width(int w) {
		border_width_ = w;
	}
	void border_color(rgb_color color) {
		border_color_ = color;
	}
	long event_mask() {
		return event_mask_;
	}
	void event_mask(long mask) {
		event_mask_ = mask;
	}
	void id(::Window id) {
		id_ = id;
	}
	::Window id() {
		return id_;
	}
};

class XPixmap : public XWindow {
public:
	XPixmap(BRect frame, unsigned int depth);
	virtual ~XPixmap();
};

class XFrameView : public BView, public WinHandle {
private:
	BBitmap* offscreen_;
	XWindow* root_;
	rgb_color bg_;
public:
	XFrameView(BRect rect, rgb_color bg);
	virtual ~XFrameView();
	void ready();

	XWindow* root() {
		return root_;
	}
	BBitmap* offscreen() {
		return offscreen_;
	}

protected:
	virtual void Draw(BRect update) override;
	virtual void MouseDown(BPoint point) override;
	virtual void MouseMoved(BPoint where, uint32 code,
		const BMessage* dragMessage) override;
	virtual void Pulse() override;

	void _MouseEvent(int type, BPoint point);
};

class XWindowFrame : public BWindow, public WinHandle {
private:
	XFrameView* view_;
public:
	XWindowFrame(BRect rect, rgb_color bg_color);
	virtual void WindowActivated(bool active);
	virtual XWindow* get_window() {
		return view_->root();
	}
	virtual bool is_root() const {
		return true;
	}
	void update();
};

#endif
