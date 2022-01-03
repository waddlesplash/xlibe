/*
 * Copyright 2003, Shibukawa Yoshiki. All rights reserved.
 * Copyright 2021-2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <unistd.h>
#include <app/Application.h>
#include <interface/Screen.h>
#include <storage/AppFileInfo.h>
#include <private/app/AppMisc.h>

#include "Atom.h"
#include "Font.h"
#include "Color.h"
#include "Extension.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
}

static int sEnvDummy = setenv("DISPLAY", ":", 0);
static thread_id sBApplicationThread;

namespace {

class XlibApplication : public BApplication {
	Display* _display;

public:
	XlibApplication(Display* display, const char* signature);

protected:
	void ReadyToRun() override;
	void MessageReceived(BMessage* message) override;
};

XlibApplication::XlibApplication(Display* display, const char* signature)
	: BApplication(signature)
	, _display(display)
{
}

void
XlibApplication::ReadyToRun()
{
	char dummy[1];
	write(_display->conn_checker, dummy, 1);
}

void
XlibApplication::MessageReceived(BMessage* message)
{
	BApplication::MessageReceived(message);
}

} // namespace

static void
set_display(Display* dpy)
{
	static Depth dlist[1];
	static Visual vlist[1];
	static Screen slist[1];
	static char vstring[] = "Xlibe";

	display_mode mode;
	BScreen screen;
	screen.GetMode(&mode);

	memset(slist, 0, sizeof(Screen));

	dlist[0].depth		= 24;
	dlist[0].nvisuals	= 1;
	dlist[0].visuals	= vlist;

	vlist[0].ext_data     = NULL;
	vlist[0].visualid     = 0;
	vlist[0].c_class      = TrueColor;
	vlist[0].bits_per_rgb = 24;
	vlist[0].map_entries  = 256;
	_x_get_rgb_masks(&vlist[0]);

	const BRect screenFrame = screen.Frame();
	slist[0].width       = screenFrame.IntegerWidth();
	slist[0].height      = screenFrame.IntegerHeight();
	slist[0].mwidth      = screenFrame.Width() * 0.2646;
	slist[0].mheight     = screenFrame.Height() * 0.2646;
		// TODO: get real mm!
	slist[0].ndepths     = 1;
	slist[0].depths      = dlist;
	slist[0].root_depth  = dlist[0].depth;
	slist[0].root_visual = vlist;
	slist[0].default_gc  = NULL;
	slist[0].cmap        = XCreateColormap(dpy, None, &vlist[0], AllocNone);
	slist[0].white_pixel = _x_rgb_to_pixel(make_color(0xFF, 0xFF, 0xFF));
	slist[0].black_pixel = _x_rgb_to_pixel(make_color(0, 0, 0));

	slist[0].display = dpy;
	slist[0].root = None;

	dpy->ext_data            = NULL;
	dpy->proto_major_version = 11;
	dpy->proto_minor_version = 4;
	dpy->vendor              = vstring;
	dpy->display_name        = vstring;
	dpy->nscreens            = 1;
	dpy->screens             = slist;
	dpy->max_keycode         = 255;
	dpy->max_request_size	 = 4096;
	dpy->bigreq_size		 = dpy->max_request_size;
	dpy->qlen                = 0;
	dpy->request			 = 1;
	dpy->head = dpy->tail    = NULL;
	dpy->qfree               = NULL;

	dpy->free_funcs = (_XFreeFuncRec *)Xcalloc(1, sizeof(_XFreeFuncRec));
}

static int32
xmain(void* data)
{
	Display* display = (Display*)data;

	// Figure out what our signature is.
	BString signature;
	entry_ref appRef;
	status_t status = BPrivate::get_app_ref(BPrivate::current_team(), &appRef);
	if (status == B_OK) {
		BFile file(&appRef, O_RDONLY);
		BAppFileInfo info(&file);
		status = info.GetSignature(signature.LockBuffer(B_FILE_NAME_LENGTH));
		signature.UnlockBuffer();
	}
	if (status != B_OK || signature.IsEmpty())
		signature = "application/x-vnd.Xlibe-unknown";

	XlibApplication app(display, signature.String());
	app.Run();
	return 0;
}

extern "C" Display*
XOpenDisplay(const char* name)
{
	Display* display = new _XDisplay;
	memset(display, 0, sizeof(Display));

	int eventsPipe[2];
	pipe(eventsPipe);
	display->fd = eventsPipe[0];
	display->conn_checker = eventsPipe[1];

	sBApplicationThread = spawn_thread(xmain, "Xlib BApplication", B_NORMAL_PRIORITY, display);
	resume_thread(sBApplicationThread);

	// Wait for BApplication startup to complete.
	char dummy[1];
	read(display->fd, dummy, 1);

	set_display(display);
	_x_init_atoms();
	_x_init_font();
	return display;
}

extern "C" int
XCloseDisplay(Display *display)
{
	status_t result;
	be_app->PostMessage(B_QUIT_REQUESTED);
	wait_for_thread(sBApplicationThread, &result);

	_x_extensions_close(display);
	_x_finalize_font();

	close(display->fd);
	close(display->conn_checker);
	delete display;
	return 0;
}

static void
fill_visual_info(Visual* v, XVisualInfo* info)
{
	info->visual = v;
	info->visualid = info->visual->visualid;
	info->screen = 0;
	info->depth = info->visual->bits_per_rgb;
	info->c_class = info->visual->c_class;
	info->colormap_size = info->visual->map_entries;
	info->bits_per_rgb = info->visual->bits_per_rgb;
	info->red_mask = info->visual->red_mask;
	info->green_mask = info->visual->green_mask;
	info->blue_mask = info->visual->blue_mask;
}

extern "C" XVisualInfo*
XGetVisualInfo(Display *display, long vinfo_mask, XVisualInfo *vinfo_template, int *nitems_return)
{
	XVisualInfo* info = (XVisualInfo*)calloc(1, sizeof(XVisualInfo));
	fill_visual_info(DefaultVisual(display, 0), info);

	if (((vinfo_mask & VisualIDMask)
		 && (vinfo_template->visualid != info->visualid))
			|| ((vinfo_mask & VisualScreenMask)
				&& (vinfo_template->screen != info->screen))
			|| ((vinfo_mask & VisualDepthMask)
				&& (vinfo_template->depth != info->depth))
			|| ((vinfo_mask & VisualClassMask)
				&& (vinfo_template->c_class != info->c_class))
			|| ((vinfo_mask & VisualColormapSizeMask)
				&& (vinfo_template->colormap_size != info->colormap_size))
			|| ((vinfo_mask & VisualBitsPerRGBMask)
				&& (vinfo_template->bits_per_rgb != info->bits_per_rgb))
			|| ((vinfo_mask & VisualRedMaskMask)
				&& (vinfo_template->red_mask != info->red_mask))
			|| ((vinfo_mask & VisualGreenMaskMask)
				&& (vinfo_template->green_mask != info->green_mask))
			|| ((vinfo_mask & VisualBlueMaskMask)
				&& (vinfo_template->blue_mask != info->blue_mask))
			) {
		free(info);
		return NULL;
	}

	*nitems_return = 1;
	return info;
}

extern "C" int
XMatchVisualInfo(Display* display, int screen, int depth, int c_class, XVisualInfo* vinfo_return)
{
	if (screen >= display->nscreens)
		return 0;
	const Screen& scr = display->screens[screen];
	for (int i = 0; i < scr.ndepths; i++) {
		const Depth& dpth = scr.depths[i];
		if (dpth.depth != depth)
			continue;

		for (int j = 0; j < dpth.nvisuals; j++) {
			const Visual& vis = dpth.visuals[j];
			if (vis.c_class != c_class)
				continue;

			fill_visual_info((Visual*)&vis, vinfo_return);
			vinfo_return->screen = screen;
			return 1;
		}
	}
	return 0;
}

extern "C" int
XScreenNumberOfScreen(Screen* screen)
{
	for (int i = 0; i < screen->display->nscreens; i++) {
		if (&screen->display->screens[i] == screen)
			return i;
	}
	return -1;
}

extern "C" int
XFree(void *data)
{
	free(data);
	return 0;
}
