Xlibe
===========================

An Xlib compatibility layer implemented on top of the Haiku API, in
order to run X11 applications on Haiku without an X server.

Xlib's API is relatively low-level, but it is just high-level enough
that it can be emulated on top of a higher-level API like Haiku's.

At present, it provides "most" commonly-used Xlib APIs, but many of them are
stubbed or incomplete implementations. (GTK, with some hacks, can compile, link,
and open a window before it runs in to missing functionality.)

History
--------------------------
This project was originally based on an old experiment [found on SourceForge.JP](http://sourceforge.jp/projects/bexlib/),
written by ja6hfa (Shibukawa Yoshiki) and kazuyakt, which had only a few functions implemented,
but provided a basic structure and initial direction.
