#include<stdio.h> 
#include<X11/Xlib.h> 
main() 
{ 
 Display *dsp;                       /* ディスプレイ構造体の宣言 */ 
 Window   win;                       /* ウィンドウ構造体の宣言 */ 
 Colormap cmap;                      /* カラーマップの宣言 */ 
 XColor  aka, ao, ki, midori, dummy; /* カラー構造体の宣言 */ 
 GC   gc;                            /* ＧＣ構造体の宣言 */ 
 int  x, y, width, height; 


 dsp = XOpenDisplay( NULL );         /* ディスプレイのオープン */ 


 cmap = DefaultColormap( dsp, 0 );   /* カラーマップを得る */ 


 /* カラーマップから色を得る */ 
 XAllocNamedColor( dsp, cmap, "red", &aka, &dummy ); 
 XAllocNamedColor( dsp, cmap, "blue", &ao, &dummy ); 
 XAllocNamedColor( dsp, cmap, "yellow", &ki, &dummy ); 
 XAllocNamedColor( dsp, cmap, "green", &midori, &dummy ); 


 /* ウィンドウの生成 */ 
 win = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 600, 400, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 
 /* ＧＣの生成 */ 
 gc = XCreateGC( dsp, win, NULL, NULL ); 


 XMapWindow( dsp, win );             /* ウィンドウの表示 */ 
 XFlush( dsp ); 
 getchar();                          /* リターンキーの入力まで停止する */ 


 /* 図形を描く */ 
 XSetForeground( dsp, gc, midori.pixel );    /* 色の設定 */ 
 x=100; y=50; width=40; height=250; 
 XFillRectangle( dsp, win, gc, x, y, width, height);  /* 長方形 */ 
 XFlush( dsp ); 
 getchar();                          /* リターンキーの入力まで停止する */ 


 XSetForeground( dsp, gc, ao.pixel );    /* 色の設定 */ 
 x=250; y=200; width=250; height=150; 
 XFillRectangle( dsp, win, gc, x, y, width, height);  /* 長方形 */ 
 XFlush( dsp ); 
 getchar();                          /* リターンキーの入力まで停止する */ 


 XSetForeground( dsp, gc, ki.pixel );    /* 色の設定 */ 
 x=350; y=80; width=200; height=200; 
 XFillArc( dsp, win, gc, x, y, width, height, 0, 360*64 );  /* 円 */ 
 XFlush( dsp ); 
 getchar();                          /* リターンキーの入力まで停止する */ 


 XSetForeground( dsp, gc, aka.pixel );    /* 色の設定 */ 
 x=50; y=50; width=500; height=250; 
 XDrawLine( dsp, win, gc, x, y, x+width, y+height );  /* 直線 */ 
 XFlush( dsp ); 
 getchar();                          /* リターンキーの入力まで停止する */ 


 XCloseDisplay( dsp );               /* ディスプレイのクローズ */ 
}
