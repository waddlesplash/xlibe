#include <X11/Xlib.h>
#include <interface/Bitmap.h>
#include "XInnerWindow.h"

extern "C" Pixmap XCreatePixmap(Display* display, Drawable d, unsigned int width, unsigned int height, unsigned int	depth) {
  BRect rect(0, 0, width - 1, height - 1);
  XPixmap* pixmap = new XPixmap(rect, depth);
  Windows::add(pixmap);
  pixmap->lock();
  pixmap->StrokeLine(BPoint(0,0), BPoint(width-1, height-1));
  pixmap->unlock();
  return Windows::last_id();
}

extern "C" int XCopyArea(Display* display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y) {
  XWindow* src_window = Windows::get_xwindow(src);
  BBitmap* src_image = src_window->offscreen();
  XWindow* dest_window = Windows::get_xwindow(dest);
  BRect src_rect(src_x, src_y, src_x + width - 1, src_y + height - 1);
  BRect dest_rect(dest_x, dest_y, dest_x + width - 1, dest_y + height - 1);

  dest_window->lock();
  src_image->Lock();
  src_window->Flush();
  dest_window->DrawBitmap(src_image, src_rect, dest_rect);
  src_image->Unlock();
  dest_window->unlock();
  
  return 0;
}
