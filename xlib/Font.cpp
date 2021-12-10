#include <interface/Font.h>
#include <interface/Rect.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <map>

#include "Font.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xatom.h>
}

struct FontIdentifier {
	font_family family;
	font_style style;
};
static std::map<BString, FontIdentifier*> sFonts;

static BString create_xlfd(font_family* family, font_style* style, uint16 face, uint32 flag);
static void compile_font_pattern(const char* pattern, regex_t& regex);

void init_font()
{
	if (!sFonts.empty())
		return;

	font_family family;
	font_style style;
	const int max_family = count_font_families();
	for(int i = 0; i != max_family; i++) {
		get_font_family(i, &family);
		const int max_style = count_font_styles(family);
		for (int j = 0; j !=max_style; ++j) {
			uint32 flag;
			uint16 face;
			get_font_style(family, j, &style, &face, &flag);

			FontIdentifier* font = new FontIdentifier;
			memcpy(font->family, family, sizeof(family));
			memcpy(font->style, style, sizeof(style));
			BString xlfd = create_xlfd(&family, &style, face, flag);
			sFonts.insert({xlfd, font});
		}
	}

	// Special case: "fixed".
	FontIdentifier* font = new FontIdentifier;
	be_fixed_font->GetFamilyAndStyle(&family, &style);
	memcpy(font->family, family, sizeof(family));
	memcpy(font->style, style, sizeof(style));
	sFonts.insert({"fixed", font});
}

void finalize_font()
{
	for (const auto& item : sFonts) {
		delete item.second;
	}
	sFonts.clear();
}

BFont
bfont_from_font(Font fid)
{
	BFont font;
	FontIdentifier* fontId = (FontIdentifier*)fid;
	font.SetFamilyAndStyle(fontId->family, fontId->style);
	return font;
}

static char get_slant(font_style* style, uint16 face)
{
	if ((face & B_ITALIC_FACE) || strstr(*style, "Italic") == NULL)
		return 'r';
	return 'i';
}

static char get_spacing(uint32 flag)
{
	if(flag & B_IS_FIXED)
		return 'm';
	return 'p';
}

static const char* get_weight(font_style* style, uint16 face)
{
	static const char* medium = "medium";
	static const char* bold = "bold";
	if ((face & B_BOLD_FACE) || strstr(*style, "Bold") == NULL)
		return bold;
	return medium;
}

static const char* get_encoding(font_family* family)
{
	static const char* iso8859_1 = "iso8859-1";
	static const char* hankaku = "jisx0201.1976-0";
	static const char* kanji = "jisx0208.1983-0";

	static const char* testchar = "\xEF\xBD\xB1\xE4\xBA\x9C";

	BFont font;
	font.SetFamilyAndFace(*family, B_REGULAR_FACE);
	switch (font.Encoding()) {
	case B_UNICODE_UTF8:
		// FIXME: just say it's Latin-1 for now
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

static BString create_xlfd(font_family* family, font_style* style, uint16 face, uint32 flag)
{
	BString string;
	string.SetToFormat("-TTFont-%s-%s-%c-normal--0-0-0-0-%c-0-%s",
		*family, get_weight(style, face), get_slant(style, face), get_spacing(flag), get_encoding(family));
	return string;
}

static int count_wc(const char* pattern)
{
	int count = 0;
	const char* i;
	for(i=pattern;*i!='\0';++i)
		if(*i == '*')
			++count;
	return count;
}

static char* create_regex_pattern_string(const char* pattern)
{
	int i;
	const int end = strlen(pattern) + count_wc(pattern);
	char* regpattern = (char*) malloc(end + 1);
	const char* c;
	for(i=0,c=pattern;i!=end;++i,++c) {
		switch(*c) {
		case '*':
			regpattern[i] = '.';
			regpattern[++i] = '*';
			break;
		case '?':
			regpattern[i] = '.';
		default:
			regpattern[i] = *c;
		}
	}
	regpattern[end] = '\0';
	return regpattern;
}

static void compile_font_pattern(const char* pattern, regex_t& regex)
{
	if ((pattern == NULL) || (strcmp(pattern, "") == 0))
		regcomp(&regex, ".*", REG_NOSUB);
	else
		regcomp(&regex, create_regex_pattern_string(pattern), REG_NOSUB);
}


extern "C" char**
XListFontsWithInfo(Display* display,
	const char*	pattern, int maxNames, int* count, XFontStruct** info_return)
{
	*count = 0;

	regex_t regex;
	compile_font_pattern(pattern, regex);

	char** nameList = (char**) malloc(maxNames * sizeof(char*) + 1);
	// TODO: Support info_return!

	for (const auto& font : sFonts) {
		if (regexec(&regex, font.first.String(), 0, 0, 0) == 0) {
			int index = (*count)++;
			nameList[index] = strdup(font.first.String());
		}

		if ((*count) == maxNames)
			break;
	}

	regfree(&regex);
	nameList[*count] = 0;
	if (info_return)
		*info_return = NULL;
	return nameList;
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
	FontIdentifier* ident = (FontIdentifier*)id;
	if (!ident)
		return NULL;

	BFont bfont = bfont_from_font(id);

	XFontStruct* font = (XFontStruct*)calloc(1, sizeof(XFontStruct));
	font->fid = id;
	font->direction = (bfont.Direction() == B_FONT_LEFT_TO_RIGHT)
		? FontLeftToRight : FontRightToLeft;

	font_height height;
	bfont.GetHeight(&height);
	font->ascent = font->max_bounds.ascent = height.ascent;
	font->descent = font->max_bounds.descent = height.descent;

	// TODO: Improve!
	font->max_bounds.width = bfont.StringWidth("@");

	// TODO: Fill in more!
	return font;
}

extern "C" Font
XLoadFont(Display *dpy, const char *name)
{
	regex_t regex;
	compile_font_pattern(name, regex);

	// FIXME: Load fonts at the actually specified sizes!

	Font fnt = None;
	for (const auto& font : sFonts) {
		if (regexec(&regex, font.first.String(), 0, 0, 0) == 0) {
			fnt = (Font)font.second;
			break;
		}
	}
	// HACK!
	if (!fnt)
		fnt = (Font)&*sFonts.at("fixed");

	regfree(&regex);
	return fnt;
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

extern "C" Bool
XGetFontProperty(XFontStruct *font_struct, Atom atom, Atom *value_return)
{
	if (atom == XA_FONT) {
		for (const auto& font : sFonts) {
		   if (font.second != (FontIdentifier*)font_struct->fid)
			   continue;
		   *value_return = XInternAtom(NULL, font.first.String(), False);
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
XTextWidth16(XFontStruct *font_struct, const XChar2b *string, int count)
{
	return XTextWidth(font_struct, (const char*)string, count);
}

extern "C" int
XTextExtents(XFontStruct* font_struct, const char *string, int count,
	int* direction_return, int* font_ascent_return, int* font_descent_return, XCharStruct* overall_return)
{
	BFont bfont = bfont_from_font(font_struct->fid);
	const char* strings[] = {string};
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
	overall_return->width = boundingBoxes[0].IntegerWidth();
	return Success;
}
