#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

// #pragma mark - fonts & text

extern "C" int
XTextWidth16(XFontStruct *font_struct, const XChar2b *string, int count)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XTextExtents16(XFontStruct* font_struct, const XChar2b* string, int nchars,
	int* direction_return, int* font_ascent_return, int* font_descent_return, XCharStruct* overall_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
Xutf8TextExtents(XFontSet font_set, const char* string, int num_bytes,
	XRectangle* overall_ink_return, XRectangle* overall_logical_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XmbTextExtents(XFontSet font_set, const char* string, int num_bytes,
	XRectangle* overall_ink_return, XRectangle* overall_logical_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XwcTextExtents(XFontSet font_set, const wchar_t* string, int num_bytes,
	XRectangle* overall_ink_return, XRectangle* overall_logical_return)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XmbTextEscapement(XFontSet font_set, const char* string, int num_bytes)
{
	return Xutf8TextEscapement(font_set, string, num_bytes);
}

extern "C" int
XwcTextEscapement(XFontSet font_set, const wchar_t* string, int num_wchars)
{
	// FIXME: wchar_t!
	return Xutf8TextEscapement(font_set, (const char*)string, num_wchars);
}

extern "C" int
XDrawText16(Display *display, Drawable w, GC gc, int x, int y, XTextItem16* items, int count)
{
	// TODO?
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XDrawString16(Display* display, Drawable w, GC gc, int x, int y, const XChar2b* str, int len)
{
	// TODO?
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" int
XDrawImageString16(Display* display, Drawable w, GC gc, int x, int y, const XChar2b* str, int len)
{
	// TODO?
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" void
XwcDrawString(Display* display, Drawable w, XFontSet font_set,
	GC gc, int x, int y, const wchar_t* str, int len)
{
	// TODO?
	UNIMPLEMENTED();
}

extern "C" void
XwcDrawImageString(Display* display, Drawable w, XFontSet font_set,
	GC gc, int x, int y, const wchar_t* str, int len)
{
	XwcDrawString(display, w, font_set, gc, x, y, str, len);
}

extern "C" void
XmbDrawString(Display *display, Drawable w, XFontSet font_set,
	GC gc, int x, int y, const char* str, int len)
{
	Xutf8DrawString(display, w, font_set, gc, x, y, str, len);
}

extern "C" void
XmbDrawImageString(Display *display, Drawable w, XFontSet font_set,
	GC gc, int x, int y, const char* str, int len)
{
	Xutf8DrawString(display, w, font_set, gc, x, y, str, len);
}

// #pragma mark - input

extern "C" int
XmbLookupString(XIC ic, XKeyPressedEvent* event,
	char* buffer_return, int bytes_buffer, KeySym* keysym_return, Status* status_return)
{
	return BadImplementation;
}

extern "C" int
XwcLookupString(XIC ic, XKeyPressedEvent* event,
	wchar_t* buffer_return, int bytes_buffer, KeySym* keysym_return, Status* status_return)
{
	return BadImplementation;
}

// #pragma mark - windows

extern "C" void
XmbSetWMProperties(Display* display, Window w,
	const char* window_name, const char* icon_name, char** argv, int argc,
	XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints)
{
	XSetStandardProperties(display, w, window_name, icon_name, None, argv, argc, normal_hints);
	XSetWMHints(display, w, wm_hints);
	XSetClassHint(display, w, class_hints);
}

// #pragma mark - properties

extern "C" int
Xutf8TextListToTextProperty(Display* display, char** list, int count, XICCEncodingStyle style,
	XTextProperty* text_prop_return)
{
	UNIMPLEMENTED();
	text_prop_return->value = NULL;
	return BadAlloc;
}

extern "C" int
XmbTextListToTextProperty(Display* display, char** list, int count, XICCEncodingStyle style,
	XTextProperty* text_prop_return)
{
	UNIMPLEMENTED();
	text_prop_return->value = NULL;
	return BadAlloc;
}

extern "C" int
XwcTextListToTextProperty(Display* display, wchar_t** list, int count,
	XICCEncodingStyle style, XTextProperty* text_prop_return)
{
	UNIMPLEMENTED();
	text_prop_return->value = NULL;
	return BadAlloc;
}

extern "C" Status
XmbTextPropertyToTextList(Display* display, const XTextProperty* text_prop,
	char*** list_return, int* count_return)
{
	UNIMPLEMENTED();
	return BadAlloc;
}

extern "C" Status
XwcTextPropertyToTextList(Display* display, const XTextProperty* text_prop,
	wchar_t*** list_return, int* count_return)
{
	UNIMPLEMENTED();
	return BadAlloc;
}

extern "C" void
XwcFreeStringList(wchar_t **list)
{
	UNIMPLEMENTED();
}
