#include <X11/Xlib.h>
#include <X11/Xlibint.h>

int
XConnectionNumber(Display* display)
{
	return ConnectionNumber(display);
}

char*
XDisplayName(const char* string)
{
	return "Haiku";
}

char*
XDisplayString(Display* display)
{
	return DisplayString(display);
}

int
XDefaultScreen(Display* display)
{
	return DefaultScreen(display);
}

Display*
XDisplayOfScreen(Screen* screen)
{
	return DisplayOfScreen(screen);
}

int
XScreenCount(Display* display)
{
	return ScreenCount(display);
}

Screen*
XScreenOfDisplay(Display* display, int screen_number)
{
	return ScreenOfDisplay(display, screen_number);
}

Screen*
XDefaultScreenOfDisplay(Display* display)
{
	return DefaultScreenOfDisplay(display);
}

Colormap
XDefaultColormap(Display* display, int screen_number)
{
	return DefaultColormap(display, screen_number);
}

Colormap
XDefaultColormapOfScreen(Screen* screen)
{
	return DefaultColormapOfScreen(screen);
}

Visual*
XDefaultVisual(Display* display, int screen_number)
{
	return DefaultVisual(display, screen_number);
}

int
XDefaultDepth(Display* display, int screen_number)
{
	return DefaultDepth(display, screen_number);
}

unsigned long
XBlackPixelOfScreen(Screen* screen)
{
	return BlackPixelOfScreen(screen);
}

unsigned long
XWhitePixelOfScreen(Screen* screen)
{
	return WhitePixelOfScreen(screen);
}

unsigned long
XBlackPixel(Display* display, int screen_number)
{
	return BlackPixel(display, screen_number);
}

GC
XDefaultGC(Display* display, int screen_number)
{
	return DefaultGC(display, screen_number);
}

unsigned long
XWhitePixel(Display* display, int screen_number)
{
	return WhitePixel(display, screen_number);
}

Window
XDefaultRootWindow(Display *display)
{
	return DefaultRootWindow(display);
}

Window
XRootWindow(Display *display, int screen_number)
{
	return RootWindow(display, screen_number);
}

Window
XRootWindowOfScreen(Screen* screen)
{
	return RootWindowOfScreen(screen);
}

int
XDisplayWidth(Display *display, int screen_number)
{
	return DisplayWidth(display, screen_number);
}

int
XDisplayHeight(Display *display, int screen_number)
{
	return DisplayHeight(display, screen_number);
}

int
XWidthOfScreen(Screen* screen)
{
	return WidthOfScreen(screen);
}

int
XHeightOfScreen(Screen* screen)
{
	return HeightOfScreen(screen);
}

char*
XResourceManagerString(Display* dpy)
{
	return dpy->xdefaults;
}

unsigned long
XNextRequest(Display* dpy)
{
	return NextRequest(dpy);
}

unsigned long
XLastKnownRequestProcessed(Display* dpy)
{
	return LastKnownRequestProcessed(dpy);
}

int
XNoOp(Display *display)
{
	return 0;
}
