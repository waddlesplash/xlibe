#include<stdio.h> 
#include<X11/Xlib.h> 
main() 
{ 
 Display *dsp;                       /* ディスプレイ構造体の宣言 */ 
 Window   win, sub[12];              /* ウィンドウ構造体の宣言 */ 
 Colormap cmap;                      /* カラーマップの宣言 */ 
 XColor   iro[12], dummy;            /* カラー構造体の宣言 */ 
 static char *cname[] = { 
                         "red", "green", "blue", "yellow", 
                         "pink", "sky blue", "orange", "violet", 
                         "dark green", "gray", "black", "white" 
                        }; 
 int  i; 


 dsp = XOpenDisplay( NULL );         /* ディスプレイのオープン */ 


 cmap = DefaultColormap( dsp, 0 );   /* カラーマップを得る */ 


 /* カラーマップから色を得る */ 
 for( i=0 ; i<12 ; i++ ) 
   XAllocNamedColor( dsp, cmap, cname[i], &iro[i], &dummy ); 


 /* ウィンドウの生成 */ 
 win = XCreateSimpleWindow( dsp, DefaultRootWindow(dsp), 
                            0, 0, 600, 50, 1, 
                            BlackPixel(dsp,0), WhitePixel(dsp,0) ); 
 for( i=0 ; i<12 ; i++ ) 
   sub[i] = XCreateSimpleWindow( dsp, win, 
                            3+50*i, 3, 40, 40, 2, 
                            BlackPixel(dsp,0), iro[i].pixel ); 


 XStoreName( dsp, win, "カラーボタン" );   /* タイトルの設定 */ 


 XMapWindow( dsp, win );             /* ウィンドウの表示 */ 
 XMapSubwindows( dsp, win );         /* サブウィンドウの表示 */ 
 XFlush( dsp ); 


 getchar();                          /* リターンキーの入力まで停止する */ 
 XCloseDisplay( dsp );               /* ディスプレイのクローズ */ 
}
