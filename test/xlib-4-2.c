#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

int main( int argc , char *argv[] ){
    Display *display;
    Window window;
    GC     gc,gc_clr;
    XEvent event;
    Pixmap pixmap;
    char title[]      = "This is TITLE";
    char icon_title[] = "ICON!";
    unsigned long background;
    unsigned long foreground;
    int oldx=0,x=0;
    int dx=1;
    int arcsize=100;
    int window_width=300;
    int window_height=200;
    

    display = XOpenDisplay(NULL);

    background = WhitePixel(display,0);
    foreground = BlackPixel(display,0);

    window = XCreateSimpleWindow(display,
                                 DefaultRootWindow(display),
                                 0,0,window_width,window_height,
                                 0,0,background);
    pixmap = XCreatePixmap(display,window,window_width,window_height,
			   DefaultDepth(display,0));

    XSetStandardProperties(display,window,title,icon_title,
			   None,argv,argc,NULL);

    gc = XCreateGC(display,window,0,0);
    XSetBackground(display,gc,background);
    XSetForeground(display,gc,foreground);

    gc_clr = XCreateGC(display,window,0,0);
    XSetBackground(display,gc_clr,background);
    XSetForeground(display,gc_clr,background);

    XSelectInput(display,window,ExposureMask);
    XMapRaised(display,window);
    XMaskEvent(display,ExposureMask,&event);

    while (1){
	XFillRectangle(display,pixmap,gc_clr,0,0,window_width,window_height);
	XFillArc(display,pixmap,gc,x,50,arcsize,arcsize,0,360*64);
	XCopyArea(display,pixmap,window,gc,0,0,window_width,window_height,0,0);

	oldx = x;
	x += dx;

	if ( x==0 ){
	    dx=1;
	} else if ( x==(window_width-arcsize) ){
	    dx=-1;
	}
    }
}
