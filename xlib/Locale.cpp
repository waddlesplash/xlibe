/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <support/String.h>
#include <kernel/OS.h>
#include <langinfo.h>
#include <iconv.h>

#include "Atom.h"
#include "Property.h"
#include "Debug.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

static inline BString
convert_to_utf8(const char* encoding, const void* str, int strBytesLen)
{
	if (encoding == NULL)
		encoding = nl_langinfo(CODESET);
	if (strBytesLen == -1)
		strBytesLen = strlen((const char*)str);
	if (strcasecmp(encoding, "UTF-8") == 0) {
		// Well, this is easy.
		return BString((const char*)str, strBytesLen);
	}

	BString ret;

	iconv_t cd = iconv_open("UTF-8", encoding);
	if (cd == (iconv_t)-1)
		return BString();

	char* out = ret.LockBuffer(strBytesLen);
	char* outStart = out;
	size_t remainingIn = strBytesLen, remainingOut = strBytesLen;
	while (remainingIn) {
		status_t result = iconv(cd, (char**)&str, &remainingIn, (char**)&out, &remainingOut);
		if (result < 0)
			result = errno;

		if (result == E2BIG) {
			const size_t length = out - outStart;
			ret.UnlockBuffer(length);
			remainingOut = (length < 64) ? 128 : length * 1.5;
			outStart = ret.LockBuffer(remainingOut);
			out = outStart + length;
			remainingOut -= length;
		}
	}
	iconv_close(cd);
	ret.UnlockBuffer(out - outStart);
	return ret;
}

static inline BString
convert_to_utf8(const XChar2b* str, int nchars)
{
	static_assert(sizeof(XChar2b) == sizeof(uint16));
	return convert_to_utf8("UCS-2", str, nchars * 2);
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
	if (!prop || prop->nitems == 0)
		return BString();
	if (prop->encoding == Atoms::UTF8_STRING)
		return BString((const char*)prop->value, prop->nitems);
	if (prop->encoding == XA_STRING)
		return convert_to_utf8(NULL, prop->value, prop->nitems);

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
	// We always return fonts with Unicode encoding.
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
	// We always return fonts with Unicode encoding.
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
	XTextProperty name_prop = make_text_property(XA_STRING, 8, window_name);
	XTextProperty icon_name_prop = make_text_property(XA_STRING, 8, icon_name);
	XSetWMProperties(display, w, window_name ? &name_prop : NULL, icon_name ? &icon_name_prop : NULL,
		argv, argc, normal_hints, wm_hints, class_hints);
}

// #pragma mark - properties

extern "C" int
Xutf8TextListToTextProperty(Display* display, char** list, int count, XICCEncodingStyle style,
	XTextProperty* text_prop_return)
{
	int status = XStringListToTextProperty(list, count, text_prop_return);
	if (status != 0)
		return status;

	// We always want things in UTF-8, no matter what "style" is specified.
	text_prop_return->encoding = Atoms::UTF8_STRING;
	return 0;
}

extern "C" int
XmbTextListToTextProperty(Display* display, char** list, int count, XICCEncodingStyle style,
	XTextProperty* text_prop_return)
{
	// Convert everything to UTF-8 first.
	BString utf8list[count];
	const char* strings[count];
	for (int i = 0; i < count; i++) {
		utf8list[i] = convert_to_utf8(NULL, list[i], -1);
		strings[i] = utf8list[i].String();
	}

	return Xutf8TextListToTextProperty(display, (char**)strings, count, style, text_prop_return);
}

extern "C" int
XwcTextListToTextProperty(Display* display, wchar_t** list, int count,
	XICCEncodingStyle style, XTextProperty* text_prop_return)
{
	UNIMPLEMENTED();
	text_prop_return->value = NULL;
	text_prop_return->nitems = 0;
	return BadAlloc;
}

extern "C" Status
Xutf8TextPropertyToTextList(Display* display, const XTextProperty* tp,
	char*** list_return, int* count_return)
{
	return XTextPropertyToStringList((XTextProperty*)tp, list_return, count_return);
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
XwcFreeStringList(wchar_t** list)
{
	UNIMPLEMENTED();
}
