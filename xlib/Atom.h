/*
 * Copyright 2021, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xatom.h>
}

namespace Atoms {

enum {
	_xa_last_predefined = XA_LAST_PREDEFINED,

#define ATOM(NAME) NAME,
#include "atoms.h"
#undef ATOM

	_predefined_atom_count,
}; // enum
static_assert(Atoms::_predefined_atom_count < 4096);

} // namespace Atoms

void _x_init_atoms();
