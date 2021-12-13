#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

// We simply do not support shapes.

Bool
XShapeQueryExtension(Display *dpy, int *event_basep, int *error_basep)
{
	return False;
}

Status
XShapeQueryVersion(Display *dpy, int *major_versionp, int *minor_versionp)
{
	return BadImplementation;
}

void
XShapeCombineRegion(Display *dpy, Window dest, int destKind, int xOff, int yOff, struct _XRegion* r, int op)
{
}

void
XShapeCombineRectangles(Display* dpy, XID dest, int destKind, int xOff, int yOff, XRectangle* rects, int n_rects, int op, int ordering)
{
}

void
XShapeCombineMask(Display* dpy, XID dest, int destKind, int xOff, int yOff, Pixmap src, int op)
{
}

void
XShapeCombineShape(Display* dpy, XID dest, int destKind, int xOff, int yOff, Pixmap src, int srcKind, int op)
{
}

void
XShapeOffsetShape(Display* dpy, XID dest, int destKind, int xOff, int yOff)
{
}

Status
XShapeQueryExtents(Display *dpy, Window w, Bool* bShaped, int* xbs, int* ybs, unsigned int* wbs, unsigned int* hbs,
	Bool* cShaped, int* xcs, int* ycs, unsigned int* wcs, unsigned int* hcs)
{
}

void
XShapeSelectInput(Display *dpy, Window window, unsigned long mask)
{
}

unsigned long
XShapeInputSelected(Display *dpy, Window window)
{
	return 0;
}

XRectangle*
XShapeGetRectangles(Display *dpy, Window window, int kind, int* count, int* ordering)
{
	return NULL;
}
