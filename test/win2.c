#include<stdio.h> 
#include <X11/Xlib.h> 
main() 
{ 
 Display *dsp;                         /* ディスプレイ構造体の宣言 */ 
 Window   win;                         /* ウィンドウ構造体の宣言 */ 


 dsp = XOpenDisplay( NULL );           /* ディスプレイのオープン */ 


 /* ウィンドウの生成 */ 
 win = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 800, 400, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 


 XMapWindow( dsp, win );               /* ウィンドウの表示 */ 
 XFlush( dsp ); 


 getchar();                            /* リターンキーの入力まで停止する */ 
 XUnmapWindow( dsp, win );             /* ウィンドウを消す */ 
 XFlush( dsp ); 


 getchar();                            /* リターンキーの入力まで停止する */ 
 XMapWindow( dsp, win );               /* ウィンドウの表示 */ 
 XFlush( dsp ); 


 getchar();                            /* リターンキーの入力まで停止 */ 
 XCloseDisplay( dsp );                 /* ディスプレイのクローズ */ 
}
