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

#include "plib.h"

int pimg_win_img(Display *disp, Drawable win, struct Image **output)
{	
	int depth, x, y, width, height, bd, dpth;
	struct Image *img=NULL;
	Window root;

	depth = DefaultDepth(disp, DefaultScreen(disp));

	if(depth!=4 && depth!=12 && depth!=16)
		return EBADDEP;

	XGetGeometry(disp, win, &root, &x, &y, &width, &height, &bd, &dpth);

	img = (struct Image*)malloc(sizeof(struct Image));
	img->delay = 0;
	img->x = 0;
	img->y = 0;
	img->next = NULL;
	img->mimg = NULL;
	img->width = width;
	img->height = height;
	img->img = XGetImage(disp, win, 0, 0, width, height, AllPlanes, ZPixmap);

	*output = img;

	return 0;
}

int pimg_win_subimg(Display *disp, Drawable win, struct Image **output,
									int ix, int iy, int iwidth, int iheight)
{	
	int depth, x, y, width, height, bd, dpth;
	struct Image *img=NULL;
	Window root;

	depth = DefaultDepth(disp, DefaultScreen(disp));

	if(depth!=4 && depth!=12 && depth!=16)
		return EBADDEP;

	XGetGeometry(disp, win, &root, &x, &y, &width, &height, &bd, &dpth);

	if(ix>=width || iy>=height || ix+iwidth>width || iy+iheight>height)
		return EBADVAL;
	
	img = (struct Image*)malloc(sizeof(struct Image));
	img->delay = 0;
	img->x = 0;
	img->y = 0;
	img->next = NULL;
	img->mimg = NULL;
	img->width = iwidth;
	img->height = iheight;
	img->img = XGetImage(disp, win, ix, iy, iwidth, iheight, AllPlanes, ZPixmap);

	*output = img;

	return 0;
}
