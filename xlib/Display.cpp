#include <string.h>
#include <Screen.h>
#include <iostream>
#include <add-ons/graphics/Accelerant.h>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

#include "XApp.h"
#include "FontList.h"

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

	BRect rect;
	display_mode mode;
	BScreen screen;
	screen.GetMode(&mode);

	memset(slist, 0, sizeof(Screen));

	dlist[0].depth = mode.space;
	dlist[0].nvisuals = 1;
	dlist[0].visuals  = vlist;

	vlist[0].ext_data     = NULL;
	vlist[0].visualid     = 0;
	vlist[0].c_class      = TrueColor;
	vlist[0].bits_per_rgb = 24;
	vlist[0].map_entries  = 256;
	vlist[0].red_mask     = 255;
	vlist[0].green_mask   = 255 << 8;
	vlist[0].blue_mask    = 255 << 16;
	rect = screen.Frame();
	slist[0].width       = static_cast<int>(rect.right - rect.left);
	slist[0].height      = static_cast<int>(rect.bottom - rect.top);
	slist[0].mwidth      = 260;
	slist[0].mheight     = 190;
	slist[0].ndepths     = 1;
	slist[0].depths      = dlist;
	slist[0].root_depth  = mode.space;
	slist[0].root_visual = vlist;
	slist[0].default_gc  = NULL;
	slist[0].cmap        = cmap;
	slist[0].white_pixel = 0xFFFFFFFFFF;
	slist[0].black_pixel = 0;

	slist[0].display = dpy;

	dpy->ext_data            = NULL;
	dpy->fd                  = 0;
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

extern "C" Display* XOpenDisplay(const char *name)
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

extern "C" int XCloseDisplay(Display *display) {
	be_app->PostMessage(B_QUIT_REQUESTED);
	status_t result;
	wait_for_thread(server_thread, &result);
	delete display;
	finalize_font();
	return 0;
}

int
XFree(void *data)
{
	free(data);
	return 0;
}

int
XSync(Display *display, Bool discard)
{
	// Nothing to do.
	return Success;
}

int
XNoOp(Display *display)
{
	return 0;
}
