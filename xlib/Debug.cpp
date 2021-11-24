#include <BitmapStream.h>
#include <View.h>
#include <TranslatorRoster.h>
#include <File.h>
#include "Debug.h"

void WriteBitmapToFile(BBitmap* bitmap, const char* filename) {
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
