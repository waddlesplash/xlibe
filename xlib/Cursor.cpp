/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <app/Cursor.h>
#include <interface/Bitmap.h>

#include "Color.h"
#include "Drawing.h"
#include "Drawables.h"
#include "Debug.h"
#include "Image.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
}

extern "C" Cursor
XCreateFontCursor(Display *display, unsigned int xshape)
{
	BCursorID shape;
	switch (xshape) {
	case XC_xterm:
		shape = B_CURSOR_ID_I_BEAM;
		break;
	case XC_watch:
		shape = B_CURSOR_ID_PROGRESS;
		break;
	case XC_cross:
	case XC_cross_reverse:
	case XC_tcross:
	case XC_crosshair:
	case XC_diamond_cross:
	case XC_circle:
	case XC_dot:
	case XC_dotbox:
	case XC_draped_box:
		shape = B_CURSOR_ID_CROSS_HAIR;
		break;
	case XC_hand1:
	case XC_hand2:
		shape = B_CURSOR_ID_FOLLOW_LINK;
		break;
	case XC_question_arrow:
		shape = B_CURSOR_ID_HELP;
		break;
	case XC_sb_left_arrow:
		shape = B_CURSOR_ID_RESIZE_EAST;
		break;
	case XC_right_ptr:
	case XC_sb_right_arrow:
		shape = B_CURSOR_ID_RESIZE_WEST;
		break;
	case XC_sb_h_double_arrow:
		shape = B_CURSOR_ID_RESIZE_EAST_WEST;
		break;
	case XC_sb_up_arrow:
		shape = B_CURSOR_ID_RESIZE_NORTH;
		break;
	case XC_sb_down_arrow:
		shape = B_CURSOR_ID_RESIZE_SOUTH;
		break;
	case XC_sb_v_double_arrow:
	case XC_double_arrow:
		shape = B_CURSOR_ID_RESIZE_NORTH_SOUTH;
		break;
	case XC_pirate:
		shape = B_CURSOR_ID_NOT_ALLOWED;
		break;
	case XC_bottom_right_corner:
	case XC_top_left_corner:
		shape = B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
		break;
	case XC_fleur:
		shape = B_CURSOR_ID_MOVE;
		break;
	case XC_bottom_left_corner:
	case XC_top_right_corner:
		shape = B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
		break;
	case XC_left_ptr:
	default:
		shape = B_CURSOR_ID_SYSTEM_DEFAULT;
		break;
	}
	return (Cursor)new BCursor(shape);
}

extern "C" Cursor
XCreateGlyphCursor(Display *display, Font source_font, Font mask_font,
	unsigned int source_char, unsigned int mask_char,
	XColor const *foreground_color, XColor const *background_color)
{
	// TODO: other options?
	return XCreateFontCursor(display, source_char);
}

extern "C" Cursor
XCreatePixmapCursor(Display* display, Pixmap source, Pixmap mask,
	XColor* foreground_color, XColor* background_color,
	unsigned int x, unsigned int y)
{
	XPixmap* src = Drawables::get_pixmap(source);
	if (!src)
		return None;
	src->sync();

	// Use XImages to perform the conversion and masking.
	const XRectangle rect = xrect_from_brect(src->offscreen()->Bounds());
	XImage* srcImg = XGetImage(display, source, 0, 0, rect.width, rect.height, -1, ZPixmap),
		*maskImg = XGetImage(display, mask, 0, 0, rect.width, rect.height, -1, ZPixmap),
		*resultImg = XCreateImage(display, NULL, 32, ZPixmap, 0, NULL, rect.width, rect.height, 32, 0);
	BBitmap* resultBitmap = _bbitmap_for_ximage(resultImg);
	if (!srcImg || !maskImg || !resultImg || !resultBitmap) {
		if (srcImg)
			XDestroyImage(srcImg);
		if (maskImg)
			XDestroyImage(maskImg);
		if (resultImg)
			XDestroyImage(resultImg);
		return None;
	}
	resultImg->data = (char*)resultBitmap->Bits();

	unsigned long fg = foreground_color->pixel | (0xFF << 24),
		bg = background_color->pixel | (0xFF << 24),
		transparent = _x_rgb_to_pixel(make_color(0, 0, 0, 0));
	for (int iy = 0; iy < rect.height; iy++) {
		for (int ix = 0; ix < rect.width; ix++) {
			unsigned long pixel = XGetPixel(srcImg, ix, iy),
				mask = XGetPixel(maskImg, ix, iy);
			XPutPixel(resultImg, ix, iy, !mask ? transparent :
				(pixel ? fg : bg));
		}
	}

	BCursor* cursor = new BCursor(resultBitmap, BPoint(x, y));

	delete resultBitmap;
	resultImg->data = NULL;
	XDestroyImage(srcImg);
	XDestroyImage(maskImg);
	XDestroyImage(resultImg);
	return (Cursor)cursor;
}

extern "C" Status
XFreeCursor(Display* display, Cursor cursor)
{
	delete (BCursor*)cursor;
	return Success;
}

extern "C" Status
XRecolorCursor(Display* display, Cursor cursor, XColor* foreground_color, XColor* background_color)
{
	UNIMPLEMENTED();
	return BadImplementation;
}

extern "C" Status
XQueryBestCursor(Display* display, Drawable d, unsigned int width, unsigned int height,
	unsigned int* width_return, unsigned int* height_return)
{
	if (width_return)
		*width_return = width;
	if (height_return)
		*height_return = height;
	return Success;
}
