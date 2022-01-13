/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */

#include <app/Application.h>
#include <app/Clipboard.h>

#include "Atom.h"
#include "Debug.h"
#include "Event.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#undef Data
}

static Window sCurrentSelectionRequestee = None;

static void
request_selection(Display* display, Window owner, Atom selection, Atom target)
{
	XEvent event = {};
	event.type = SelectionRequest;
	event.xselectionrequest.owner = owner;
	event.xselectionrequest.requestor = DefaultRootWindow(display);
	event.xselectionrequest.selection = selection;
	event.xselectionrequest.target = target;
	event.xselectionrequest.property = Atoms::CLIPBOARD;
	event.xselectionrequest.time = _x_current_time();
	_x_put_event(display, event);
}

extern "C" Window
XGetSelectionOwner(Display* display, Atom selection)
{
	if (selection == Atoms::CLIPBOARD) {
		if (sCurrentSelectionRequestee != None)
			return sCurrentSelectionRequestee;

		// Unless we are currently in the middle of updating the clipboard, return the root,
		// so that XGetSelectionOwner is always called when the clipboard is changed.
		return DefaultRootWindow(display);
	}

	// We don't support PRIMARY or SECONDARY selections.
	return None;
}

extern "C" int
XSetSelectionOwner(Display* display, Atom selection, Window owner, Time time)
{
	if (selection == Atoms::CLIPBOARD) {
		sCurrentSelectionRequestee = owner;
		request_selection(display, owner, selection, Atoms::TARGETS);
		return 0;
	}

	return BadImplementation;
}

extern "C" int
XConvertSelection(Display* display, Atom selection, Atom target,
	Atom property, Window requestor, Time time)
{
	UNIMPLEMENTED();
	return 0;
}

void
_x_handle_send_root_selection(Display* dpy, const XEvent& event)
{
	if (event.type == SelectionNotify) {
		// Nothing to do: this is handled through XSetWindowProperty directly.
		return;
	}

	UNIMPLEMENTED();
}

Status
_x_handle_set_clipboard(Display* dpy, Window w, Atom type, const unsigned char* data, int nelements)
{
	if (type == XA_ATOM) {
		// First pass: possible types are being sent.
		const Atom* atoms = (const Atom*)data;
		bool requested = false;
		for (int i = 0; i < nelements; i++) {
			// TODO: Handle more than just text!
			if (atoms[i] != Atoms::UTF8_STRING)
				continue;

			request_selection(dpy, sCurrentSelectionRequestee, Atoms::CLIPBOARD, atoms[i]);
			requested = true;
		}
		if (requested) {
			// Clear the clipboard as we are about to set it.
			be_clipboard->Lock();
			be_clipboard->Clear();
			be_clipboard->Commit();
			be_clipboard->Unlock();
		}

		// TODO: More properly, unset this once all properties have been set.
		sCurrentSelectionRequestee = None;
		return Success;
	}

	// Actual data is being sent. Let's handle it.
	if (type == Atoms::INCR) {
		// TODO: incremental data receive!
		UNIMPLEMENTED();
		return BadImplementation;
	}

	bool changed = false;
	be_clipboard->Lock();
	BMessage* clip = be_clipboard->Data();

	if (type == Atoms::UTF8_STRING) {
		clip->AddData("text/plain", B_MIME_TYPE, data, nelements);
		changed = true;
	}

	if (changed)
		be_clipboard->Commit();
	be_clipboard->Unlock();

	return Success;
}
