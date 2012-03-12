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
	char stab[10], dtab[10];

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

	ret = pimg_load(av[1], disp, &simg, SHAPE_MASK);
	switch(ret) {
		case EBADIMG:
			printf("Bad Image\n");
			exit(1);
		case EBIGIMG:
			printf("Big Image\n");
			exit(1);
		case ENOTIMG:
			printf("Not Image\n");
			exit(1);
		case ENOFILE:
			printf("No file specified.\n");
			exit(1);
		case ENOMEMR:
			printf("No enough memory\n");
			exit(1);
		case ENOSUPP:
			printf("Image format not supported\n");
			exit(1);
		case EBADDEP:
			printf("Depth of the display should be 4/12/16 bits\n");
			exit(1);
	}

	sprintf(stab, "%c%c%c", 0x00, 0x00, 0x00);
	sprintf(dtab, "%c%c%c", 0x88, 0x99, 0xff);

	pimg_change_col(disp, &simg, &zimg, stab, dtab, 1);

	do {
	} while(process_events());
	
	pimg_free(&simg);
	pimg_free(&zimg);
	
	XCloseDisplay(disp);
	return 0;
}
