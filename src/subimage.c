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

int pimg_subimg(Display *disp, struct Image **input, struct Image **output,
									unsigned x, unsigned y, unsigned width, unsigned height)
{
	int depth, iw, ih;
	struct Image *simg, *img=NULL;
	
	if(input==NULL)
		return EBADIMG;

	depth = DefaultDepth(disp, DefaultScreen(disp));

	if(depth!=4 && depth!=12 && depth!=16)
		return EBADDEP;

	simg = *input;
	iw = simg->width;
     ih = simg->height;

	if(x<0 || y<0 || x+width>iw || y+height>ih)
		return EBADVAL;

	img = (struct Image*)malloc(sizeof(struct Image));
     img->delay = 0;
	img->x = 0;
	img->y = 0;
	img->next = NULL;
	img->img = img->mimg = NULL;
	img->width = width;
	img->height = height;
				
	img->img = XSubImage(simg->img, x, y, width, height);
	if(simg->mimg)
		img->mimg = XSubImage(simg->mimg, x, y, width, height);
		
	*output = img;

	return 0;
}
