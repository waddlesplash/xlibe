/* kt-clock.c */

#include <X11/Xlib.h>
#include <math.h>
//-----
#define	WIN_WIDTH	100
#define	WIN_HEIGHT	100
#define NOWT		25
//-----
#define END_TIME	30		//----- clock timer end (sec)
int			ti = 1;			//----- clock timer sleep (sec)
//-----
Display		*w_dis;
Window		w_win;
GC			w_gc;
//-----
void draw_hand( double th, int r_start, int r_end )
{
	int x_st, x_end, y_st, y_end;

	x_st = WIN_WIDTH / 2 + r_start * cos( th );
	x_end = WIN_WIDTH / 2 + r_end * cos( th );
	y_st = WIN_WIDTH / 2 + r_start * sin( th );
	y_end = WIN_WIDTH / 2 + r_end * sin( th );
	XDrawLine( w_dis, w_win, w_gc, x_st, y_st, x_end, y_end );
	return;
}
//-----
void drawFace( void )
{
	time_t ti;
	struct tm *now;
	char wknowtimeE[14], wknowtimeJ[16];

	time( &ti );
	now = localtime( &ti );
	
	XClearWindow( w_dis, w_win );
	
	XDrawArc( w_dis, w_win, w_gc, 0, 0, 99, 99, 0, 360 * 64 );
	XDrawArc( w_dis, w_win, w_gc, 20, 20, 59, 59, 0, 360 * 64 );
	XDrawArc( w_dis, w_win, w_gc, 30, 30, 39, 39, 0, 360 * 64 );
	XDrawArc( w_dis, w_win, w_gc, 40, 40, 19, 19, 0, 360 * 64 );

	draw_hand( M_PI / 2 - ( now->tm_hour ) * 2 * M_PI / 12, 10, 20 );
	draw_hand( M_PI / 2 - ( now->tm_min ) * 2 * M_PI / 60, 20, 30 );
	draw_hand( M_PI / 2 - ( now->tm_sec ) * 2 * M_PI / 60, 30, 50 );
//-----
	sprintf( wknowtimeE, "[%02dh:%02dt:%02ds]\n",
	                       now->tm_hour, now->tm_min, now->tm_sec );
	XDrawString( w_dis, w_win, w_gc, 25, 108,
	                       wknowtimeE, strlen( wknowtimeE ));
	sprintf( wknowtimeJ, "[%02dŽž%02d•ª%02d•b]\n",
	                       now->tm_hour, now->tm_min, now->tm_sec );
	XDrawString16( w_dis, w_win, w_gc, 18, 120,
	                      (XChar2b*)wknowtimeJ, strlen( wknowtimeJ ));
//-----
	XFlush( w_dis );
	return;
}
//----- main
int	main( int argc, char *argv[] )
{
	XSetWindowAttributes	w_att;
	XEvent					w_eve;
	char					w_title[]			= "kt-clock";
	char					w_icon_title[]		= "ICON!";
	int						i;
	XFontStruct				*w_xfs;

	w_dis = XOpenDisplay( NULL );
	w_win = XCreateSimpleWindow( w_dis, RootWindow( w_dis, 0 ),10 ,100 ,
				WIN_WIDTH, WIN_HEIGHT + NOWT, 2,
				BlackPixel( w_dis, 0 ), WhitePixel( w_dis, 0 ));

	XSetStandardProperties( w_dis, w_win, w_title, w_icon_title,
		None, argv, argc, NULL );

	w_att.override_redirect = True;
	XChangeWindowAttributes( w_dis, w_win, CWOverrideRedirect, &w_att );

    w_gc = XCreateGC( w_dis, w_win, 0, 0 );

	w_xfs = XLoadQueryFont( w_dis, "fixed" );
	XSetFont( w_dis, w_gc, w_xfs->fid );

    XSelectInput( w_dis, w_win, ExposureMask );
    XMapWindow( w_dis, w_win );

    do{
        XNextEvent( w_dis, &w_eve);
    }while( w_eve.type != Expose );
    
    XSetForeground( w_dis, w_gc, WhitePixel( w_dis, 0 ));
    XFillRectangle( w_dis, w_win, w_gc, 0, 0, WIN_WIDTH, WIN_HEIGHT + NOWT );
    XSetForeground( w_dis, w_gc, BlackPixel( w_dis, 0 ));
//-----
	printf( "timer start. wait %d sec.\n", END_TIME );
//-----
    for( i = 0; i < END_TIME; i++ ) {
	    XSetLineAttributes( w_dis, w_gc, 1, LineSolid, CapButt, JoinMiter );
        drawFace( );
        sleep( ti );
    }
//-----
	XDestroyWindow( w_dis , w_win );
	XCloseDisplay( w_dis );

	return( 0 );
}
