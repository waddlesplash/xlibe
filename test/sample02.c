/* 0302.c ; make windows and map them */ 

#include <X11/Xlib.h>
#include <X11/Xutil.h> 
#include <stdio.h> 

int 
main() 
{ 
  Display *d; 
  Window w0, w1, w2, w3; 
  unsigned long black, white; 

  d = XOpenDisplay(NULL); 

  black = BlackPixel(d, 0); 
  white = WhitePixel(d, 0); 

  w0 = XCreateSimpleWindow(d, RootWindow(d,0), 100, 100, 
                           600, 400, 2, black, white); 
  w1 = XCreateSimpleWindow(d, w0,               50,  50, 
                           200, 150, 2, black, white); 
  w1 = XCreateSimpleWindow(d, w0,              125, 75, 
                           200, 150, 2, black, white); 
  w2 = XCreateSimpleWindow(d, w0,              200, 100, 
                           200, 150, 2, black, white); 
  w1 = XCreateSimpleWindow(d, w0,              275,  125, 
                           200, 150, 2, black, white); 
  w2 = XCreateSimpleWindow(d, w0,              350, 150, 
                           200, 150, 2, black, white); 
  w1 = XCreateSimpleWindow(d, w0,              425, 175, 
                           200, 150, 2, black, white);

  XMapWindow(d, w0); 
  XMapSubwindows(d, w0); 
  XFlush(d); 

  getchar(); 
  XCloseDisplay(d); 

  return 0;
} 
