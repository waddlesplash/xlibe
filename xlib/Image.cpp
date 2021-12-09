#include <interface/Bitmap.h>

#include "Drawables.h"
#include "Debug.h"
#include "Color.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
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
	delete bbitmap;
	delete image;
	return Success;
}

static inline uint8*
GetImageDataPointer(XImage* image, int x, int y)
{
	return (uint8*)&(image->data[
		(y * image->bytes_per_line) + ((x * image->bits_per_pixel) / NBBY)]);
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
		int mask = (0x80 >> ( x %8));
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
		*((uint16 *)destPtr) = (uint16)(pixel & 0xFFFF);
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
		image->bits_per_pixel = 8;
		image->bitmap_unit = 8;
	} else {
		if (!visual)
			visual = display->screens[0].root_visual;
		image->bits_per_pixel = visual->bits_per_rgb;
		image->bitmap_unit = visual->bits_per_rgb;
	}
	if (bytes_per_line == 0)
		bytes_per_line = width * image->bitmap_unit;
	image->bytes_per_line = bytes_per_line;

	image->byte_order = LSBFirst;
	image->bitmap_bit_order = LSBFirst;
	image->red_mask = visual->red_mask;
	image->green_mask = visual->green_mask;
	image->blue_mask = visual->blue_mask;

	image->f.destroy_image = DestroyImage;
	image->f.get_pixel = ImageGetPixel;
	image->f.put_pixel = ImagePutPixel;

	// Now we make the auxiliary bitmap.
	BBitmap* auxBitmap = new BBitmap(BRect(BPoint(0, 0), BSize(width, height)), 0,
		x_color_space(visual, image->bits_per_pixel), image->bytes_per_line);
	if (!auxBitmap) {
		delete image;
		return NULL;
	}
	image->obdata = (char*)auxBitmap;

	// Try to be helpful?
	if (!image->data)
		image->data = (char*)auxBitmap->Bits();

	return image;
}

extern "C" XImage*
XGetImage(Display *display, Drawable d, int x, int y,
  unsigned int width, unsigned int height, unsigned long plane_mask,  int format)
{
	UNIMPLEMENTED();
	return NULL;
}
