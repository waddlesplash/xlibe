#include<stdio.h> 
#include<X11/Xlib.h> 
main() 
{ 
 Display *dsp;                       /* ディスプレイ構造体の宣言 */ 
 Window   win, sub1, sub2, sub3;     /* ウィンドウ構造体の宣言 */ 
 Colormap cmap;                      /* カラーマップ構造体の宣言 */ 
 XColor   iro1, iro2, iro3, dummy;   /* カラー構造体の宣言 */ 


 dsp = XOpenDisplay( NULL );         /* ディスプレイのオープン */ 


 cmap = DefaultColormap( dsp, 0 );   /* カラーマップを得る */ 


 /* カラーマップから色を得る */ 
 XAllocNamedColor( dsp, cmap, "red", &iro1, &dummy ); 
 XAllocNamedColor( dsp, cmap, "green", &iro2, &dummy ); 
 XAllocNamedColor( dsp, cmap, "blue", &iro3, &dummy ); 


 /* ウィンドウの生成 */ 
 win = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 600, 50, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 
 sub1 = XCreateSimpleWindow( dsp, win, 
                            3, 3, 40, 40, 2, 
                            BlackPixel(dsp,0), iro1.pixel ); 
 sub2 = XCreateSimpleWindow( dsp, win, 
                            53, 3, 40, 40, 2, 
                            BlackPixel(dsp,0), iro2.pixel ); 
 sub3 = XCreateSimpleWindow( dsp, win, 
                            103, 3, 40, 40, 2, 
                            BlackPixel(dsp,0), iro3.pixel ); 


 XStoreName( dsp, win, "カラーボタン" );   /* タイトルの設定 */ 


 XMapWindow( dsp, win );             /* ウィンドウの表示 */ 
 XMapSubwindows( dsp, win );         /* サブウィンドウの表示 */ 
 XFlush( dsp ); 


 getchar();                          /* リターンキーの入力まで停止する */ 
 XCloseDisplay( dsp );               /* ディスプレイのクローズ */ 
}
