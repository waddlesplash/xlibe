/*
 * Copyright 2003, Shibukawa Yoshiki. All rights reserved.
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#include "Font.h"

#include <interface/Font.h>
#include <interface/Rect.h>
#include <support/StringList.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <map>

#include "Drawing.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xatom.h>
}

struct XLFD {
	BString foundry = "*";
	BString family = "*";
	BString weight = "*";
	char slant = '*';
	BString setwidth = "*";
	BString add_style = "*";
	int pixels = 0, decipoints = 0;
	int resolution_x = 0, resolution_y = 0;
	char spacing = '*';
	int average_width = 0;
	BString charset = "*";
	BString encoding = "*";
};

static XLFD create_xlfd(font_family* family, font_style* style, uint16 face, uint32 flag);


struct FontEntry {
	XLFD xlfd;
	font_style style;
};
static std::map<uint16_t, FontEntry*> sFonts;
static uint16_t sLastFontID = 1;

enum {
	DEFAULT_PLAIN_FONT = 0,
	DEFAULT_FIXED_FONT,

	COUNT_DEFAULT_FONTS
};
static uint16_t sDefaultFonts[COUNT_DEFAULT_FONTS] = {};

struct FontSet {
	Display* display;
	Font font;
	XFontSetExtents extents;
};


static Font
make_Font(uint16_t id, uint16_t pointSize)
{
	return (id & UINT16_MAX) | ((pointSize & UINT16_MAX) << 16);
}

static void
extract_Font(Font font, uint16_t& id, uint16_t& pointSize)
{
	id = (font & UINT16_MAX);
	pointSize = (pointSize >> 16) & UINT16_MAX;
}

static FontEntry*
lookup_font(int id)
{
	const auto& it = sFonts.find(id);
	if (it == sFonts.end())
		return NULL;
	return it->second;
}

void
_x_init_font()
{
	if (!sFonts.empty())
		return;

	font_family default_plain_family, default_fixed_family;
	font_style defailt_plain_style, default_fixed_style;
	be_plain_font->GetFamilyAndStyle(&default_plain_family, &defailt_plain_style);
	be_fixed_font->GetFamilyAndStyle(&default_fixed_family, &default_fixed_style);

	font_family family;
	font_style style;
	const int max_family = count_font_families();
	for(int i = 0; i != max_family; i++) {
		get_font_family(i, &family);
		const int max_style = count_font_styles(family);
		for (int j = 0; j < max_style; j++) {
			uint32 flag;
			uint16 face;
			get_font_style(family, j, &style, &face, &flag);

			FontEntry* font = new FontEntry;
			memcpy(font->style, style, sizeof(style));
			font->xlfd = create_xlfd(&family, &style, face, flag);
			const int id = sLastFontID++;
			sFonts.insert({id, font});

			// Check if this is one of the default fonts.
			if (strcmp(family, default_plain_family) == 0 && strcmp(style, defailt_plain_style) == 0)
				sDefaultFonts[DEFAULT_PLAIN_FONT] = id;
			else if (strcmp(family, default_fixed_family) == 0 && strcmp(style, default_fixed_style) == 0)
				sDefaultFonts[DEFAULT_FIXED_FONT] = id;
		}
	}
}

void
_x_finalize_font()
{
	for (const auto& item : sFonts)
		delete item.second;
	sFonts.clear();
}

BFont
bfont_from_font(Font fid)
{
	uint16_t id, pointSize;
	extract_Font(fid, id, pointSize);

	BFont font;
	FontEntry* fontId = lookup_font(id);
	if (!fontId)
		return font;
	font.SetFamilyAndStyle(fontId->xlfd.family, fontId->style);
	if (pointSize)
		font.SetSize(pointSize);
	return font;
}

static char
get_slant(font_style* style, uint16 face)
{
	if ((face & B_ITALIC_FACE) || strstr(*style, "Italic") == NULL)
		return 'r';
	return 'i';
}

static char
get_spacing(uint32 flag)
{
	if (flag & B_IS_FIXED)
		return 'm';
	return 'p';
}

static const char*
get_weight(font_style* style, uint16 face)
{
	static const char* medium = "medium";
	static const char* bold = "bold";
	if ((face & B_BOLD_FACE) || strstr(*style, "Bold") != NULL)
		return bold;
	return medium;
}

static const char*
get_encoding(font_family* family)
{
	static const char* iso10646_8 = "iso10646-8";
	static const char* iso8859_1 = "iso8859-1";
	static const char* hankaku = "jisx0201.1976-0";
	static const char* kanji = "jisx0208.1983-0";

	static const char* testchar = "\xEF\xBD\xB1\xE4\xBA\x9C";

	BFont font;
	font.SetFamilyAndFace(*family, B_REGULAR_FACE);
	switch (font.Encoding()) {
	case B_UNICODE_UTF8:
		return iso10646_8;
	case B_ISO_8859_1:
		return iso8859_1;
	default: {
		static bool check[2];
		font.GetHasGlyphs(testchar, 2, check);
		if (check[1])
			return kanji;
		if (check[0])
			return hankaku;
		// presume Latin?
		return iso8859_1;
	}
	}
}

static XLFD
create_xlfd(font_family* family, font_style* style, uint16 face, uint32 flag)
{
	XLFD xlfd;
	xlfd.foundry = "TTFont";
	xlfd.family = *family;
	xlfd.weight = get_weight(style, face);
	xlfd.slant = get_slant(style, face);
	xlfd.setwidth = "normal";
	xlfd.add_style = "";
	xlfd.spacing = get_spacing(flag);

	xlfd.charset = get_encoding(family);
	xlfd.charset.MoveCharsInto(xlfd.encoding, xlfd.charset.FindLast('-') + 1, 8);
	xlfd.charset.RemoveLast("-");
	return xlfd;
}

static XLFD
parse_xlfd(const char* string)
{
	XLFD xlfd;
	const BString bstring(string);
	if (bstring.FindFirst('-') < 0) {
		// Special case: set family only.
		xlfd.family = bstring;

		// Special case: fixed has a different default size.
		if (bstring == "fixed")
			xlfd.decipoints = 100;

		return xlfd;
	}

	BStringList values;
	bstring.Split("-", false, values);

	for (int field = 0; field < values.CountStrings(); field++) {
		const BString value = values.StringAt(field);

		switch (field) {
		case 1: xlfd.foundry = value; break;
		case 2: xlfd.family = value; break;
		case 3: xlfd.weight = value; break;
		case 4: xlfd.slant = value.ByteAt(0); break;
		case 5: xlfd.setwidth = value; break;
		case 6: xlfd.add_style = value; break;
		case 7: xlfd.pixels			= strtol(value.String(), NULL, 10); break;
		case 8: xlfd.decipoints		= strtol(value.String(), NULL, 10); break;
		case 9: xlfd.resolution_x	= strtol(value.String(), NULL, 10); break;
		case 10: xlfd.resolution_y	= strtol(value.String(), NULL, 10); break;
		case 11: xlfd.spacing = value.ByteAt(0); break;
		case 12: xlfd.average_width	= strtol(value.String(), NULL, 10); break;
		case 13: xlfd.charset = value; break;
		case 14: xlfd.encoding = value; break;
		break;
		}
	}
	return xlfd;
}

static BString
serialize_xlfd(const XLFD& xlfd)
{
	BString string;
	string.SetToFormat("-%s-%s-%s-%c-%s-%s-%d-%d-%d-%d-%c-%d-%s-%s",
		xlfd.foundry.String(), xlfd.family.String(), xlfd.weight.String(),
		xlfd.slant, xlfd.setwidth.String(), xlfd.add_style.String(),
		xlfd.pixels, xlfd.decipoints, xlfd.resolution_x, xlfd.resolution_y,
		xlfd.spacing, xlfd.average_width, xlfd.charset.String(), xlfd.encoding.String());
	return string;
}

static bool
compare_xlfds(const XLFD& compare, const XLFD& base, uint16_t baseID)
{
	const BString wild_string = "*";

	// skip: foundry, resolution_x, resolution_y, average_width
#define COMPARE(FIELD, WILD) (compare.FIELD == WILD || base.FIELD == WILD || compare.FIELD == base.FIELD)
#define COMPARE_CHAR(FIELD) \
	(compare.FIELD == '*' || base.FIELD == '*' \
		|| compare.FIELD == base.FIELD || tolower(compare.FIELD) == base.FIELD)
#define COMPARE_STRING(FIELD) \
	(compare.FIELD == wild_string || base.FIELD == wild_string \
		|| compare.FIELD.ICompare(base.FIELD) == 0)

	if (!COMPARE_STRING(family)) {
		// Special case: "Helvetica" matches the default display font,
		// and "fixed" or "cursor" matches the default fixed font.
		bool match = false;
		if (baseID == sDefaultFonts[DEFAULT_PLAIN_FONT] && compare.family.ICompare("Helvetica") == 0)
			match = true;
		if (baseID == sDefaultFonts[DEFAULT_FIXED_FONT] && compare.family.ICompare("fixed") == 0)
			match = true;
		if (baseID == sDefaultFonts[DEFAULT_FIXED_FONT] && compare.family.ICompare("cursor") == 0)
			match = true;
		if (!match)
			return false;
	}
	if (!COMPARE_STRING(weight))
		return false;
	if (!COMPARE_CHAR(slant))
		return false;
	if (!COMPARE_STRING(setwidth))
		return false;
	if (!COMPARE_STRING(add_style))
		return false;
	if (!COMPARE(pixels, 0))
		return false;
	if (!COMPARE(decipoints, 0))
		return false;
	if (!COMPARE_CHAR(spacing))
		return false;
	if (!COMPARE_STRING(charset))
		return false;
	if (!COMPARE_STRING(encoding))
		return false;

#undef COMPARE

	return true;
}

extern "C" char**
XListFontsWithInfo(Display* display,
	const char*	pattern, int maxNames, int* count, XFontStruct** info_return)
{
	*count = 0;

	XLFD patternXLFD = parse_xlfd(pattern);
	patternXLFD.decipoints = patternXLFD.pixels = 0;

	char** nameList = (char**)malloc(maxNames * sizeof(char*) + 1);
	for (const auto& font : sFonts) {
		if (compare_xlfds(patternXLFD, font.second->xlfd, font.first)) {
			int index = (*count)++;
			nameList[index] = strdup(serialize_xlfd(font.second->xlfd).String());
			if (info_return)
				info_return[index] = XQueryFont(display, font.first);
		}

		if ((*count) == maxNames)
			break;
	}

	nameList[*count] = 0;
	return nameList;
}

extern "C" Font
XLoadFont(Display* dpy, const char* name)
{
	XLFD patternXLFD = parse_xlfd(name);

	// Clear out sizes before comparing.
	uint16 ptSize = patternXLFD.decipoints / 10;
	if (ptSize == 0)
		ptSize = patternXLFD.pixels * 0.75f;
	patternXLFD.decipoints = patternXLFD.pixels = 0;

	int id = 0;
	for (const auto& font : sFonts) {
		if (compare_xlfds(patternXLFD, font.second->xlfd, font.first)) {
			id = font.first;
			break;
		}
	}

	if (id != 0)
		return make_Font(id, ptSize);
	return 0;
}

extern "C" int
XUnloadFont(Display* dpy, Font font)
{
	// Nothing to do.
	return Success;
}

extern "C" char**
XListFonts(Display *dpy, const char *pattern, int maxNames, int *count)
{
	return XListFontsWithInfo(dpy, pattern, maxNames, count, NULL);
}

extern "C" int
XFreeFontNames(char** list)
{
	if (list) {
		int i = 0;
		while (list[i] != NULL) {
			free(list[i]);
			i++;
		}
	}
	free(list);
	return 0;
}

extern "C" XFontStruct*
XQueryFont(Display *display, Font id)
{
	FontEntry* ident = lookup_font(id);
	if (!ident)
		return NULL;

	BFont bfont = bfont_from_font(id);

	XFontStruct* font = (XFontStruct*)calloc(1, sizeof(XFontStruct));
	font->fid = id;
	font->direction = (bfont.Direction() == B_FONT_LEFT_TO_RIGHT)
		? FontLeftToRight : FontRightToLeft;

	font->min_char_or_byte2 = 0;
	font->max_char_or_byte2 = 0xFF;
	font->min_byte1 = 0;
	font->max_byte1 = 0;

	font_height height;
	bfont.GetHeight(&height);
	font->ascent = font->max_bounds.ascent = height.ascent;
	font->descent = font->max_bounds.descent = height.descent;

	font->max_bounds.width = bfont.StringWidth("@");

	return font;
}

extern "C" XFontStruct*
XLoadQueryFont(Display *display, const char *name)
{
	return XQueryFont(display, XLoadFont(display, name));
}

extern "C" int
XFreeFont(Display *dpy, XFontStruct *fs)
{
	free(fs);
	return Success;
}

extern "C" XFontSet
XCreateFontSet(Display* dpy, const char* base_font_name_list,
	char*** missing_charset_list_return, int* missing_charset_count_return, char** def_string_return)
{
	BStringList fonts;
	BString(base_font_name_list).Split(",", true, fonts);

	// As we deal with everything in UTF-8 internally, we do not need
	// to deal with encodings. So, just load fonts from the list till we
	// succeed with at least one.
	Font font = 0;
	for (int i = 0; i < fonts.CountStrings(); i++) {
		font = XLoadFont(dpy, fonts.StringAt(i).String());
		if (font != 0)
			break;
	}
	if (font == 0)
		return NULL;

	FontSet* fontset = new FontSet;
	fontset->display = dpy;
	fontset->font = font;

	XFontStruct* st = XQueryFont(dpy, font);

	// Come up with some kind of values for the extents.
	// TODO: How important are these? How to improve them?
	fontset->extents.max_ink_extent = make_xrect(0, 0,
		st->max_bounds.width, st->max_bounds.ascent + st->max_bounds.descent);
	fontset->extents.max_logical_extent = make_xrect(0, 0,
		st->max_bounds.width + 1, fontset->extents.max_ink_extent.height + 1);

	XFreeFont(dpy, st);

	if (missing_charset_list_return) {
		*missing_charset_list_return = NULL;
		*missing_charset_count_return = 0;
	}
	if (def_string_return)
		*def_string_return = NULL;
	return (XFontSet)fontset;
}

extern "C" int
XFontsOfFontSet(XFontSet font_set,
	XFontStruct*** font_struct_list_return, char*** font_name_list_return)
{
	FontSet* fontset = (FontSet*)font_set;

	XFontStruct** structs = (XFontStruct**)calloc(sizeof(XFontStruct*), 1);
	structs[0] = XQueryFont(fontset->display, fontset->font);

	if (font_name_list_return) {
		char** names = (char**)calloc(sizeof(char*), 1);
		Atom name = None;
		XGetFontProperty(structs[0], XA_FONT, &name);
		names[0] = XGetAtomName(fontset->display, name);
	}

	if (font_struct_list_return) {
		*font_struct_list_return = structs;
	} else {
		XFreeFont(fontset->display, structs[0]);
		free(structs);
	}
	return Success;
}

extern "C" XFontSetExtents*
XExtentsOfFontSet(XFontSet xf)
{
	return &((FontSet*)xf)->extents;
}

extern "C" void
XFreeFontSet(Display* dpy, XFontSet xf)
{
	FontSet* fontset = (FontSet*)xf;
	XUnloadFont(dpy, fontset->font);
	delete fontset;
}

extern "C" Bool
XGetFontProperty(XFontStruct* font_struct, Atom atom, Atom* value_return)
{
	uint16_t id, pointSize;
	extract_Font(font_struct->fid, id, pointSize);
	if (atom == XA_FONT) {
		for (const auto& font : sFonts) {
		   if (font.first != id)
			   continue;
		   *value_return = XInternAtom(NULL, serialize_xlfd(font.second->xlfd).String(), False);
		   return True;
		}
	}
	return False;
}

extern "C" int
XTextWidth(XFontStruct* font_struct, const char *string, int count)
{
	BFont bfont = bfont_from_font(font_struct->fid);
	return bfont.StringWidth(string, count);
}

extern "C" int
XTextExtents(XFontStruct* font_struct, const char* string, int nchars,
	int* direction_return, int* font_ascent_return, int* font_descent_return, XCharStruct* overall_return)
{
	const BFont bfont = bfont_from_font(font_struct->fid);
	BString copy(string, nchars);
	const char* strings[] = {copy.String()};
	BRect boundingBoxes[1];
	bfont.GetBoundingBoxesForStrings(strings, 1, B_SCREEN_METRIC, NULL, boundingBoxes);

	if (direction_return)
		*direction_return = font_struct->direction;
	if (font_ascent_return)
		*font_ascent_return = font_struct->ascent;
	if (font_descent_return)
		*font_descent_return = font_struct->descent;

	memset(overall_return, 0, sizeof(XCharStruct));
	overall_return->ascent = boundingBoxes[0].top;
	overall_return->descent = boundingBoxes[0].bottom;
	overall_return->width = boundingBoxes[0].IntegerWidth() + 1;
	return Success;
}

extern "C" int
Xutf8TextEscapement(XFontSet font_set, const char* string, int num_bytes)
{
	XFontStruct dummy;
	dummy.fid = ((FontSet*)font_set)->font;
	return XTextWidth(&dummy, string, num_bytes);
}
