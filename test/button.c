#include <X11/Xlib.h> 
#include <stdio.h>

main() 
{ 
  Display       *d;           /*ディスプレイ構造体へのポインタ*/ 
  Window        w, wb;        /*ウインドウのID*/ 
  XEvent        event; 
  Font          f;            /*フォントID */ 
  GC            gc; 

  d = XOpenDisplay( NULL );  /*ディスプレイをオープン*/ 
        
  /* ( 260, 160 ) ピクセルのウインドウを画面左上から( 180, 50 ) 
     のところに作成 */ 
  w = XCreateSimpleWindow( d, RootWindow( d, 0 ), 
                          180, 50, 
                          260, 160, 
                          1, WhitePixel(d, 0), BlackPixel(d, 0) ); 
  XSelectInput( d, w, ExposureMask|ButtonPressMask ); 

  /*ボタン(というか、子ウインドウ)を作成*/ 
  wb = XCreateSimpleWindow( d, w, 
                           20, 20, 
                           100, 20, 
                           1, WhitePixel(d, 0), BlackPixel(d, 0) ); 

  XSelectInput( d, wb, ExposureMask|ButtonPressMask ); 

  gc = XCreateGC( d, w, 0, 0 );  /*GC作成*/ 
  f = XLoadFont( d, "fixed" );   /*フォントをロード*/ 
  XSetFont( d, gc, f );          /*GCにそのフォントをセット*/ 

  XMapWindow( d, w );  /*作成したウインドウを画面に表示する*/ 
  XMapWindow( d, wb ); /*作成した子ウインドウを画面に表示する*/ 
  XFlush( d );         /*バッファにたまってるX命令を全部Xサーバーに送る*/ 

  while( 1 ){ 
    XNextEvent( d, &event ); 
    switch( event.type ){ 

    case Expose: 
      if ( event.xany.window == wb ){ 
        XSetForeground( d, gc, BlackPixel(d, 0) ); 
        XFillRectangle( d, w, gc, 0, 0, 260, 160 ); 
        XSetForeground( d, gc, WhitePixel(d, 0) ); 
        XDrawString( d, wb, gc, 10, 10, "push", 4 ); 
      } 
      break; 

    case ButtonPress: 
      printf("%d", event.xany.window);
      if ( event.xany.window == w ){ 
        printf( "w " ); 
        fflush( stdout ); 
      } 
      if ( event.xany.window == wb ){ 
        printf( "wb "); 
        fflush( stdout ); 
      } 
      break; 
    } 
  } 
} 
