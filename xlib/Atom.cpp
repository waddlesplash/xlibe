extern "C" {
#include <X11/Xlib.h>
}

#include <unordered_set>

static std::unordered_set<const char*> sAtoms;

extern "C" Atom
XInternAtom(Display *dpy, const char *name, Bool onlyIfExists)
{
	if (onlyIfExists)
		return sAtoms.find(name) != sAtoms.end() ? (Atom)name : 0;
	sAtoms.insert(name);
	return (Atom)name;
}

extern "C" Status
XInternAtoms(Display *dpy,
	char **names, int count, Bool onlyIfExists, Atom *atoms_return)
{
	int i, missed = 0;
	for (i = 0; i < count; i++) {
		if (!(atoms_return[i] = XInternAtom(dpy, names[i], onlyIfExists))) {
			missed = i;
		}
	}
	return missed ? Success : BadAtom;
}
