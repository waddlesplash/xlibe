/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <stdio.h>
#include <interface/Bitmap.h>

#include "Drawables.h"
#include "Drawing.h"
#include "Debug.h"
#include "Color.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/extensions/XShm.h>
}

extern "C" int
_XInitImageFuncPtrs(XImage *image)
{
	return 0;
}

static int
DestroyImage(XImage* image)
{
	BBitmap* bbitmap = (BBitmap*)image->obdata;
	if (image->data != bbitmap->Bits())
		free(image->data);
	delete bbitmap;
	delete image;
	return Success;
}

static inline uint8*
GetImageDataPointer(XImage* image, int x, int y)
{
	return (uint8*)&(image->data[
		(y * image->bytes_per_line) + ((x * image->bitmap_unit) / NBBY)]);
}

static unsigned long
ImageGetPixel(XImage* image, int x, int y)
{
	unsigned long pixel = 0;
	uint8* srcPtr = GetImageDataPointer(image, x, y);

	switch (image->bits_per_pixel) {
	case 1:
		pixel = ((*srcPtr) & (0x80 >> (x % 8))) ? 1 : 0;
		break;
	case 8:
		pixel = *srcPtr;
		break;
	case 15:
	case 16:
		pixel = *(uint16*)srcPtr;
		break;
	case 24:
		if (image->red_mask > image->blue_mask) {
			pixel = srcPtr[2] | (srcPtr[1] << 8) | (srcPtr[0] << 16);
		} else {
			pixel = srcPtr[0] | (srcPtr[1] << 8) | (srcPtr[2] << 16);
		}
		break;
	case 32:
		pixel = *(uint32*)srcPtr;
		break;
	}
	return pixel;
}

static int
ImagePutPixel(XImage* image, int x, int y, unsigned long pixel)
{
	uint8* destPtr = GetImageDataPointer(image, x, y);

	switch (image->bits_per_pixel) {
	case 1: {
		int mask = (0x80 >> (x % 8));
		if (pixel) {
			(*destPtr) |= mask;
		} else {
			(*destPtr) &= ~mask;
		}
		break;
	}
	case 8:
		*destPtr = pixel;
		break;
	case 15:
	case 16:
		*((uint16*)destPtr) = (uint16)(pixel & 0xFFFF);
		break;
	case 24:
		if (image->red_mask > image->blue_mask) {
			destPtr[0] = (pixel >> 16) & 0xFF;
			destPtr[1] = (pixel >> 8) & 0xFF;
			destPtr[2] = pixel & 0xFF;
		} else {
			destPtr[0] = pixel & 0xFF;
			destPtr[1] = (pixel >> 8) & 0xFF;
			destPtr[2] = (pixel >> 16) & 0xFF;
		}
		break;
	case 32:
		*((uint32*)destPtr) = pixel;
		break;
	}
	return 0;
}

extern "C" XImage*
XCreateImage(Display *display, Visual *visual,
	unsigned int depth, int format, int offset, char *data,
	unsigned int width, unsigned int height,
	int bitmap_pad, int bytes_per_line)
{
	if (format != ZPixmap)
		return NULL;

	XImage* image = new XImage;
	if (!image)
		return NULL;
	memset(image, 0, sizeof(XImage));

	image->height = height;
	image->width = width;
	image->depth = depth;
	image->xoffset = offset;
	image->format = format;
	image->data = data;
	image->bitmap_pad = bitmap_pad;

	if (depth == 8) {
		image->bits_per_pixel = image->bitmap_unit = 8;
	} else {
		if (!visual && depth >= 24)
			visual = display->screens[0].root_visual;
		image->bits_per_pixel = depth;
		image->bitmap_unit = ((depth+7)/8)*8;
	}
	image->bytes_per_line = bytes_per_line;

	image->byte_order = LSBFirst;
	image->bitmap_bit_order = LSBFirst;
	if (visual) {
		image->red_mask = visual->red_mask;
		image->green_mask = visual->green_mask;
		image->blue_mask = visual->blue_mask;
	}

	if (!XInitImage(image)) {
		delete image;
		return NULL;
	}

	return image;
}

extern "C" Status
XInitImage(XImage* image)
{
	if (image->bytes_per_line == 0) {
		image->bytes_per_line = image->width * (image->bitmap_unit / 8);
		image->bytes_per_line += image->bitmap_pad / 8;
	}

	memset(&image->f, 0, sizeof(image->f));
	image->f.destroy_image = DestroyImage;
	image->f.get_pixel = ImageGetPixel;
	image->f.put_pixel = ImagePutPixel;

	// Create the auxiliary bitmap.
	BBitmap* bbitmap = new BBitmap(brect_from_xrect(make_xrect(0, 0, image->width, image->height)), 0,
		_x_color_space(NULL, image->bits_per_pixel), image->bytes_per_line);
	if (!bbitmap || bbitmap->InitCheck() != B_OK) {
		fprintf(stderr, "libX11: Failed to create bitmap for XImage!\n");
		return 0;
	}
	image->obdata = (char*)bbitmap;

	return 1;
}

extern "C" XImage*
XGetSubImage(Display* display, Drawable d,
	int x, int y, unsigned int width, unsigned int height,
	unsigned long plane_mask, int format,
	XImage* dest_image, int dest_x, int dest_y)
{
	XPixmap* pixmap = Drawables::get_pixmap(d);
	if (!pixmap)
		return NULL;
	if (format != ZPixmap)
		return NULL;

	// TODO: plane_mask?

	BBitmap* bbitmap = (BBitmap*)dest_image->obdata;
	if (!dest_image->data)
		dest_image->data = (char*)bbitmap->Bits();

	const BRect dest_rect = brect_from_xrect(make_xrect(dest_x, dest_y, width, height));
	pixmap->sync();
	bbitmap->ImportBits(pixmap->offscreen(), BPoint(x, y),
		dest_rect.LeftTop(), dest_rect.IntegerWidth(), dest_rect.IntegerHeight());

	if (dest_image->data != bbitmap->Bits()) {
		memcpy(dest_image->data, bbitmap->Bits(),
			dest_image->height * dest_image->bytes_per_line);
	}

	return dest_image;
}

extern "C" XImage*
XGetImage(Display *display, Drawable d,
	int x, int y, unsigned int width, unsigned int height,
	unsigned long plane_mask, int format)
{
	XPixmap* pixmap = Drawables::get_pixmap(d);
	if (!pixmap)
		return NULL;

	XImage* image = XCreateImage(display, NULL, pixmap->depth(), format, 0, NULL,
		width, height, 32, 0);
	if (!image)
		return NULL;

	if (!XGetSubImage(display, d, x, y, width, height, plane_mask, format, image, 0, 0)) {
		XDestroyImage(image);
		return NULL;
	}

	return image;
}
