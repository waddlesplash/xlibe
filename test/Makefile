all: test

test:	xhello button diamond

xhello: xhello.o
	g++ xhello.o ../libX11.a -lbe -lroot

button: button.o
	g++ button.o ../libX11.a -lbe -lroot

diamond: diamond.o
	g++ diamond.o ../libX11.a -lbe -lroot

.cpp.o:
	gcc -c -g $< -I.. -I.

.c.o:
	gcc -c -g $< -I.. -I.
