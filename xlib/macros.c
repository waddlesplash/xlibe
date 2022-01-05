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

long
XMaxRequestSize(Display* dpy)
{
	return dpy->max_request_size;
}

long
XExtendedMaxRequestSize(Display* dpy)
{
	return dpy->bigreq_size;
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

char*
XServerVendor(Display* dpy)
{
	return ServerVendor(dpy);
}

int
XVendorRelease(Display* dpy)
{
	return VendorRelease(dpy);
}

int
XDisplayKeycodes(Display* dpy, int* min_keycodes_return, int* max_keycodes_return)
{
	if (min_keycodes_return)
		*min_keycodes_return = dpy->min_keycode;
	if (max_keycodes_return)
		*max_keycodes_return = dpy->max_keycode;
	return 0;
}

void
XLockDisplay(Display* dpy)
{
	LockDisplay(dpy);
}

void
XUnlockDisplay(Display* dpy)
{
	UnlockDisplay(dpy);
}

int
XNoOp(Display *display)
{
	return 0;
}
