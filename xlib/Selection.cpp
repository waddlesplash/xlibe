/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#include "Selection.h"

#include <list>
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

static Atom sCurrentSelectionRequestProperty = None;
static Atom sCurrentSelectionRequestTarget = None;

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
	if (selection != Atoms::CLIPBOARD)
		return 0;

	// We only support requesting the selection from the root window.
	XEvent event = {};
	event.type = SelectionRequest;
	event.xselectionrequest.owner = DefaultRootWindow(display);
	event.xselectionrequest.requestor = requestor;
	event.xselectionrequest.selection = selection;
	event.xselectionrequest.target = target;
	event.xselectionrequest.property = property;
	event.xselectionrequest.time = time;
	_x_handle_send_root_selection(display, event);
	return 1;
}

void
_x_handle_send_root_selection(Display* dpy, const XEvent& event)
{
	if (event.type == SelectionNotify) {
		// Nothing to do: this is handled through XSetWindowProperty directly.
		return;
	} else if (event.type == SelectionRequest) {
		if (event.xselectionrequest.selection != Atoms::CLIPBOARD)
			return; // This should not happen, as XGetSelectionOwner returns None.

		sCurrentSelectionRequestProperty = event.xselectionrequest.property;
		sCurrentSelectionRequestTarget = event.xselectionrequest.target;

		XEvent sevent = {};
		sevent.type = SelectionNotify;
		sevent.xselection.requestor = event.xselectionrequest.requestor;
		sevent.xselection.selection = event.xselectionrequest.selection;
		sevent.xselection.target = event.xselectionrequest.target;
		sevent.xselection.property = event.xselectionrequest.property;
		sevent.xselection.time = _x_current_time();
		_x_put_event(dpy, sevent);

		return;
	}

	UNIMPLEMENTED();
}

bool
_x_handle_get_clipboard(Display* dpy, Window w, Atom property,
	Atom* actual_type_return, int* actual_format_return,
	unsigned long* nitems_return, unsigned char** prop_return)
{
	if (property != sCurrentSelectionRequestProperty)
		return false;

	BString requestTypeName; {
		char* atomName = XGetAtomName(dpy, sCurrentSelectionRequestTarget);
		requestTypeName = atomName;
		XFree(atomName);
	}

	// Some special cases.
	if (requestTypeName == "text/plain;charset=utf-8")
		sCurrentSelectionRequestTarget = Atoms::UTF8_STRING;

	be_clipboard->Lock();
	const BMessage* clip = be_clipboard->Data();

	switch (sCurrentSelectionRequestTarget) {
	case Atoms::TARGETS: {
		char* name;
		std::list<Atom> atoms;
		int32 i = 1;
		while (clip->GetInfo(B_MIME_DATA, i++, &name, NULL, NULL) == B_OK) {
			atoms.push_back(XInternAtom(dpy, name, False));

			if (strcmp(name, "text/plain") == 0)
				atoms.push_back(Atoms::UTF8_STRING);
		}

		Atom* atoms_return = (Atom*)malloc(sizeof(Atom) * atoms.size());
		std::copy(atoms.begin(), atoms.end(), atoms_return);

		*actual_type_return = XA_ATOM;
		*actual_format_return = 32;
		*nitems_return = atoms.size();
		*prop_return = (unsigned char*)atoms_return;
		break;
	}

	case XA_STRING:
	case Atoms::UTF8_STRING: {
		const char* text;
		ssize_t textLength;
		clip->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLength);

		char* textBuffer = (char*)calloc(sizeof(char), textLength + 1);
		memcpy(textBuffer, text, textLength);

		*actual_type_return = Atoms::UTF8_STRING;
		*actual_format_return = 8;
		*nitems_return = textLength;
		*prop_return = (unsigned char*)textBuffer;
		break;
	}

	// TODO: More types (esp. raw MIME fetches!)
	}

	be_clipboard->Unlock();
	sCurrentSelectionRequestProperty = None;
	return true;
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
