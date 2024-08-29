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
	gc->values.graphics_exposures = True;
	gc->values.clip_x_origin = gc->values.clip_y_origin = 0;
	gc->values.clip_mask = None;
	gc->dirty = 0;
	XChangeGC(display, gc, mask, gc_values);
	return gc;
}

extern "C" int
XFreeGC(Display* display, GC gc)
{
	if (gc) {
		XDrawable* drawable = Drawables::get(gc->gid);
		if (drawable && drawable->last_gc == gc)
			drawable->last_gc = NULL;

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
	gc->dirty |= mask;
	return 0;
}

static int
_x_compare_gcs(GC first, GC second)
{
	if (first == second)
		return 0;
	if (first == NULL || second == NULL)
		return INT32_MAX;

	int mask = 0;
	if (first->values.function != second->values.function)
		mask |= GCFunction;
	if (first->values.plane_mask != second->values.plane_mask)
		mask |= GCPlaneMask;
	if (first->values.foreground != second->values.foreground)
		mask |= GCForeground;
	if (first->values.background != second->values.background)
		mask |= GCBackground;
	if (first->values.line_width != second->values.line_width)
		mask |= GCLineWidth;
	if (first->values.line_style != second->values.line_style)
		mask |= GCLineStyle;
	if (first->values.cap_style != second->values.cap_style)
		mask |= GCCapStyle;
	if (first->values.join_style != second->values.join_style)
		mask |= GCJoinStyle;
	if (first->values.fill_style != second->values.fill_style)
		mask |= GCFillStyle;
	if (first->values.fill_rule != second->values.fill_rule)
		mask |= GCFillRule;
	if (first->values.arc_mode != second->values.arc_mode)
		mask |= GCArcMode;
	if (first->values.tile != second->values.tile)
		mask |= GCTile;
	if (first->values.stipple != second->values.stipple)
		mask |= GCStipple;
	if (first->values.ts_x_origin != second->values.ts_x_origin)
		mask |= GCTileStipXOrigin;
	if (first->values.ts_y_origin != second->values.ts_y_origin)
		mask |= GCTileStipYOrigin;
	if (first->values.font != second->values.font)
		mask |= GCFont;
	if (first->values.subwindow_mode != second->values.subwindow_mode)
		mask |= GCSubwindowMode;
	if (first->values.graphics_exposures != second->values.graphics_exposures)
		mask |= GCGraphicsExposures;
	if (first->values.clip_x_origin != second->values.clip_x_origin)
		mask |= GCClipXOrigin;
	if (first->values.clip_y_origin != second->values.clip_y_origin)
		mask |= GCClipYOrigin;
	if (first->values.clip_mask != second->values.clip_mask)
		mask |= GCClipMask;
	if (first->values.dash_offset != second->values.dash_offset)
		mask |= GCDashOffset;
#if 0
	// TODO
	if (first->values.dash_list != second->values.dash_list)
		mask |= GCDashList;
#endif

	return mask;
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
		dest->dirty |= GCClipMask;
	}
	return 0;
}

extern "C" int
XSetFunction(Display *display, GC gc, int function)
{
	gc->values.function = function;
	gc->dirty |= GCFunction;
	return 0;
}

extern "C" int
XSetForeground(Display *display, GC gc, unsigned long color)
{
	gc->values.foreground = color;
	gc->dirty |= GCForeground;
	return 0;
}

extern "C" int
XSetBackground(Display *display, GC gc, unsigned long color)
{
	gc->values.background = color;
	gc->dirty |= GCBackground;
	return 0;
}

extern "C" int
XSetState(Display *display, GC gc, unsigned long foreground,
	unsigned long background, int function, unsigned long plane_mask)
{
	gc->values.foreground = foreground;
	gc->values.background = background;
	gc->values.function = function;
	gc->values.plane_mask = plane_mask;
	gc->dirty |= GCForeground | GCBackground | GCFunction | GCPlaneMask;
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
	gc->dirty |= GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle;
	return 0;
}

extern "C" int
XSetFillStyle(Display* display, GC gc, int fill_style)
{
	gc->values.fill_style = fill_style;
	gc->dirty |= GCFillStyle;
	return 0;
}

extern "C" int
XSetFillRule(Display* display, GC gc, int fill_rule)
{
	gc->values.fill_rule = fill_rule;
	gc->dirty |= GCFillRule;
	return 0;
}

extern "C" int
XSetArcMode(Display* display, GC gc, int arc_mode)
{
	gc->values.arc_mode = arc_mode;
	gc->dirty |= GCArcMode;
	return 0;
}

extern "C" int
XSetFont(Display *display, GC gc, Font font)
{
	gc->values.font = font;
	gc->dirty |= GCFont;
	return 0;
}

extern "C" int
XSetSubwindowMode(Display *display, GC gc, int subwindow_mode)
{
	gc->values.subwindow_mode = subwindow_mode;
	gc->dirty |= GCSubwindowMode;
	return 0;
}

extern "C" int
XSetClipOrigin(Display *display, GC gc, int clip_x_origin, int clip_y_origin)
{
	gc->values.clip_x_origin = clip_x_origin;
	gc->values.clip_y_origin = clip_y_origin;
	gc->dirty |= GCClipXOrigin | GCClipYOrigin;
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
	gc->dirty |= GCClipMask;
	return Success;
}

extern "C" int
XSetClipRectangles(Display *display, GC gc, int clip_x_origin, int clip_y_origin,
	XRectangle* rect, int count, int ordering)
{
	XSetClipOrigin(display, gc, clip_x_origin, clip_y_origin);

	ClipMask* mask = gc_clip_mask(gc);
	mask->region.MakeEmpty();
	for (int i = 0; i < count; i++)
		XUnionRectWithRegion(&rect[i], (Region)&mask->region, (Region)&mask->region);

	gc->dirty |= GCClipMask;
	return Success;
}

extern "C" Status
XSetClipMask(Display* display, GC gc, Pixmap pixmap)
{
	if (pixmap == None) {
		ClipMask* mask = gc_clip_mask(gc, false);
		if (!mask)
			return Success;

		mask->region.MakeEmpty();
		gc->dirty |= GCClipMask;
		return Success;
	}

	XPixmap* pxm = Drawables::get_pixmap(pixmap);
	if (!pxm)
		return BadPixmap;

	ClipMask* mask = gc_clip_mask(gc);
	mask->region.Set(pxm->offscreen()->Bounds());

	// TODO: Actually use the pixmap for clipping!
	UNIMPLEMENTED();

	gc->dirty |= GCClipMask;
	return Success;
}

bool
_x_gc_has_clipping(GC gc)
{
	ClipMask* mask = gc_clip_mask(gc, false);
	if (!mask)
		return false;

	return mask->region.CountRects() > 0;
}

extern "C" Status
XSetDashes(Display *display, GC gc, int dash_offset, const char *dash_list, int n)
{
	// Not supported.
	return BadImplementation;
}

void
_x_check_gc(XDrawable* drawable, GC gc)
{
	if (!gc) {
		debugger("Need default GC!");
		return;
	}

	if (gc->gid != drawable->id()) {
		XDrawable* previousDrawable = Drawables::get(gc->gid);
		if (previousDrawable && previousDrawable->last_gc == gc)
			previousDrawable->last_gc = NULL;

		if (drawable->last_gc != gc)
			gc->dirty = _x_compare_gcs(gc, drawable->last_gc);

		gc->gid = drawable->id();
	}
	if (!gc->dirty)
		return;
	drawable->last_gc = gc;

	BView* view = drawable->view();

	if (gc->dirty & GCFunction) {
		drawing_mode mode;
		alpha_function func = B_ALPHA_OVERLAY;
		switch (gc->values.function) {
		//case GXclear:	break;
		case GXand:
			mode = B_OP_BLEND;
		break;
		//case GXandReverse: break;
		case GXcopy:
			mode = B_OP_COPY;
		break;
		case GXandInverted:
			mode = B_OP_SUBTRACT;
		break;
		//case GXnoop: break;
		case GXxor:
			mode = B_OP_ALPHA;
			func = B_ALPHA_COMPOSITE_SOURCE_IN;
		break;
		case GXor:
			mode = B_OP_ALPHA;
			func = B_ALPHA_COMPOSITE_SOURCE_OUT;
		break;
		//case GXnor: break;
		//case GXequiv: break;
		case GXinvert:
			mode = B_OP_INVERT;
		break;
		//case GXorReverse: break;
		//case GXcopyInverted: break;
		//case GXorInverted: break;
		//case GXnand: break;
		//case GXset: break;
		default:
			debugger("Unsupported GX mode!");
		}
		view->SetDrawingMode(mode);
		view->SetBlendingMode(B_PIXEL_ALPHA, func);
	}

	if (gc->dirty & GCForeground)
		view->SetHighColor(_x_pixel_to_rgb(gc->values.foreground));
	if (gc->dirty & GCBackground)
		view->SetLowColor(_x_pixel_to_rgb(gc->values.background));
	if (gc->dirty & GCLineWidth)
		view->SetPenSize(gc->values.line_width);

	if (gc->dirty & (GCCapStyle | GCJoinStyle)) {
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
	}

	if (gc->dirty & GCFillRule) {
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
	}

	if ((gc->dirty & GCFont) && gc->values.font) {
		BFont bfont = _bfont_from_font(gc->values.font);
		view->SetFont(&bfont);
	}

	if (gc->dirty & GCSubwindowMode) {
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
	}

	if (gc->dirty & (GCClipMask | GCClipXOrigin | GCClipYOrigin)) {
		view->ConstrainClippingRegion(NULL);
		ClipMask* mask = gc_clip_mask(gc, false);
		if (mask && mask->region.CountRects() > 0) {
			BRegion region = mask->region;
			region.OffsetBy(gc->values.clip_x_origin, gc->values.clip_y_origin);
			view->ConstrainClippingRegion(&region);
		}
	}

	gc->dirty = 0;
}
