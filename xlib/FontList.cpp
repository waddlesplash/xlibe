#include <X11/Xlib.h>
#include <X11/Xlcint.h>
#include <Font.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <iostream>

static XFontStruct* gfontstruct = 0;
static char** gfontlist = 0;

void init_font();
void finalize_font();

char get_slant(font_style* style);
char get_spacing(uint32 flag);
const char* get_weight(font_style* style);
char* create_xlfd(font_family* family, font_style* style, uint32 flag);
void compile_font_pattern(const char* pattern, regex_t& regex);

void init_font() {
  if(gfontstruct)
    return;
  int i, last, max_family=count_font_families();
  int cap = max_family * 4;
  gfontlist = (char**) malloc(cap * sizeof(char*));
  gfontstruct = (XFontStruct*) calloc(cap, sizeof(XFontStruct));
  for(i=0,last=0;i!=max_family;++i) {
    font_family family;
    get_font_family(i, &family);
    int j, max_style = count_font_styles(family);
    for(j=0;j!=max_style;++j) {
      uint32 flag;
      font_style style;
      get_font_style(family, j, &style, &flag);
      if(last == cap) {
        cap += 100;
        realloc(gfontlist, cap * sizeof(char*));
        realloc(gfontstruct, cap * sizeof(XFontStruct));
      }
      gfontstruct[last].fid = i * 256 + j;
      gfontlist[last] = create_xlfd(&family, &style, flag);
      ++last;
    }
  }
  gfontlist[last] = 0;
}

void finalize_font() {
  int i;
  if(gfontlist) {
    for(i=0;gfontlist[i] != 0;++i)
      free(gfontlist[i]);
    free(gfontlist);
  }
}

char get_slant(font_style* style) {
  if(strstr(*style, "Italic") == NULL)
    return 'r';
  return 'i';
}

char get_spacing(uint32 flag) {
  if(flag & B_IS_FIXED)
    return 'm';
  return 'p';
}

const char* get_weight(font_style* style) {
  static const char* medium = "medium";
  static const char* bold = "bold";
  if(strstr(*style, "Bold") == NULL)
    return bold;
  return medium;
}

const char* get_encoding(font_family* family) {
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

char* create_xlfd(font_family* family, font_style* style, uint32 flag) {
  char buf[100];
  sprintf(buf, "-TTFont-%s-%s-%c-normal--0-0-0-0-%c-0-%s",
    *family, get_weight(style), get_slant(style), get_spacing(flag), get_encoding(family));
  char* xlfd = (char*) calloc(strlen(buf) + 1, sizeof(char));
  strcpy(xlfd, buf);
  return xlfd;
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


extern "C" char** XListFonts(register Display *dpy, const char *pattern, int maxNames, int *actualCount) {
  init_font();
  regex_t regex;
  char** list;
  *actualCount = 0;
  int i;
  
  compile_font_pattern(pattern, regex);
  list = (char**) malloc(maxNames*sizeof(char*));
  
  for(i=0;i!=maxNames;++i) {
    if(gfontlist[i] == 0)
      break;
    if(regexec(&regex, gfontlist[i], 0, 0, 0) == 0) {
      list[(*actualCount)++] = gfontlist[i];
    }
  }
  regfree(&regex);
  list[*actualCount] = 0;
  return list;
}

extern char **XListFontsWithInfo(Display* display, const char*	pattern, int maxNames, int* count, XFontStruct**	info_return) {
  init_font();
  regex_t regex;
  char** list;
  *count = 0;
  int i, last;
  
  compile_font_pattern(pattern, regex);
  list = (char**) malloc(maxNames*sizeof(char*));
  
  for(i=0, last=0;i!=maxNames;++i) {
    if(gfontlist[i] == 0)
      break;
    if(regexec(&regex, gfontlist[i], 0, 0, 0) == 0) {
      list[last++] = gfontlist[i];
    }
  }
  regfree(&regex);
  list[last] = 0;
  return list;
}

extern "C" int XFreeFontNames(char** list) {
  int i;
  if(list)
    free(list);
  return 0;
}
