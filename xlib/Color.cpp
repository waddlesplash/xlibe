#include <X11/Xlib.h>
#include "ColorTable.h"
#include "Color.h"
#include <ctype.h>
#include <stdio.h>
#include <iostream>

static int numXColors = 0;

long RGB(unsigned short red, unsigned short green, unsigned short blue) {
  long result = red / 256 + green + (blue & 0xFF00) * 256;
  return result;
}

rgb_color create_rgb(unsigned long color) {
  rgb_color rgb;
  rgb.red = 255 & color;
  color >>= 8;
  rgb.green = 255 & color;
  color >>= 8;
  rgb.blue = color;
  return rgb;
}

int strcasecmp(const char *a, const char *b)
{
  int i = 0, c;
  if((a == NULL) || (b == NULL)) return -1;
  while(((!(c = toupper(a[i]) - toupper(b[i]))) && a[i] && b[i])) i++;
  return c;
}

int FindColor(const char *name, XColor *def) {
  int l, u, r, i;
  if (numXColors == 0) {
    XColorEntry *ePtr;
    for (ePtr = xColors; ePtr->name != NULL; ePtr++) {
      numXColors++;
    }
  }
  l = 0;
  u = numXColors - 1;
  while (l <= u) {
    i = (l + u) / 2;
    r = strcasecmp(name, xColors[i].name);
    if (r == 0) {
      break;
    } else if (r < 0) {
      u = i-1;
    } else {
      l = i+1;
    }
  }
  if (l > u) {
    return 0;
  }
  def->red   = xColors[i].red << 8;
  def->green = xColors[i].green << 8;
  def->blue  = xColors[i].blue << 8;
  return 1;
}

extern "C" Status XAllocNamedColor(Display *dpy, Colormap cmap, const char *colorname, XColor *hard_def, XColor *exact_def) {
  XParseColor(dpy, cmap, colorname, exact_def);
  hard_def->pixel = exact_def->pixel;
  return 0;
}

extern "C" Status XParseColor(Display *dpy, Colormap cmap, const char *spec, XColor *def) {
  if (spec[0] == '#') {
    char fmt[16];
    int i, red, green, blue;
  
    if ((i = strlen(spec+1))%3) {
      return 0;
    }
    i /= 3;

    sprintf(fmt, "%%%dx%%%dx%%%dx", i, i, i);
    if (sscanf(spec+1, fmt, &red, &green, &blue) != 3) {
      return 0;
    }
    def->red = ((unsigned short) red) << 8;
    def->green = ((unsigned short) green) << 8;
    def->blue = ((unsigned short) blue) << 8;
  } else {
    if (!FindColor(spec, def)) {
      return 0;
    }
  }
  def->pixel = RGB(def->red, def->green, def->blue);
  def->flags = DoRed | DoGreen | DoBlue;
  def->pad   = 0;

  return 1;
}

extern "C" Status XAllocColor(Display *dpy, Colormap cmap, XColor *def) {
  def->pixel = RGB(def->red , def->green , def->blue);
  return 0;
}
