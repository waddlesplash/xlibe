#include <string.h>
#include <unistd.h>
#include <interface/Screen.h>

#include "XApp.h"
#include "Font.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
}

static int sEnvDummy = setenv("DISPLAY", ":", 0);

thread_id server_thread;
thread_id main_thread;

static void
set_display(Display* dpy)
{
	static Depth dlist[1];
	static Visual vlist[1];
	static Screen slist[1];
	static char vstring[] = "libB11";
	Colormap cmap = 0;

	int eventsPipe[2];
	pipe(eventsPipe);

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
	vlist[0].red_mask     = 255;
	vlist[0].green_mask   = 255 << 8;
	vlist[0].blue_mask    = 255 << 16;

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
	slist[0].cmap        = cmap;
	slist[0].white_pixel = 0xFFFFFF;
	slist[0].black_pixel = 0;

	slist[0].display = dpy;
	slist[0].root = None;

	dpy->ext_data            = NULL;
	dpy->fd                  = eventsPipe[0];
	dpy->conn_checker		 = eventsPipe[1];
	dpy->proto_major_version = 11;
	dpy->proto_minor_version = 4;
	dpy->vendor              = vstring;
	dpy->display_name        = vstring;
	dpy->nscreens            = 1;
	dpy->screens             = slist;
	dpy->max_keycode         = 255;
	dpy->qlen                = 0;
	dpy->head = dpy->tail    = NULL;
	dpy->qfree               = NULL;

	dpy->free_funcs = (_XFreeFuncRec *)Xcalloc(1, sizeof(_XFreeFuncRec));
}

int32 xmain(void* data)
{
	char sig[50];
	thread_info info;
	strcpy(sig, "application/x-");
	get_thread_info(find_thread(NULL), &info);
	strcat(sig, info.name);
	XApp myapp(sig);
	myapp.Run();
	return 0;
};

extern "C" Display*
XOpenDisplay(const char *name)
{
	Display* display = new _XDisplay;
	memset(display, 0, sizeof(Display));
	main_thread = find_thread(NULL);
	thread_info info;
	get_thread_info(main_thread, &info);
	rename_thread(main_thread, "X Server");
	server_thread = spawn_thread(xmain, info.name, B_NORMAL_PRIORITY, 0);
	resume_thread(server_thread);
	suspend_thread(main_thread);
	init_font();
	set_display(display);
	return display;
}

extern "C" int
XCloseDisplay(Display *display)
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	status_t result;
	wait_for_thread(server_thread, &result);
	close(display->fd);
	close(display->conn_checker);
	delete display;
	finalize_font();
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
XConnectionNumber(Display* display)
{
	return ConnectionNumber(display);
}

extern "C" char*
XDisplayName(const char* string)
{
	return "Haiku";
}

extern "C" int
XDefaultScreen(Display* display)
{
	return DefaultScreen(display);
}

extern "C" int
XScreenCount(Display* display)
{
	return ScreenCount(display);
}

extern "C" Screen*
XScreenOfDisplay(Display* display, int screen)
{
	return ScreenOfDisplay(display, screen);
}

extern "C" Window
XDefaultRootWindow(Display *display)
{
	return DefaultRootWindow(display);
}

extern "C" Window
XRootWindow(Display *display, int screen_number)
{
	return RootWindow(display, screen_number);
}

extern "C" int
XDisplayWidth(Display *display, int screen_number)
{
	return DisplayWidth(display, screen_number);
}

extern "C" int
XDisplayHeight(Display *display, int screen_number)
{
	return DisplayHeight(display, screen_number);
}

extern "C" long
XMaxRequestSize(Display *display)
{
	// Arbitrary.
	return (4096 * 4);
}

extern "C" int
XFree(void *data)
{
	free(data);
	return 0;
}

extern "C" int
XNoOp(Display *display)
{
	return 0;
}
