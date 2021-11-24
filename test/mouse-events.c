/*
 * events.c - demonstrate handling of X events using an events loop.
 */

#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>		/* getenv(), etc. */

/*
 * function: create_simple_window. Creates a window with a white background
 *           in the given size.
 * input:    display, size of the window (in pixels), and location of the window
 *           (in pixels).
 * output:   the window's ID.
 * notes:    window is created with a black border, 2 pixels wide.
 *           the window is automatically mapped after its creation.
 */
Window
create_simple_window(Display* display, int width, int height, int x, int y)
{
  int screen_num = DefaultScreen(display);
  int win_border_width = 2;
  Window win;

  /* create a simple window, as a direct child of the screen's */
  /* root window. Use the screen's black and white colors as   */
  /* the foreground and background colors of the window,       */
  /* respectively. Place the new window's top-left corner at   */
  /* the given 'x,y' coordinates.                              */
  win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                            x, y, width, height, win_border_width,
                            BlackPixel(display, screen_num),
                            WhitePixel(display, screen_num));

  /* make the window actually appear on the screen. */
  XMapWindow(display, win);

  /* flush all pending requests to the X server. */
  XFlush(display);

  return win;
}

GC
create_gc(Display* display, Window win, int reverse_video)
{
  GC gc;				/* handle of newly created GC.  */
  unsigned long valuemask = 0;		/* which values in 'values' to  */
					/* check when creating the GC.  */
  XGCValues values;			/* initial values for the GC.   */
  unsigned int line_width = 2;		/* line width for the GC.       */
  int line_style = LineSolid;		/* style for lines drawing and  */
  int cap_style = CapButt;		/* style of the line's edje and */
  int join_style = JoinBevel;		/*  joined lines.		*/
  int screen_num = DefaultScreen(display);

  gc = XCreateGC(display, win, valuemask, &values);
  if (gc < 0) {
	fprintf(stderr, "XCreateGC: \n");
  }

  /* allocate foreground and background colors for this GC. */
  if (reverse_video) {
    XSetForeground(display, gc, WhitePixel(display, screen_num));
    XSetBackground(display, gc, BlackPixel(display, screen_num));
  }
  else {
    XSetForeground(display, gc, BlackPixel(display, screen_num));
    XSetBackground(display, gc, WhitePixel(display, screen_num));
  }

  /* define the style of lines that will be drawn using this GC. */
  XSetLineAttributes(display, gc,
                     line_width, line_style, cap_style, join_style);

  /* define the fill style for the GC. to be 'solid filling'. */
  XSetFillStyle(display, gc, FillSolid);

  return gc;
}

/*
 * function: handle_expose. handles an Expose event by redrawing the window.
 * input:    display, 2 GCs, XExposeEvent event structure, dimensions of
 *           the window, pixels array.
 * output:   none.
 */
void
handle_expose(Display* display, GC gc, GC rev_gc, XExposeEvent* expose_event,
              unsigned int win_width, unsigned int win_height,
	      short pixels[1000][1000])
{
  /* if this is the first in a set of expose events - ignore this event. */
  if (expose_event->count != 0)
    return;
  /* draw the contents of our window. */

  /* draw one pixel near each corner of the window */
  XDrawPoint(display, expose_event->window, gc, 5, 5);
  XDrawPoint(display, expose_event->window, gc, 5, win_height-5);
  XDrawPoint(display, expose_event->window, gc, win_width-5, 5);
  XDrawPoint(display, expose_event->window, gc, win_width-5, win_height-5);

  /* draw two intersecting lines, one horizontal and one vertical, */
  /* which intersect at point "50,100".                            */
  XDrawLine(display, expose_event->window, gc, 50, 0, 50, 200);
  XDrawLine(display, expose_event->window, gc, 0, 100, 200, 100);

  /* now use the XDrawArc() function to draw a circle whose diameter */
  /* is 30 pixels, and whose center is at location '50,100'.         */
  XDrawArc(display, expose_event->window, gc,
           50-(30/2), 100-(30/2), 30, 30, 0, 360*64);

  {
    XPoint points[] = {
      {0, 0},
      {15, 15},
      {0, 15},
      {0, 0}
    };
    int npoints = sizeof(points)/sizeof(XPoint);

    /* draw a small triangle at the top-left corner of the window. */
    /* the triangle is made of a set of consecutive lines, whose   */
    /* end-point pixels are specified in the 'points' array.       */
    XDrawLines(display, expose_event->window, gc,
               points, npoints, CoordModeOrigin);
  }

  /* draw a rectangle whose top-left corner is at '120,150', its width is */
  /* 50 pixels, and height is 60 pixels.                                  */
  XDrawRectangle(display, expose_event->window, gc, 120, 150, 50, 60);

  /* draw a filled rectangle of the same size as above, to the left of the */
  /* previous rectangle.                                                   */
  XFillRectangle(display, expose_event->window, gc, 60, 150, 50, 60);

  /* finally, draw all the pixels in the 'pixels' array. */
  {
    int x, y;
    for (x=0; x<win_width; x++)
      for (y=0; y<win_height; y++)
	switch(pixels[x][y]) {
	  case 1: /* draw point. */
            XDrawPoint(display, expose_event->window, gc, x, y);
	    break;
	  case -1: /* erase point. */
            XDrawPoint(display, expose_event->window, rev_gc, x, y);
	    break;
        }
  }
}

/*
 * function: handle_drag. handles a Mouse drag event - if the left button
 *           is depressed - draws the pixel below the mouse pointer. if the
 *           middle button is depressed - erases the pixel below the mouse
 *           pointer.
 * input:    display, 2 GCs, XButtonEvent event structure, dimensions of
 *           the window, pixels array.
 * output:   none.
 */
void
handle_drag(Display* display, GC gc, GC rev_gc, XButtonEvent* drag_event,
            unsigned int win_width, unsigned int win_height,
            short pixels[1000][1000])
{
  int x, y;

  /* invert the pixel under the mouse. */
  x = drag_event->x;
  y = drag_event->y;
  switch (drag_event->state) {
    case Button1Mask: /* draw the given pixel in black color. */
      XDrawPoint(display, drag_event->window, gc, x, y);
      pixels[x][y] = 1;
      break;
    case Button2Mask: /* draw the given pixel in white color. */
      XDrawPoint(display, drag_event->window, rev_gc, x, y);
      pixels[x][y] = -1;
      break;
  }
}

/*
 * function: handle_button_down. handles a Mouse press event - if the left
 *           button is depressed - draws the pixel below the mouse pointer.
 *           if the middle button is depressed - erases the pixel below the
 *           mouse pointer.
 * input:    display, 2 GCs, XButtonEvent event structure, dimensions of
 *           the window, pixels array.
 * output:   none.
 */
void
handle_button_down(Display* display, GC gc, GC rev_gc,
                   XButtonEvent* button_event,
                   unsigned int win_width, unsigned int win_height,
                   short pixels[1000][1000])
{
  int x, y;

  /* invert the pixel under the mouse. */
  x = button_event->x;
  y = button_event->y;
  switch (button_event->button) {
    case Button1: /* draw the given pixel in black color. */
      XDrawPoint(display, button_event->window, gc, x, y);
      pixels[x][y] = 1;
      break;
    case Button2: /* draw the given pixel in white color. */
      XDrawPoint(display, button_event->window, rev_gc, x, y);
      pixels[x][y] = -1;
      break;
  }
}

int main(int argc, char* argv[])
{
  Display* display;		/* pointer to X Display structure.           */
  int screen_num;		/* number of screen to place the window on.  */
  Window win;			/* pointer to the newly created window.      */
  unsigned int display_width,
               display_height;	/* height and width of the X display.        */
  unsigned int width, height;	/* height and width for the new window.      */
  char *display_name = getenv("DISPLAY");  /* address of the X display.      */
  GC gc, rev_gc;		/* GC (graphics context) used for drawing    */
				/*  in our window.			     */
  short pixels[1000][1000];	/* used to store pixels on screen that were  */
				/* explicitly drawn or erased by the user.   */

  /* initialize the 'pixels' array to contain 0 values. */
  {
    int x, y;
    for (x=0; x<1000; x++)
      for (y=0; y<1000; y++)
        pixels[x][y] = 0;
  }

  /* open connection with the X server. */
  display = XOpenDisplay(display_name);
  if (display == NULL) {
    fprintf(stderr, "%s: cannot connect to X server '%s'\n",
            argv[0], display_name);
    exit(1);
  }

  /* get the geometry of the default screen for our display. */
  screen_num = DefaultScreen(display);
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);

  /* make the new window occupy 1/9 of the screen's size. */
  width = (display_width / 3);
  height = (display_height / 3);
  printf("window width - '%d'; height - '%d'\n", width, height);

  /* create a simple window, as a direct child of the screen's   */
  /* root window. Use the screen's white color as the background */
  /* color of the window. Place the new window's top-left corner */
  /* at the given 'x,y' coordinates.                             */
  win = create_simple_window(display, width, height, 0, 0);

  /* allocate two new GCs (graphics contexts) for drawing in the window. */
  /* the first is used for drawing black over white, the second is used  */
  /* for drawing white over black.                                       */
  gc = create_gc(display, win, 0);
  rev_gc = create_gc(display, win, 1);

  /* subscribe to the given set of event types. */
  XSelectInput(display, win, ExposureMask | KeyPressMask |
                     ButtonPressMask | Button1MotionMask |
			 Button2MotionMask | StructureNotifyMask);

  /* perform an events loop */
  {
    int done = 0;
    XEvent an_event;
    while (!done) {
      XNextEvent(display, &an_event);
      switch (an_event.type) {
        case Expose:
          /* redraw our window. */
	  handle_expose(display, gc, rev_gc, (XExposeEvent*)&an_event.xexpose,
             		width, height, pixels);
          break;
  
        case ConfigureNotify:
          /* update the size of our window, for expose events. */
          width = an_event.xconfigure.width;
          height = an_event.xconfigure.height;
          break;
  
        case ButtonPress:
          /* invert the pixel under the mouse pointer. */
          handle_button_down(display, gc, rev_gc,
                             (XButtonEvent*)&an_event.xbutton,
                             width, height, pixels);
          break;
  
        case MotionNotify:
          /* invert the pixel under the mouse pointer. */
          handle_drag(display, gc, rev_gc,
                      (XButtonEvent*)&an_event.xbutton,
                      width, height, pixels);
	  break;
  
        case KeyPress:
          /* exit the application by braking out of the events loop. */
          done = 1;
          break;
  
        default: /* ignore any other event types. */
          break;
      } /* end switch on event type */
    } /* end while events handling */
  }

  /* free the GCs. */
  XFreeGC(display, gc);
  XFreeGC(display, rev_gc);

  /* close the connection to the X server. */
  XCloseDisplay(display);
  return(0);
}
