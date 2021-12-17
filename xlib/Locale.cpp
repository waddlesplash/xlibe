#include <support/String.h>
#include <kernel/OS.h>

#include "Atom.h"
#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

static inline void
unicode_to_utf8(uint32 c, char** out)
{
	char *s = *out;

	if (c < 0x80) {
		*(s++) = c;
	} else if (c < 0x800) {
		*(s++) = 0xc0 | (c >> 6);
		*(s++) = 0x80 | (c & 0x3f);
	} else if (c < 0x10000) {
		*(s++) = 0xe0 | (c >> 12);
		*(s++) = 0x80 | ((c >> 6) & 0x3f);
		*(s++) = 0x80 | (c & 0x3f);
	} else if (c <= 0x10ffff) {
		*(s++) = 0xf0 | (c >> 18);
		*(s++) = 0x80 | ((c >> 12) & 0x3f);
		*(s++) = 0x80 | ((c >> 6) & 0x3f);
		*(s++) = 0x80 | (c & 0x3f);
	}
	*out = s;
}

static inline BString
convert_to_utf8(const XChar2b* str, int strLen)
{
	BString ret;
	const int32 maxLength = strLen * 2;
	char* out = ret.LockBuffer(maxLength);
	char*const outStart = out;
	for (int i = 0; i < strLen; i++) {
		unicode_to_utf8(str[i].byte2 | (str[i].byte1 << 8), &out);

		if ((maxLength - (out - outStart)) <= 1)
			break;
	}
	ret.UnlockBuffer(out - outStart);
	return ret;
}

// #pragma mark - general

extern "C" Bool
XSupportsLocale()
{
	// TODO?
	return True;
}

BString
_x_text_decode(const XTextProperty* prop)
{
	if (prop->nitems == 0)
		return BString();
	if (prop->encoding == Atoms::UTF8_STRING)
		return BString((const char*)prop->value, prop->nitems);

	debugger("Unhandled encoding!");
	return BString();
}

// #pragma mark - fonts

extern "C" int
XTextWidth16(XFontStruct* font_struct, const XChar2b* str, int nchars)
{
	const BString string = convert_to_utf8(str, nchars);
	return XTextWidth(font_struct, string.String(), string.Length());
}

extern "C" int
XTextExtents16(XFontStruct* font_struct, const XChar2b* str, int nchars,
	int* direction_return, int* font_ascent_return, int* font_descent_return, XCharStruct* overall_return)
{
	const BString string = convert_to_utf8(str, nchars);
	return XTextExtents(font_struct, string.String(), string.Length(),
		direction_return, font_ascent_return, font_descent_return, overall_return);
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
	UNIMPLEMENTED();
	return BadImplementation;
}

// #pragma mark - drawing

extern "C" int
XDrawString16(Display* display, Drawable w, GC gc, int x, int y, const XChar2b* str, int nchars)
{
	const BString string = convert_to_utf8(str, nchars);
	return XDrawString(display, w, gc, x, y, string.String(), string.Length());
}

extern "C" int
XDrawImageString16(Display* display, Drawable w, GC gc, int x, int y, const XChar2b* str, int nchars)
{
	return XDrawString16(display, w, gc, x, y, str, nchars);
}

extern "C" int
XDrawText16(Display *display, Drawable w, GC gc, int x, int y, XTextItem16* items, int count)
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
	XmbDrawString(display, w, font_set, gc, x, y, str, len);
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
