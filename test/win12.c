#include<stdio.h> 
#include<X11/Xlib.h> 
main() 
{ 
 Display *dsp;                       /* ディスプレイ構造体の宣言 */ 
 Window   win;                       /* ウィンドウ構造体の宣言 */ 
 GC   gc;                            /* ＧＣ構造体の宣言 */ 
 int  x, y, width, height; 
 int  i; 


 dsp = XOpenDisplay( NULL );         /* ディスプレイのオープン */ 


 /* ウィンドウの生成 */ 
 win = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 320, 320, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 
 /* ＧＣの生成 */ 
 gc = XCreateGC( dsp, win, NULL , NULL ); 


 XStoreName( dsp, win, "Color Map" ); /* タイトルの設定 */ 


 XMapWindow( dsp, win );             /* ウィンドウの表示 */ 
 XFlush( dsp ); 
 getchar();                          /* リターンキーの入力まで停止する */ 


 /* 図形を描く */ 
 width=16;  height=16;  i=0; 
 for( y=2 ; y<320 ; y+=20 ) 
   { 
    for( x=2 ; x<320 ; x+=20 ) 
      { 
       XSetForeground( dsp, gc, i );    /* 色の設定 */ 
       XFillRectangle( dsp, win, gc, x, y, width, height ); /* 長方形 */ 
       i++; 
      } 
   } 


 XFlush( dsp ); 
 getchar();                          /* リターンキーの入力まで停止する */ 


 XCloseDisplay( dsp );               /* ディスプレイのクローズ */ 
}
