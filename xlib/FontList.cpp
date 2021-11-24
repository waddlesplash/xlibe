#include <Font.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <iostream>
#include <map>

extern "C" {
#include <X11/Xlib.h>
}

static std::map<BString, XFontStruct*> sFonts;
static int sLastFontID = 0;

void init_font();
void finalize_font();

static BString create_xlfd(font_family* family, font_style* style, uint16 face, uint32 flag);
void compile_font_pattern(const char* pattern, regex_t& regex);

void init_font()
{
	if (!sFonts.empty())
		return;

	const int max_family = count_font_families();
	for(int i = 0; i != max_family; i++) {
		font_family family;
		get_font_family(i, &family);
		const int max_style = count_font_styles(family);
		for (int j = 0; j !=max_style; ++j) {
			uint32 flag;
			font_style style;
			uint16 face;
			get_font_style(family, j, &style, &face, &flag);

			XFontStruct* font = (XFontStruct*)calloc(1, sizeof(XFontStruct));
			BString xlfd = create_xlfd(&family, &style, face, flag);
			font->fid = ++sLastFontID;
			sFonts.insert({xlfd, font});
		}
	}
}

void finalize_font()
{
	for (const auto& item : sFonts) {
		delete item.second;
	}
	sFonts.clear();
}

static char get_slant(font_style* style, uint16 face)
{
	if ((face & B_ITALIC_FACE) || strstr(*style, "Italic") == NULL)
		return 'r';
	return 'i';
}

static char get_spacing(uint32 flag) {
	if(flag & B_IS_FIXED)
		return 'm';
	return 'p';
}

static const char* get_weight(font_style* style, uint16 face) {
	static const char* medium = "medium";
	static const char* bold = "bold";
	if ((face & B_BOLD_FACE) || strstr(*style, "Bold") == NULL)
		return bold;
	return medium;
}

static const char* get_encoding(font_family* family) {
	static const char* latin = "iso8859-1";
	static const char* hankaku = "jisx0201.1976-0";
	static const char* kanji = "jisx0208.1983-0";
	static const char* testchar = "\xEF\xBD\xB1\xE4\xBA\x9C";
	static bool check[2];
	static BFont font;
	font.SetFamilyAndFace(*family, B_REGULAR_FACE);
	font.GetHasGlyphs(testchar, 2, check);
	if(check[1])
		return kanji;
	if(check[0])
		return hankaku;
	return latin;
}

static BString create_xlfd(font_family* family, font_style* style, uint16 face, uint32 flag)
{
	BString string;
	string.SetToFormat("-TTFont-%s-%s-%c-normal--0-0-0-0-%c-0-%s",
		*family, get_weight(style, face), get_slant(style, face), get_spacing(flag), get_encoding(family));
	return string;
}

int count_wc(const char* pattern) {
	int count = 0;
	const char* i;
	for(i=pattern;*i!='\0';++i)
		if(*i == '*')
			++count;
	std::cout << count << std::endl;
	return count;
}

char* create_regex_pattern_string(const char* pattern) {
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

void compile_font_pattern(const char* pattern, regex_t& regex) {
	if((pattern == NULL) || (strcmp(pattern, "") == 0))
		regcomp(&regex, ".*", REG_NOSUB);
	else
		regcomp(&regex, create_regex_pattern_string(pattern), REG_NOSUB);
}


char **XListFontsWithInfo(Display* display,
	const char*	pattern, int maxNames, int* count, XFontStruct** info_return)
{
	init_font();
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

char** XListFonts(Display *dpy, const char *pattern, int maxNames, int *count)
{
	return XListFontsWithInfo(dpy, pattern, maxNames, count, NULL);
}

extern "C" int XFreeFontNames(char** list)
{
	if (list)
		free(list);
	return 0;
}
