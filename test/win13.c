#include<stdio.h> 
#include<X11/Xlib.h> 
main() 
{ 
 Display *dsp;                       /* ディスプレイ構造体の宣言 */ 
 Window   win;                       /* ウィンドウ構造体の宣言 */ 
 Colormap cmap;                      /* カラーマップの宣言 */ 
 XColor   yama, hosi, sora[64];      /* カラー構造体の宣言 */ 
 GC   gc;                            /* ＧＣ構造体の宣言 */ 
 int x, y, width, height; 
 int m; 


 dsp = XOpenDisplay( NULL );         /* ディスプレイのオープン */ 


 cmap = DefaultColormap( dsp, 0 );   /* カラーマップを得る */ 


 /* カラーマップから色を得る */ 
 yama.red=0;  yama.green=20000;  yama.blue=0; 
 XAllocColor( dsp, cmap, &yama ); 
 hosi.red=65000;  hosi.green=65000;  hosi.blue=0; 
 XAllocColor( dsp, cmap, &hosi ); 
 for( m=0 ; m<64 ; m++ ) 
   { 
    sora[m].red=m*1000; 
    sora[m].green=30000-m*(64-m)*15; 
    sora[m].blue=64000-m*1000; 
    XAllocColor( dsp, cmap, &sora[m] ); 
   } 


 /* ウィンドウの生成 */ 
 win = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 600, 400, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 
 /* ＧＣの生成 */ 
 gc = XCreateGC( dsp, win, NULL , NULL ); 


 XStoreName( dsp, win, "夕暮れ" );  /* タイトルの設定 */ 


 XMapWindow( dsp, win );             /* ウィンドウの表示 */ 
 XFlush( dsp ); 
 getchar();                          /* リターンキーの入力まで停止する */ 


 /* 図形を描く */ 
 XSetForeground( dsp, gc, yama.pixel );  /* 色の設定 */ 
 x=0; y=320; width=600; height=80; 
 XFillRectangle( dsp, win, gc, x, y, width, height ); /* 長方形 */ 


 for( m=0 ; m<64 ; m++ ) 
    { 
     XSetForeground( dsp, gc, sora[m].pixel );  /* 色の設定 */ 
     x=0; y=m*5; width=600; height=5; 
     XFillRectangle( dsp, win, gc, x, y, width, height ); /* 長方形 */ 
    } 


 XSetForeground( dsp, gc, hosi.pixel );  /* 色の設定 */ 
 XDrawPoint( dsp, win, gc,  30, 55 ); 
 XDrawPoint( dsp, win, gc, 100, 15 ); 
 XDrawPoint( dsp, win, gc, 200, 10 ); 
 XDrawPoint( dsp, win, gc, 250, 30 ); 
 XDrawPoint( dsp, win, gc, 340, 20 ); 
 XDrawPoint( dsp, win, gc, 450, 40 ); 
 XDrawPoint( dsp, win, gc, 510, 25 ); 
 XDrawPoint( dsp, win, gc, 570, 45 ); 
 XFlush( dsp ); 


 getchar();                          /* リターンキーの入力まで停止する */ 


 XCloseDisplay( dsp );               /* ディスプレイのクローズ */ 
}
