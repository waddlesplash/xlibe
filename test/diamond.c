/* diamond.cc */ 

#include <X11/Xlib.h> 
#include <stdio.h> 
#include <math.h> 

#define N 16 


unsigned long GetColor( Display* dis, char* color_name ) 
{ 
    Colormap cmap; 
    XColor near_color, true_color; 

    cmap = DefaultColormap( dis, 0 ); 
    XAllocNamedColor( dis, cmap, color_name, &near_color, &true_color ); 
    return( near_color.pixel ); 
}


int main( void ) 
{ 
    Display* dis; 
    Window win; 
    XSetWindowAttributes att; 
    GC gc; 
    XEvent ev; 

    int x[N],y[N]; 
    int i,j; 

    dis = XOpenDisplay( NULL ); 
    win = XCreateSimpleWindow( dis, RootWindow(dis,0), 100, 100, 
      256, 256, 5, WhitePixel(dis,0), BlackPixel(dis,0) ); 

    att.backing_store = WhenMapped; 
    XChangeWindowAttributes( dis, win, CWBackingStore, &att ); 

    XSelectInput( dis, win, ExposureMask ); 
    XMapWindow( dis, win ); 

    do{ 
        XNextEvent( dis, &ev); 
    }while( ev.type != Expose ); 

    gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 ); 
    XSetForeground( dis, gc, GetColor( dis, "green")  ); 


    for( i=0 ; i<N ; i++ ){ 
        x[i] = (int)(128*cos(2*M_PI*i/N))+128; 
        y[i] = (int)(128*sin(2*M_PI*i/N))+128; 
    } 

    for( i=0 ; i<N ; i++ ){ 
        for( j=i+1 ; j<N ; j++ ){ 
            XDrawLine( dis, win, gc, x[i], y[i], x[j], y[j] ); 
        } 
    } 

    XFlush( dis ); 

    printf("Push return key."); 
    getchar(); 
    XDestroyWindow( dis , win ); 
    XCloseDisplay( dis ); 

    return(0); 
}
