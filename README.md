Xlibe
===========================

An Xlib compatibility layer implemented on top of the [Haiku](https://www.haiku-os.org/) API, in
order to run X11 applications on Haiku without an X server.

![Untitled](https://user-images.githubusercontent.com/2175324/165316265-712a2347-e63f-43f6-9424-658a11fd2b62.png)

Xlib's API is relatively low-level, but it is just high-level enough
that it can be emulated on top of a higher-level API like Haiku's.

At present, it provides enough functioning Xlib APIs for Cairo and GTK
to run with only occasional glitches. (Other X11 applications vary greatly
in terms of what does and does not work.)

Requirements
--------------------------
```
pkgman install devel:libiconv devel:kbproto devel:xproto
```

History
--------------------------
This project was originally based on an old experiment [found on SourceForge.JP](http://sourceforge.jp/projects/bexlib/),
written by ja6hfa (Shibukawa Yoshiki) and kazuyakt, which had only a few functions implemented,
but provided a basic structure and initial direction.
