/* kt-draw.c */

#include <X11/Xlib.h>

unsigned long GetColor( Display* dis, char* color_name )
{
    Colormap cmap;
    XColor near_color, true_color;

    cmap = DefaultColormap( dis, 0 );
    XAllocNamedColor( dis, cmap, color_name, &near_color, &true_color );
    return( near_color.pixel );
}

int	main( int argc, char *argv[] )
{
	Display					*w_dis;
	Window					w_win;
	XSetWindowAttributes	w_att;
	GC						w_gc;
	XEvent					w_eve;
	XPoint					p1[10], p2[10];
	XSegment				s1[10], s2[10];
	XArc					arc1[10], arc2[10];
	char					w_title[]			= "kt-draw";
	char					w_icon_title[]		= "ICON!";
	int						i;

	w_dis = XOpenDisplay( NULL );
	w_win = XCreateSimpleWindow( w_dis, RootWindow( w_dis, 0 ),20 ,20 ,
				500, 400, 2, 0, 1);

	XSetStandardProperties( w_dis, w_win, w_title, w_icon_title,
		None, argv, argc, NULL );

	w_att.override_redirect = True;
	XChangeWindowAttributes( w_dis, w_win, CWOverrideRedirect, &w_att );

    XSelectInput( w_dis, w_win, ExposureMask ); 
    XMapWindow( w_dis, w_win ); 

    do{ 
        XNextEvent( w_dis, &w_eve); 
    }while( w_eve.type != Expose ); 

	w_gc = XCreateGC( w_dis, w_win, 0, 0 );
    XSetForeground( w_dis, w_gc, GetColor( w_dis, "green" ));

// sample start
	XDrawLine( w_dis, w_win, w_gc, 50, 100, 450, 100 );
	XDrawLine( w_dis, w_win, w_gc, 50, 100+50, 450, 100+50 );

	for( i = 0; i < 10; i++ ) {
		s1[i].x1 = (i * 5) + 250;
		s1[i].y1 = 110;
		s1[i].x2 = (i * 5) + 250;
		s1[i].y2 = 130;
	}
	XDrawSegments( w_dis, w_win, w_gc, s1, 10 );
	
	for( i = 0; i < 10; i++ ) {
		s2[i].x1 = 310;
		s2[i].y1 = (i * 4) + 110;
		s2[i].x2 = 350;
		s2[i].y2 = (i * 4) + 110;
	}
	XDrawSegments( w_dis, w_win, w_gc, s2, 10 );
	
	for( i = 0; i < 10; i++ ) {
		p1[i].x = (i * 5) + 280;
		if (i==0 || i==2 || i==4 || i==6 || i==8 ) {
			p1[i].y = (i * 5) + 200;
		}
		else {
			p1[i].y = (i * (-5)) + 200;
		}
	}
	XDrawLines( w_dis, w_win, w_gc, p1, 10, CoordModeOrigin );

	p2[0].x = 340;
	p2[0].y = 200;
	for( i = 1; i < 10; i++ ) {
		p2[i].x = 5;
				if (i==0 || i==2 || i==4 || i==6 || i==8 ) {
			p2[i].y = i * 5;
		}
		else {
			p2[i].y = i * (-5);
		}
	}
	XDrawLines( w_dis, w_win, w_gc, p2, 10, CoordModePrevious );

	XDrawPoint( w_dis, w_win, w_gc, 55, 105 );
	XDrawPoint( w_dis, w_win, w_gc, 55+10, 105+10 );

	for( i = 0; i < 10; i++ ) {
		p1[i].x = (i * 5) + 100;
		p1[i].y = 300;
	}
	XDrawPoints( w_dis, w_win, w_gc, p1, 10, CoordModeOrigin );

	p2[0].x = 100;
	p2[0].y = 310;
	for( i = 1; i < 10; i++ ) {
		p2[i].x = 5;
		p2[i].y = 0;
	}
	XDrawPoints( w_dis, w_win, w_gc, p2, 10, CoordModePrevious );

	XDrawArc( w_dis, w_win, w_gc, 50, 50, 50, 50, 45*64, 135*64 );
	XDrawArc( w_dis, w_win, w_gc, 50+100, 50, 50, 50, (45+30)*64, (135+30)*64 );

	for( i = 0; i < 10; i++ ) {
		arc1[i].x = (i * 15) + 250;
		arc1[i].y = 50;
		arc1[i].width = 10;
		arc1[i].height = 10;
		arc1[i].angle1 = 0;
		arc1[i].angle2 = ((i + 1) * 36) * 64;
	}
	XDrawArcs( w_dis, w_win, w_gc, arc1, 10 );
	
	for( i = 0; i < 10; i++ ) {
		arc2[i].x = (i * 15) + 250;
		arc2[i].y = 50 + 15;
		arc2[i].width = 10;
		arc2[i].height = 10;
		arc2[i].angle1 = 0;
		arc2[i].angle2 = ((10 - i) * 36) * 64;
	}
	XDrawArcs( w_dis, w_win, w_gc, arc2, 10 );
	
	XFillArc( w_dis, w_win, w_gc, 100, 50, 50, 50, 45*64, 135*64 );
	XFillArc( w_dis, w_win, w_gc, 100+100, 50, 50, 50, (45+30)*64, (135+30)*64 );

	XDrawRectangle( w_dis, w_win, w_gc, 60, 110, 160, 210 );
	XDrawRectangle( w_dis, w_win, w_gc, 60+30, 110+30, 160, 210 );

	XFillRectangle( w_dis, w_win, w_gc, 70, 120, 100, 100 );
	XFillRectangle( w_dis, w_win, w_gc, 70+100, 120+100, 100, 100 );
// sample end

	XFlush( w_dis );

    printf( "Push return key." );
	getchar( );

	XDestroyWindow( w_dis , w_win );
	XCloseDisplay( w_dis );

	return( 0 );
}
