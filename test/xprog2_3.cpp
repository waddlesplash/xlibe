/*
** 第２章 Xlibを使う
**   2-3 絵を使いまわす
**
** 「ピックスマップを使う」プログラム
*/
#include <X11/Xlib.h>                   /* Xlib ヘッダ      */
#include <iostream>

int main(void)
{
  Display*             display;         /* Display ID       */
  int                  screen;          /* Screen ID        */
  Window               window;          /* Window ID        */
  XSetWindowAttributes attr;            /* Window Attribute */
  XEvent               event;           /* Event            */
  GC                   gc;              /* Graphic Context  */
  Colormap             colormap;        /* Colormap         */
  XColor               near_color, true_color;
  unsigned long        red, green;
  Pixmap               pixmap;          /* Pixmap           */
  int i;
 

  /*
  **  Xサーバと接続する
  */
  display = XOpenDisplay( NULL );       /* Xサーバに接続            */
  if (display == NULL)                  /* 接続に失敗の場合         */
    return 1;                           /* エラー終了する           */

  screen = DefaultScreen( display );    /* デフォルトスクリーン番号 */

  /*
  ** ウィンドウを作る
  */
  window = XCreateSimpleWindow(         /* Window作成               */
          display,
          RootWindow(display, screen),  /* ルートウィンドウを指定   */
          100, 100, 256, 256,
          3,                            /* ウィンドウ枠の線幅       */
          BlackPixel(display, screen),  /* ウィンドウ枠の色         */
          WhitePixel(display, screen)); /* ウィンドウの背景色       */

  /*
  ** ウィンドウの属性を変更
  */
  attr.backing_store = WhenMapped;      /* ウィンドウの内容を記憶   */
  XChangeWindowAttributes( display, window, CWBackingStore, &attr);

  /*
  ** ウィンドウを表示する
  */
  XSelectInput( display, window, ExposureMask );
  XMapWindow( display, window );        /* ウィンドウを表示         */
  /*
  ** Xサーバに依頼を送る
  */
  XFlush( display );                    /* Xコマンドをフラッシュ    */
  /*
  ** ウィンドウが表示完了するまで待つ
  */
  do {
    XNextEvent( display, &event );
  } while ( event.type != Expose );

  /*
  ** Pixmap を作成する
  */
  pixmap = XCreatePixmap( display, window, 32, 32, DefaultDepth( display, screen ));
  /*
  ** GC を作成する
  */
  gc = XCreateGC( display, RootWindow( display, screen ), 0, 0);
  XSetGraphicsExposures( display, gc, False );

  /*
  ** 色を取得する
  */
  colormap = DefaultColormap( display, screen );
  XAllocNamedColor( display, colormap, "green", &near_color, &true_color );
  green = near_color.pixel;
  XAllocNamedColor( display, colormap, "red", &near_color, &true_color );
  red = near_color.pixel;

  /*
  ** 描画する
  */
  XSetForeground( display, gc, green );
  XFillRectangle( display, pixmap, gc, 0, 0, 32, 32 );
  XSetForeground( display, gc, red );
  XFillArc( display, pixmap, gc, 0, 0, 32, 32, 0, 360*64 );
  XFlush( display );                    /* Xコマンドをフラッシュ    */

  for( i=0 ; i<5 ; i++ ){
    XCopyArea( display, pixmap, window, gc, 0, 0, 32, 32, i*48+16, 112 );
  }
  XFlush( display );                    /* Xコマンドをフラッシュ    */

  /*
  ** ウィンドウがクリックされるまで待つ
  */
  XSelectInput( display, window, ButtonPressMask );
  do {
    XNextEvent( display, &event );
  } while ( event.type != ButtonPress );

  /*
  ** 後片付け
  */
  XDestroyWindow( display, window );    /* ウィンドウを削除         */
  XCloseDisplay(display);               /* Xサーバとの接続を解除    */
} 
