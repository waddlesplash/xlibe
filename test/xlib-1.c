#include <X11/Xlib.h>
#include <X11/Xutil.h>

int main( int argc , char *argv[] ){
    Display *display;
    Window window;
    GC     gc;
    char title[]      = "This is TITLE";
    char message[]    = "Hello New World!";
    char icon_title[] = "ICON!";
    unsigned long background;
    unsigned long foreground;

    display = XOpenDisplay(NULL);

    background = WhitePixel(display,0);
    foreground = BlackPixel(display,0);

    window = XCreateSimpleWindow(display,
                                 DefaultRootWindow(display),
                                 0,0,200,100,
                                 0,0,background);

    XSetStandardProperties(display,window,title,icon_title,
			   None,argv,argc,NULL);

    gc = XCreateGC(display,window,0,0);

    XSetBackground(display,gc,background);
    XSetForeground(display,gc,foreground);

    XMapRaised(display,window);

    XSelectInput(display,window,ExposureMask);

    while (1){
	XEvent event;
        XNextEvent(display,&event);
        switch ( event.type ){
          case Expose:
	    XDrawImageString(display,window,gc,20,50,message,strlen(message));
            break;
        }
    }
}
