SUBDIRS = xlib test

all: xlib test

$(SUBDIRS)::
	make -C $@
