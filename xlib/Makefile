OBJS = Bell.o Color.o Display.o Drawing.o Event.o Font.o FontList.o GC.o XInnerWindow.o XWindow.o XApp.o

all: libX11.a

libX11.a: $(OBJS)
	ar rcs $@ $(OBJS)
	cp $@ ..

.cpp.o:
	gcc -c -g $< -I.. -I.

.c.o:
	gcc -c -g $< -I.. -I.
