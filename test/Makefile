COPT = ../libX11.a -lbe -lroot

all: test

test:	kt-clock kt-draw xlib-1 xlib-3 xhello button diamond sample01 sample02 win1 win2 win3 win4 win5 win8 win9 win11 win12 win13 xprog2_3

xprog2_3: xprog2_3.o
	g++ $< -o $@ $(COPT)

kt-clock: kt-clock.o
	g++ $< -o $@ $(COPT)

kt-draw: kt-draw.o
	g++ $< -o $@ $(COPT)

xlib-1: xlib-1.o
	g++ $< -o $@ $(COPT)

xlib-3: xlib-3.o
	g++ $< -o $@ $(COPT)

xhello: xhello.o
	g++ $< -o $@ $(COPT)

button: button.o
	g++ $< -o $@ $(COPT)

diamond: diamond.o
	g++ $< -o $@ $(COPT)

sample01: sample01.o
	g++ $< -o $@ $(COPT)

sample02: sample02.o
	g++ $< -o $@ $(COPT)

win1: win1.o
	g++ $< -o $@ $(COPT)

win2: win2.o
	g++ $< -o $@ $(COPT)

win3: win3.o
	g++ $< -o $@ $(COPT)

win4: win4.o
	g++ $< -o $@ $(COPT)

win5: win5.o
	g++ $< -o $@ $(COPT)

win8: win8.o
	g++ $< -o $@ $(COPT)

win9: win9.o
	g++ $< -o $@ $(COPT)

win11: win11.o
	g++ $< -o $@ $(COPT)

win13: win13.o
	g++ $< -o $@ $(COPT)

win12: win12.o
	g++ $< -o $@ $(COPT)

.cpp.o:
	gcc -c -g $< -I.. -I.

.c.o:
	gcc -c -g $< -I.. -I.
