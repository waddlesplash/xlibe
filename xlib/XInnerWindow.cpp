#include "XInnerWindow.h"
#include "Color.h"
#include <Bitmap.h>
#include <Message.h>
#include <iostream>
#include "Event.h"

std::map<Window, WinHandle*> Windows::windows_;
Window Windows::max_ = 0;

void Windows::add(WinHandle* window) {
  windows_[++max_] = window;
  window->get_window()->id(max_);
}

WinHandle* Windows::get(Window id) {
  if(id == 0)
    return 0;
  return windows_[id];
}

Window Windows::last_id() {
  return max_;
}

bool Windows::is_bewindow(Window id) {
  if(windows_.find(id) == windows_.end())
    return false;
  if(dynamic_cast<XWindow*>(windows_[id]))
    return false;
  return true;
}

XWindowFrame* Windows::get_bewindow(Window id) {
  if(windows_.find(id) == windows_.end())
    return 0;
  return dynamic_cast<XWindowFrame*>(windows_[id]);
}

XWindow* Windows::get_xwindow(Window id) {
  if(windows_.find(id) == windows_.end())
    return 0;
  return windows_[id]->get_window();
}

XPixmap* Windows::get_pixmap(Window id) {
  if(windows_.find(id) == windows_.end())
    return 0;
  return dynamic_cast<XPixmap*>(windows_[id]);
}

void Windows::flush() {
  std::map<Window, WinHandle*>::iterator i;
  for(i=windows_.begin();i!=windows_.end();++i) {
    XWindowFrame* window = dynamic_cast<XWindowFrame*>((*i).second);
    if(window) {
      window->get_window()->Window()->Flush();
      window->update();
    }
  }
}

void Windows::erase(Window id) {
  WinHandle* handle = windows_[id];
  delete handle;
  windows_.erase(id);
}

XWindowFrame::XWindowFrame(BRect rect, rgb_color bg_color)
 : BWindow(rect, "*****", B_TITLED_WINDOW, 0, B_CURRENT_WORKSPACE) {
  BRect viewrect(0, 0, rect.Width(), rect.Height());
  view_ = new XFrameView(viewrect, bg_color);
  AddChild(view_);
}

void XWindowFrame::WindowActivated(bool active) {
  if(active) {
    view_->root()->expose();
  }
}

void XWindowFrame::update() {
  view_->LockLooper();
  view_->offscreen()->Lock();
  view_->DrawBitmap(view_->offscreen());
  view_->offscreen()->Unlock();
  view_->UnlockLooper();
}

XWindow::XWindow(BRect rect, int border, rgb_color border_color, rgb_color bg)
  : BView(rect, "", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW), bg_color_(bg), border_color_(border_color), border_width_(border), event_mask_(0), gc_(0) {
  SetLowColor(bg);  
}

void XWindow::draw_border() {
  lock();
  SetHighColor(bg_color_);
  if(border_width_ != 0) {
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
  unlock();
}

void XWindow::contains(const BPoint &point, unsigned long &win) {
  int i, max;
  lock();
  max = CountChildren();
  if(Frame().Contains(point))
    win = id();
  for(i=0;i!=max;++i) {
    dynamic_cast<XWindow*>(ChildAt(i))->contains(point, win);
  }
  unlock();
}

void XWindow::expose() {
  if(get_window()->event_mask() & ExposureMask) {
    int i, max;
    XEvent* event = new XEvent;
    event->type = Expose;
    event->xany.window = id();
    add_event(event);
    lock();
    max = CountChildren();
    for(i=0;i!=max;++i) {
      dynamic_cast<XWindow*>(ChildAt(i))->expose();
    }
    unlock();    
  }
}

void XWindow::bg_color(rgb_color bg_color) {
  bg_color_ = bg_color;
}

void XWindow::lock() {
  offscreen()->Lock();
}

void XWindow::unlock() {
  offscreen()->Unlock();
}

XPixmap::XPixmap(BRect frame, unsigned int depth) : XWindow(frame, 0, create_rgb(0), create_rgb(0)) {
  color_space space;
  if(depth <= 8)
    space = B_COLOR_8_BIT;
  if(depth <= 16)
    space = B_RGB_16_BIT;
  else
    space = B_RGB_32_BIT;
  offscreen(new BBitmap(frame, space, true));
}

XPixmap::~XPixmap() {
  delete offscreen();
}

XFrameView::XFrameView(BRect rect, rgb_color bg)
  : BView(rect, "", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW), bg_(bg) {
  root_ = new XWindow(rect, 0, create_rgb(0), bg);
  offscreen_ = new BBitmap(rect, B_RGB_32_BIT, true);
  offscreen_->AddChild(root_);
  offscreen_->Lock();
  root_->SetHighColor(bg_);
  root_->FillRect(rect);
  offscreen_->Unlock();
  root_->offscreen(offscreen_);
}

XFrameView::~XFrameView() {
}

void XFrameView::Draw(BRect updateRect) {
  DrawBitmap(offscreen_, updateRect, updateRect);
}

void XFrameView::MouseDown(BPoint point) {
  if(root()->event_mask() & ButtonPressMask) {
    int32 buttons = 0;
    Window()->CurrentMessage()->FindInt32("buttons", &buttons);
    XEvent* event = new XEvent;
    event->type = ButtonPress;
    event->xbutton.x = (int)point.x;
    event->xbutton.y = (int)point.y;
    event->xany.window = root_->id();
    switch(buttons) {
      case B_PRIMARY_MOUSE_BUTTON:
        event->xbutton.button = 1;
        break;
      case B_SECONDARY_MOUSE_BUTTON:
        event->xbutton.button = 3;
        break;
      case B_TERTIARY_MOUSE_BUTTON:
        event->xbutton.button = 2;
        break;
    }
    root_->contains(point, event->xany.window);
    add_event(event);
  }
}
