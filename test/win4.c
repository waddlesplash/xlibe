#include<stdio.h> 
#include<X11/Xlib.h> 
main() 
{ 
 Display *dsp;                            /* ディスプレイ構造体の宣言 */ 
 Window   win;                              /* ウィンドウ構造体の宣言 */ 
 int      i; 


 dsp = XOpenDisplay( NULL );                /* ディスプレイのオープン */ 


/* ウィンドウの生成 */ 
 win = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 800, 400, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 


 XMapWindow( dsp, win );                          /* ウィンドウの表示 */ 
 XFlush( dsp ); 


 for( i=0 ; i<256 ; i+=10 )     /* カラーマップの色を順に設定するループ */ 
   { 
    sleep(1);                               /* １秒間停止して色を表示 */ 
    XSetWindowBackground( dsp, win, i );   /* 背景色を色番号 i に設定 */ 
    XClearWindow( dsp, win );                   /* ウィンドウのクリア */ 
    XFlush( dsp ); 
   } 


 XCloseDisplay( dsp );                      /* ディスプレイのクローズ */ 
}
