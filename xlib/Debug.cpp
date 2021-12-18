/*
 * Copyright 2003, Shibukawa Yoshiki. All rights reserved.
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#include "Debug.h"

#include <translation/BitmapStream.h>
#include <translation/TranslatorRoster.h>
#include <interface/Bitmap.h>
#include <interface/View.h>
#include <storage/File.h>

#include <cstdio>

extern "C" void
_x_trace(const char* trace, const char* func)
{
	fprintf(stderr, "%s: %s\n", trace, func);
}

void
WriteBitmapToFile(BBitmap* bitmap, const char* filename)
{
	BBitmap* writeimage = new BBitmap(bitmap->Bounds().OffsetToCopy(0, 0), bitmap->ColorSpace(), true);
	BView* view = new BView(bitmap->Bounds().OffsetToCopy(0, 0), NULL, 0, 0);
	writeimage->AddChild(view);

	writeimage->Lock();
	view->DrawBitmap(bitmap);
	writeimage->Unlock();

	BTranslatorRoster *roster = BTranslatorRoster::Default();
	BBitmapStream stream(writeimage);
	BFile file(filename, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
	roster->Translate(&stream, NULL, NULL, &file, B_PNG_FORMAT);
}
