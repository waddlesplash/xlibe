/*
 * Copyright 2003, Shibukawa Yoshiki. All rights reserved.
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <interface/Region.h>
#include <interface/Bitmap.h>
#include <stdio.h>

#include "Drawables.h"
#include "Color.h"
#include "Font.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
}

#include "Debug.h"

struct ClipMask {
	BRegion region;
};

extern "C" GC
XCreateGC(Display* display, Window window,
	unsigned long mask, XGCValues* gc_values)
{
	GC gc = new _XGC;
	gc->values.function = GXcopy;
	gc->values.foreground = BlackPixel(display, 0);
	gc->values.background = WhitePixel(display, 0);
	gc->values.line_style = LineSolid;
	gc->values.line_width = 0;
	gc->values.cap_style = CapButt;
	gc->values.join_style = JoinMiter;
	gc->values.fill_style = FillSolid;
	gc->values.fill_rule = EvenOddRule;
	gc->values.arc_mode = ArcChord;
	gc->values.font = 0;
	gc->values.subwindow_mode = ClipByChildren;
	gc->values.clip_x_origin = gc->values.clip_y_origin = 0;
	gc->values.clip_mask = None;
	gc->dirty = True;
	XChangeGC(display, gc, mask, gc_values);
	return gc;
}

extern "C" int
XFreeGC(Display* display, GC gc)
{
	if (gc) {
		delete (ClipMask*)gc->values.clip_mask;
		delete gc;
	}
	return Success;
}

extern "C" int
XGetGCValues(Display* display, GC gc, unsigned long mask, XGCValues* values)
{
	if (mask & GCFunction)
		values->function = gc->values.function;
	if (mask & GCPlaneMask)
		values->plane_mask = gc->values.plane_mask;
	if (mask & GCForeground)
		values->foreground = gc->values.foreground;
	if (mask & GCBackground)
		values->background = gc->values.background;
	if (mask & GCLineWidth)
		values->line_width = gc->values.line_width;
	if (mask & GCLineStyle)
		values->line_style = gc->values.line_style;
	if (mask & GCCapStyle)
		values->cap_style = gc->values.cap_style;
	if (mask & GCJoinStyle)
		values->join_style = gc->values.join_style;
	if (mask & GCFillStyle)
		values->fill_style = gc->values.fill_style;
	if (mask & GCFillRule)
		values->fill_rule = gc->values.fill_rule;
	if (mask & GCArcMode)
		values->arc_mode = gc->values.arc_mode;
	if (mask & GCTile)
		values->tile = gc->values.tile;
	if (mask & GCStipple)
		values->stipple = gc->values.stipple;
	if (mask & GCTileStipXOrigin)
		values->ts_x_origin = gc->values.ts_x_origin;
	if (mask & GCTileStipYOrigin)
		values->ts_y_origin = gc->values.ts_y_origin;
	if (mask & GCFont)
		values->font = gc->values.font;
	if (mask & GCSubwindowMode)
		values->subwindow_mode = gc->values.subwindow_mode;
	if (mask & GCGraphicsExposures)
		values->graphics_exposures = gc->values.graphics_exposures;
	if (mask & GCClipXOrigin)
		values->clip_x_origin = gc->values.clip_x_origin;
	if (mask & GCClipYOrigin)
		values->clip_y_origin = gc->values.clip_y_origin;
#if 0
	// TODO
	if (mask & GCClipMask)
		...
#endif
	if (mask & GCDashOffset)
		values->dash_offset = gc->values.dash_offset;
#if 0
	// TODO
	if (mask & GCDashList)
		...
#endif
	return 1;
}

extern "C" int
XChangeGC(Display *display, GC gc, unsigned long mask, XGCValues *values)
{
	if (mask & GCFunction)
		gc->values.function = values->function;
	if (mask & GCPlaneMask)
		gc->values.plane_mask = values->plane_mask;
	if (mask & GCForeground)
		gc->values.foreground = values->foreground;
	if (mask & GCBackground)
		gc->values.background = values->background;
	if (mask & GCLineWidth)
		gc->values.line_width = values->line_width;
	if (mask & GCLineStyle)
		gc->values.line_style = values->line_style;
	if (mask & GCCapStyle)
		gc->values.cap_style = values->cap_style;
	if (mask & GCJoinStyle)
		gc->values.join_style = values->join_style;
	if (mask & GCFillStyle)
		gc->values.fill_style = values->fill_style;
	if (mask & GCFillRule)
		gc->values.fill_rule = values->fill_rule;
	if (mask & GCArcMode)
		gc->values.arc_mode = values->arc_mode;
	if (mask & GCTile)
		gc->values.tile = values->tile;
	if (mask & GCStipple)
		gc->values.stipple = values->stipple;
	if (mask & GCTileStipXOrigin)
		gc->values.ts_x_origin = values->ts_x_origin;
	if (mask & GCTileStipYOrigin)
		gc->values.ts_y_origin = values->ts_y_origin;
	if (mask & GCFont)
		gc->values.font = values->font;
	if (mask & GCSubwindowMode)
		gc->values.subwindow_mode = values->subwindow_mode;
	if (mask & GCGraphicsExposures)
		gc->values.graphics_exposures = values->graphics_exposures;
	if (mask & GCClipXOrigin)
		gc->values.clip_x_origin = values->clip_x_origin;
	if (mask & GCClipYOrigin)
		gc->values.clip_y_origin = values->clip_y_origin;
	if (mask & GCClipMask) {
		// Presume this is a real pixmap, as we don't control GCValues.
		XSetClipMask(display, gc, values->clip_mask);
	}
	if (mask & GCDashOffset)
		gc->values.dash_offset = values->dash_offset;
#if 0
	// TODO
	if (mask & GCDashList)
		XSetDashes(display, gc, &values->dashes, 2);
#endif
	gc->dirty = True;
	return 0;
}

extern "C" int
XCopyGC(Display *display, GC src, unsigned long mask, GC dest)
{
	int status = XChangeGC(display, dest, mask & ~GCClipMask, &src->values);
	if (!status)
		return status;

	if (mask & GCClipMask) {
		ClipMask* clip_mask = (ClipMask*)src->values.clip_mask;
		dest->values.clip_mask = (Pixmap)(mask ? new ClipMask(*clip_mask) : None);
	}
	return 0;
}

extern "C" int
XSetFunction(Display *display, GC gc, int function)
{
	gc->values.function = function;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetForeground(Display *display, GC gc, unsigned long color)
{
	gc->values.foreground = color;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetBackground(Display *display, GC gc, unsigned long color)
{
	gc->values.background = color;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetGraphicsExposures(Display *display, GC gc, Bool graphics_exposures)
{
	gc->values.graphics_exposures = graphics_exposures;
	return 0;
}

extern "C" int
XSetLineAttributes(Display* display, GC gc,
	unsigned int line_width, int line_style, int cap_style, int join_style)
{
	gc->values.line_width = line_width;
	gc->values.line_style = line_style;
	gc->values.cap_style = cap_style;
	gc->values.join_style = join_style;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetFillStyle(Display* display, GC gc, int fill_style)
{
	gc->values.fill_style = fill_style;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetFillRule(Display* display, GC gc, int fill_rule)
{
	gc->values.fill_rule = fill_rule;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetArcMode(Display* display, GC gc, int arc_mode)
{
	gc->values.arc_mode = arc_mode;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetFont(Display *display, GC gc, Font font)
{
	gc->values.font = font;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetSubwindowMode(Display *display, GC gc, int subwindow_mode)
{
	gc->values.subwindow_mode = subwindow_mode;
	gc->dirty = True;
	return 0;
}

extern "C" int
XSetClipOrigin(Display *display, GC gc, int clip_x_origin, int clip_y_origin)
{
	gc->values.clip_x_origin = clip_x_origin;
	gc->values.clip_y_origin = clip_y_origin;
	gc->dirty = True;
	return 0;
}

static inline ClipMask*
gc_clip_mask(GC gc, bool allocate = true)
{
	ClipMask* mask = (ClipMask*)gc->values.clip_mask;
	if (!mask && allocate) {
		mask = new ClipMask;
		gc->values.clip_mask = (Pixmap)mask;
	}
	return mask;
}

extern "C" int
XSetRegion(Display* display, GC gc, Region r)
{
	ClipMask* mask = gc_clip_mask(gc);
	BRegion* region = (BRegion*)r;
	mask->region = *region;
	gc->dirty = True;
	return Success;
}

extern "C" int
XSetClipRectangles(Display *display, GC gc, int clip_x_origin, int clip_y_origin,
	XRectangle* rect, int count, int ordering)
{
	ClipMask* mask = gc_clip_mask(gc);

	XSetClipOrigin(display, gc, clip_x_origin, clip_y_origin);

	mask->region.MakeEmpty();
	for (int i = 0; i < count; i++)
		XUnionRectWithRegion(&rect[i], (Region)&mask->region, (Region)&mask->region);

	gc->dirty = True;
	return Success;
}

extern "C" Status
XSetClipMask(Display* display, GC gc, Pixmap pixmap)
{
	XPixmap* pxm = Drawables::get_pixmap(pixmap);
	if (!pxm)
		return BadPixmap;

	ClipMask* mask = gc_clip_mask(gc);
	mask->region.Set(pxm->offscreen()->Bounds());

	// TODO: Actually use the pixmap for clipping!
	UNIMPLEMENTED();

	gc->dirty = True;
	return Success;
}

extern "C" Status
XSetDashes(Display *display, GC gc, int dash_offset, const char *dash_list, int n)
{
	// Not supported.
	return BadImplementation;
}

void
bex_check_gc(XDrawable* drawable, GC gc)
{
	if (!gc) {
		// Use the window's default GC, or make one for it.
		if (!drawable->default_gc)
			drawable->default_gc = XCreateGC(NULL, 0, 0, NULL);
		gc = drawable->default_gc;
	}
	if (drawable->gc == gc && !gc->dirty)
		return;
	drawable->gc = gc;

	BView* view = drawable->view();

	drawing_mode mode;
	alpha_function func = B_ALPHA_OVERLAY;
	switch (gc->values.function) {
	//case GXclear:
	case GXand:
		mode = B_OP_BLEND;
	break;
	//case GXandReverse:
	case GXcopy:
		mode = B_OP_COPY;
	break;
	case GXandInverted:
		mode = B_OP_SUBTRACT;
	break;
	//case GXnoop:
	case GXxor:
		mode = B_OP_ALPHA;
		func = B_ALPHA_COMPOSITE_SOURCE_IN;
	break;
	case GXor:
		mode = B_OP_ALPHA;
		func = B_ALPHA_COMPOSITE_SOURCE_OUT;
	break;
	//case GXnor:
	//case GXequiv:
	//case GXinvert:
	//case GXorReverse:
	//case GXcopyInverted:
	//case GXorInverted:
	//case GXnand:
	//case GXset:
	default:
		debugger("Unsupported GX mode!");
	}
	view->SetDrawingMode(mode);
	view->SetBlendingMode(B_PIXEL_ALPHA, func);

	view->SetHighColor(_x_pixel_to_rgb(gc->values.foreground));
	view->SetLowColor(_x_pixel_to_rgb(gc->values.background));
	view->SetPenSize(gc->values.line_width);

	cap_mode cap;
	switch (gc->values.cap_style) {
	case CapRound:
		cap = B_ROUND_CAP;
		break;
	case CapProjecting:
		cap = B_SQUARE_CAP;
		break;
	case CapNotLast:
	case CapButt:
		cap = B_ROUND_CAP;
		break;
	default:
		debugger("Unknown cap mode!");
		break;
	}

	join_mode join;
	switch (gc->values.join_style) {
	case JoinRound:
		join = B_ROUND_JOIN;
		break;
	case JoinBevel:
		join = B_BEVEL_JOIN;
		break;
	case JoinMiter:
		join = B_MITER_JOIN;
		break;
	default:
		debugger("Unknown join style!");
		break;
	}
	view->SetLineMode(cap, join);

	int32 fillRule = 0;
	switch (gc->values.fill_rule) {
	case EvenOddRule:
		fillRule = B_EVEN_ODD;
		break;
	case WindingRule:
		fillRule = B_NONZERO;
		break;
	default:
		debugger("Unknown fill rule!");
		break;
	}
	view->SetFillRule(fillRule);

	// TODO: use mask!
	if (gc->values.font) {
		BFont bfont = bfont_from_font(gc->values.font);
		view->SetFont(&bfont);
	}

	// TODO: use mask!
	switch (gc->values.subwindow_mode) {
	case ClipByChildren:
		view->SetFlags(view->Flags() & ~B_DRAW_ON_CHILDREN);
		break;
	case IncludeInferiors:
		view->SetFlags(view->Flags() | B_DRAW_ON_CHILDREN);
		break;
	default:
		debugger("Unsupported subwindow mode!");
	}

	// TODO: use mask!
	view->ConstrainClippingRegion(NULL);
	ClipMask* mask = gc_clip_mask(gc, false);
	if (mask && mask->region.CountRects()) {
		BRegion region = mask->region;
		region.OffsetBy(gc->values.clip_x_origin, gc->values.clip_y_origin);
		view->ConstrainClippingRegion(&region);
	}

	gc->dirty = False;
}
