#include<stdio.h> 
#include <X11/Xlib.h> 
main() 
{ 
 Display *dsp;                           /* ディスプレイ構造体の宣言 */ 
 Window   win1,win2;                     /* ウィンドウ構造体の宣言 */ 


 dsp = XOpenDisplay( NULL );             /* ディスプレイのオープン */ 


 /* ウィンドウ１の生成 */ 
 win1 = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 100, 800, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 
 /* ウィンドウ２の生成 */ 
 win2 = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 800, 100, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 


 XMapWindow( dsp, win1 );               /* ウィンドウ１の表示 */ 
 XFlush( dsp ); 
 getchar();                             /* リターンキーの入力まで停止 */ 


 XUnmapWindow( dsp, win1 );             /* ウィンドウ１を消す */ 
 XMapWindow( dsp, win2 );               /* ウィンドウ２の表示 */ 
 XFlush( dsp ); 
 getchar();                             /* リターンキーの入力まで停止 */ 


 XMapWindow( dsp, win1 );               /* ウィンドウ１の表示 */ 
 XFlush( dsp ); 
 getchar();                             /* リターンキーの入力まで停止 */ 


 XCloseDisplay( dsp );                  /* ディスプレイのクローズ */ 
}
