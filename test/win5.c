#include<stdio.h> 
#include<X11/Xlib.h> 
main() 
{ 
 Display *dsp;                            /* ディスプレイ構造体の宣言 */ 
 Window   win;                              /* ウィンドウ構造体の宣言 */ 
 Colormap cmap;                                 /* カラーマップの宣言 */ 
 XColor   aka;                                  /* カラー構造体の宣言 */ 


 dsp = XOpenDisplay( NULL );                /* ディスプレイのオープン */ 


 cmap = DefaultColormap( dsp, 0 );              /* カラーマップを得る */ 


 aka.red=65535;  aka.green=0;  aka.blue=0;            /* 赤色のデータ */ 
 XAllocColor( dsp, cmap, &aka );          /* カラーマップから色を得る */ 


/* ウィンドウの生成 */ 
 win = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 800, 400, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 


 XMapWindow( dsp, win );                          /* ウィンドウの表示 */ 
 XFlush( dsp ); 


 getchar();                             /* リターンキーの入力まで停止 */ 
 XSetWindowBackground( dsp, win, aka.pixel );         /* 背景色の設定 */ 
 XClearWindow( dsp, win );                      /* ウィンドウのクリア */ 
 XFlush( dsp ); 


 getchar();                             /* リターンキーの入力まで停止 */ 
 XCloseDisplay( dsp );                      /* ディスプレイのクローズ */ 
}
