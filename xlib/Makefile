OBJS = Bell.o Color.o Debug.o Display.o Drawing.o Event.o Font.o FontList.o GC.o Pixmap.o XInnerWindow.o XWindow.o XApp.o

all: libX11.a

clean:
	rm *.o
	rm libX11.a

libX11.a: $(OBJS)
	ar rcs $@ $(OBJS)
	cp $@ ..

.cpp.o:
	gcc -c -g $< -I.. -I.

.c.o:
	gcc -c -g $< -I.. -I.
