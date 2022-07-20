/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#include "Settings.h"

#include <support/DataIO.h>
#include <support/String.h>

#include <interface/Font.h>

#include "Atom.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
}

namespace Xlibe {

class XSettings final {
	BMallocIO _settings;
	CARD32 _nsettings = 0;

public:
	enum SettingType {
		XSettingsTypeInteger	= 0,
		XSettingsTypeString		= 1,
		XSettingsTypeColor		= 2,
	};

public:
	XSettings()
	{
		// header
		_Append(CARD8(LSBFirst)); /* byte order */
		for (int i = 0; i < 3; i++)
			_Append(CARD8(0)); /* unused */
		_Append(CARD32(1)); /* SERIAL */
		_Append(CARD32(0)); /* N_SETTINGS (placeholder) */
	}

	void
	AddInteger(const BString& name, int32 value)
	{
		_AppendHeader(XSettingsTypeInteger, name);
		_Append(CARD32(value));
	}

	void
	AddString(const BString& name, const BString& setting)
	{
		_AppendHeader(XSettingsTypeString, name);
		_Append(setting);
	}

	void
	Get(void*& data, size_t& length)
	{
		_settings.WriteAt(8, &_nsettings, sizeof(CARD32));

		length = _settings.BufferLength();
		data = malloc(length);
		memcpy(data, _settings.Buffer(), length);
	}

private:
	static int
	_Padding(int len, int to = 4)
	{
		return ((len + to - 1) & (~(to - 1))) - len;
	}

	template<typename T>
	void
	_Append(T value)
	{
		_settings.Write(&value, sizeof(T));
	}

	void
	_Append(const BString& string, int length_len = 4)
	{
		const int32 length = string.Length();
		if (length_len == 4)
			_Append(CARD32(length));
		else if (length_len == 2)
			_Append(CARD16(length));
		else
			debugger("XSettings: invalid length_len");

		_settings.Write(string.String(), length);

		const CARD64 pad = 0;
		_settings.Write(&pad, _Padding(length));
	}

	void
	_AppendHeader(SettingType type, const BString& name)
	{
		_nsettings++;

		_Append(CARD8(type));
		_Append(CARD8(0)); // unused
		_Append(name, 2);
		_Append(CARD32(0)); // last-change-serial
	}
};

} // namespace Xlibe

int
_x_handle_get_settings(Display* dpy, Window w,
	Atom* actual_type_return, int* actual_format_return,
	unsigned long* nitems_return, unsigned char** prop_return)
{
	if (w != DefaultRootWindow(dpy))
		return BadImplementation;

	Xlibe::XSettings settings; {
		settings.AddString("Net/IconThemeName", "haiku");
		settings.AddInteger("Xft/DPI", (be_plain_font->Size() / 12.0f) * 96 * 1024);

		font_family family;
		be_plain_font->GetFamilyAndStyle(&family, NULL);
		settings.AddString("Gtk/FontName", family);
	}

	void* data; size_t length;
	settings.Get(data, length);

	*actual_type_return = Atoms::_XSETTINGS_SETTINGS;
	*actual_format_return = 8;
	*prop_return = (unsigned char*)data;
	*nitems_return = length;

	return 0;
}
