#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "XInnerWindow.h"
#include "Color.h"
#include "Event.h"
#include <map>

extern "C" Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int w, unsigned int h, unsigned int brd, unsigned long brd_col, unsigned long bg) {
	if(parent == 0) {
		if (x==0)
			x+=40;
		if (y==0)
			y+=40;
		BRect rect(x, y, x + w, y + h);
		Windows::add(new XWindowFrame(rect, create_rgb(bg)));
	} else {
		BRect rect(x, y, x + w + brd*2, y + h + brd *2);
		WinHandle* parent_window = Windows::get(parent);
		XWindow* window = new XWindow(rect, brd, create_rgb(brd_col), create_rgb(bg));
		parent_window->get_window()->AddChild(window);
		window->offscreen(parent_window->get_window()->offscreen());
		window->draw_border();
		Windows::add(window);
	}
	return Windows::last_id();
}

extern "C" int XDestroyWindow(Display *display, Window w) {
	return 0;
}

extern "C" int XChangeWindowAttributes(Display *display, Window w, unsigned long vmask, XSetWindowAttributes *attr) {
	return 0;
}

extern "C" int XMapWindow(Display *display, Window window) {
	WinHandle* handle = Windows::get(window);
	if(handle->is_root()) {
		dynamic_cast<XWindowFrame*>(handle)->Show();
	} else {
		dynamic_cast<XWindow*>(handle)->Show();
	}
	return 0;
}

extern "C" int XUnmapWindow(Display *display, Window window) {
	WinHandle* handle = Windows::get(window);
	if(handle->is_root()) {
		dynamic_cast<XWindowFrame*>(handle)->Hide();
	} else {
		dynamic_cast<XWindow*>(handle)->Hide();
	}
	return 0;
}

extern "C" int XMapSubwindows(Display *display, Window window) {
	XMapWindow(display, window);
	return 0;
}

extern "C" int XMapRaised(Display* display, Window w) {
	if(w == 0)
		return 0;
	if(Windows::is_bewindow(w)) {
		XWindowFrame* window = Windows::get_bewindow(w);
		window->Show();
		window->Activate();
	} else
		Windows::get_xwindow(w)->Show();
	return 0;
}

extern "C" int XStoreName(Display *display, Window w, const char *wname) {
	if(w == 0)
		return 0;
	if(Windows::is_bewindow(w))
		Windows::get_bewindow(w)->SetTitle(wname);
	Windows::get_xwindow(w)->SetName(wname);
	return 0;
}

extern "C" int XFlush(Display *display) {
	Windows::flush();
	return 0;
}

extern "C" int XSetWindowBackground(Display *display, Window w, unsigned long bg) {
	XWindow* window = Windows::get_xwindow(w);
	window->bg_color(create_rgb(bg));
	return 0;
}

extern "C" int XSetWindowBorder(Display* display, Window w, unsigned long border_pixel) {
	if(w == 0)
		return 0;
	if(!Windows::is_bewindow(w)) {
		XWindow* window = Windows::get_xwindow(w);
		window->border_color(create_rgb(border_pixel));
		window->draw_border();
	}
	return 0;
}

extern "C" int XClearWindow(Display *display, Window w) {
	XWindow* window = Windows::get_xwindow(w);
	window->draw_border();
	return 0;
}

extern "C" int XSetStandardProperties(Display* display, Window w, const char* window_name, const char* icon_name, Pixmap icon_pixmap, char** argv, int argc, XSizeHints* hints) {
	XStoreName(display, w, window_name);
}
