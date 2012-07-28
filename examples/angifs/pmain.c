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
				draw_image(zimg, 0, 0);
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
						pimg_zoom(disp, &simg, &zimg, xfact, yfact, 0);
						draw_image(zimg, 0, 0);
					}
				}
				if(key==XK_x) {
					if(xfact<4.0) {
						XSetForeground(disp, gc, WhitePixel(disp,scr));
						XFillRectangle(disp, win, gc, 10, 10, zimg->width, zimg->height);
						pimg_free(&zimg);
						xfact+=0.2;
						yfact+=0.2;
						pimg_zoom(disp, &simg, &zimg, xfact, yfact, 0);
						draw_image(zimg, 0, 0);
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
	struct Image *t;

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
	win = XCreateSimpleWindow(disp, RootWindow(disp,scr), 0, 0, 640, 480, \
					depth, BlackPixel(disp,scr), WhitePixel(disp,scr));
	XSetStandardProperties(disp, win, "Photoalbum", "photoalbum", None, 0, 0, NULL);
	XSelectInput(disp, win, ExposureMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask);
	XMapWindow(disp, win);

	// Create Graphics Context
	gc = XCreateGC(disp, win, 0, NULL);
	
	// Get Color map
	cmap = DefaultColormap(disp, scr);

	mpix = XCreatePixmap(disp, win, 640, 480, 1);
	
	ret = pimg_load(av[1], disp, &simg, 0);
	//ret = pimg_load(av[1], disp, &simg, SHAPE_MASK);
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
	}

	pimg_save("/tmp/p.jpg", disp, &simg, FORMAT_JPEG, 100);
	pimg_save("/tmp/p.pnm", disp, &simg, FORMAT_PNM, 100);

	pimg_zoom(disp, &simg, &zimg, xfact, yfact, 0);
	
	t = simg;
	
	XSetForeground(disp, gc, BlackPixel(disp, scr));
	XDrawRectangle(disp, win, gc, 25, 25, 50, 75);

#if 0
	// To get the mask pixmap
	if(t->mimg) {
		pmgc = XCreateGC(disp, mpix, 0, NULL);
		XPutImage(disp, mpix, pmgc, t->mimg, 0, 0, 10, 10, t->width, t->height);
		XSetClipMask(disp, gc, mpix);
	
		printf("W&H : %d %d\n",t->mimg->width, t->mimg->height);
		if(t->mimg->format==ZPixmap)
			printf("ZPixmap\n");
		printf("Byte order, Bitmap unit/bitorder/pad: %d %d/%d/%d\n",t->mimg->byte_order,
					t->mimg->bitmap_unit, t->mimg->bitmap_bit_order, t->mimg->bitmap_pad);
		printf("Depth %d\n",t->mimg->depth);
		printf("Bytes/line %d\n",t->mimg->bytes_per_line);
		printf("Bits/pix %d\n",t->mimg->bits_per_pixel);
		printf("MASK_RGB %ld %ld %ld\n",t->mimg->red_mask,t->mimg->green_mask,t->mimg->blue_mask);
			unsigned char ch;
			unsigned char bm[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
			int h, l, r;
			for(h=0; h<t->mimg->height; h++) {
				for(l=0; l<t->mimg->bytes_per_line; l++) {
					ch = *(t->mimg->data+ h*t->mimg->bytes_per_line + l);
					for(r=0; r<8; r++)
						printf("%d",(ch&bm[r])?1:0);
				}
				puts("");
			}
	}
#endif
	
	do {
		if(simg->next!=NULL) {
			if(t==NULL)
				t = simg;
			draw_image(t, 0, 0);
			usleep(t->delay*10000);
			t = t->next;
		}
	} while(process_events());

	pimg_free(&simg);
	pimg_free(&zimg);
	
	XCloseDisplay(disp);
	return 0;
}
