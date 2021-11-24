extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

int XLookupString(XKeyEvent* event_struct, char* buffer_return, int bytes_buffer,
		KeySym* keysym_return, XComposeStatus *status_in_out)
{
	*buffer_return = event_struct->keycode;
	*keysym_return = -1; // FIXME!
	return 1;
}
