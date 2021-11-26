#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char* argv[])
{
	Display* d = XOpenDisplay(0);
	if (!d)
	{
		fprintf (stderr, "Oops, can't open display\n");
		exit(EXIT_FAILURE);
	}

	while (*++argv)
	{
		XFontStruct* f = XLoadQueryFont(d, *argv);
		unsigned long ret;
		if (f == 0)
			printf ("XLoadQueryFont failed for %s!\n", *argv);
		else
		{
			if (!XGetFontProperty(f, XA_FONT, &ret))
				printf ("XGetFontProperty(%s, XA_FONT) failed!\n", *argv);
			else
				printf ("Full name for %s is %s\n", *argv, XGetAtomName(d, (Atom)ret));
		}
	}
	return 0;
}
