/*
 * Copyright (C) 2012 Santhosh N <santhoshn@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/xpm.h>

#include <pimglib.h>

Display *disp;
Window win;
Pixmap pix=None, mpix=None;
GC gc, pgc, pmgc;
Colormap cmap;
int scr, depth;
XEvent event;
XFontStruct *bfont, *mfont, *sfont;
KeySym key;
float xfact=1.0, yfact=1.0;

struct Image *simg, *zimg;

// Return the required color
int get_color(char *col)
{
	XColor color;

	XAllocNamedColor(disp, cmap, col, &color, &color);

	return color.pixel;
}

void draw_image(struct Image *img, int x, int y)
{
	XPutImage(disp, win, gc, img->img, 0, 0, img->x+x, img->y+y, img->width, img->height);
}

int process_events()
{
	while(XPending(disp)) {
		XNextEvent(disp, &event);
		switch(event.type) {
			case Expose:
				if(event.xexpose.count>0) break;
				draw_image(zimg, 10, 10);
				break;
			case ButtonPress:
				break;
			case ButtonRelease:
				break;
			case KeyPress:
				key = XLookupKeysym(&event.xkey, 0);
				if(key==XK_q)
					return 0;
				if(key==XK_z) {
					if(xfact>0.2) {
						XSetForeground(disp, gc, WhitePixel(disp,scr));
						XFillRectangle(disp, win, gc, 10, 10, zimg->width, zimg->height);
						pimg_free(&zimg);
						xfact-=0.2;
						yfact-=0.2;
						pimg_zoom(disp, &simg, &zimg, xfact, yfact, ZOOM_LOW_QUALITY);
						draw_image(zimg, 10, 10);
					}
				}
				if(key==XK_x) {
					if(xfact<4.0) {
						XSetForeground(disp, gc, WhitePixel(disp,scr));
						XFillRectangle(disp, win, gc, 10, 10, zimg->width, zimg->height);
						pimg_free(&zimg);
						xfact+=0.2;
						yfact+=0.2;
						pimg_zoom(disp, &simg, &zimg, xfact, yfact, ZOOM_LOW_QUALITY);
						draw_image(zimg, 10, 10);
					}
				}
				break;
			default:
				break;
		}
	}
	return 1;
}

int main(int ac, char **av)
{
	int ret;
	struct Image *wimg=NULL;

	// Open display
	disp = XOpenDisplay(NULL);
	if(disp==NULL) {
		fprintf(stderr,"Error opening display.\n");
		exit(1);
	}

	// Get screen info here
	scr = DefaultScreen(disp);

	depth = DefaultDepth(disp, scr);
	
	// Get the font here
	sfont = XLoadQueryFont(disp, "-picopeta-simputer-medium-r-normal--9-120-101-108-p-120-fontspecific-0");
	mfont = XLoadQueryFont(disp, "-picopeta-simputer-medium-r-normal--11-120-101-108-p-120-fontspecific-0");
	bfont = XLoadQueryFont(disp, "-picopeta-simputer-bold-r-normal--11-120-101-108-p-120-fontspecific-0");

	// Create and map the window here
	win = XCreateSimpleWindow(disp, RootWindow(disp,scr), 0, 0, 280, 360, \
					depth, BlackPixel(disp,scr), WhitePixel(disp,scr));
	XSetStandardProperties(disp, win, "Photoalbum", "photoalbum", None, 0, 0, NULL);
	XSelectInput(disp, win, ExposureMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask);
	XMapWindow(disp, win);

	// Create Graphics Context
	gc = XCreateGC(disp, win, 0, NULL);
	
	// Get Color map
	cmap = DefaultColormap(disp, scr);

	mpix = XCreatePixmap(disp, win, 440, 580, 1);
	
	ret = pimg_load(av[1], disp, &simg, DITHER);
	//ret = pimg_load(av[1], disp, &simg, SHAPE_MASK);
	if(ret<0) {
		puts(pimg_errmsg(ret));
		exit(-1);
	}

	//pimg_save("/tmp/p.jpg", disp, &simg, FORMAT_JPEG, 100);
	//pimg_save("/tmp/p.pnm", disp, &simg, FORMAT_PNM, 100);

	pimg_zoom(disp, &simg, &zimg, xfact, yfact, ZOOM_LOW_QUALITY);
	
	do {
	} while(process_events());
	
	pimg_win_img(disp, win, &wimg);
	pimg_save("/tmp/p.jpg", disp, &simg, FORMAT_JPEG, 85);

	pimg_free(&simg);
	pimg_free(&zimg);
	
	XCloseDisplay(disp);
	return 0;
}
