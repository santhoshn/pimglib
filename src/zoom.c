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

// TODO: Implement zoom quality

#include "plib.h"

unsigned int *build_index(unsigned int width, float zoom, unsigned int *rwidth)
{
	float fzoom;
	unsigned int *index, a;

	if(zoom==0.0) {
		fzoom = 100.0;
		*rwidth = width;
	} else {
		fzoom = zoom;
		*rwidth = (fzoom*width+0.5);
	}
	
	index = (unsigned int*)malloc(sizeof(unsigned int)*(*rwidth));
	
	for(a=0; a<*rwidth; a++)
		if(zoom)
			*(index+a) = (float)a/fzoom;
		else
			*(index+a) = a;
	
	return(index);
}

int pimg_zoom(Display *disp, struct Image **input, struct Image **output, 
											float xfact, float yfact, unsigned quality)
{
	unsigned char *ibuff;
	struct Image *img, *zimg;
	int depth, x, y;
	unsigned int *xindex, *yindex, xwidth, ywidth;

	if(input==NULL)
		return EBADIMG;

	depth = DefaultDepth(disp, DefaultScreen(disp));

	img = *input;
	
	if(xfact<=0.0 || yfact<=0.0)
		return EBADVAL;

	xindex = build_index(img->width, xfact, &xwidth);
	yindex = build_index(img->height, yfact, &ywidth);

	// Allocate memory for the zoomed image
	ibuff = (unsigned char*)malloc(xwidth*(ywidth+1)*(_dbpp>>3));
	if(ibuff==NULL) {
		free(xindex);
		free(yindex);
		return ENOMEMR;
	}

	// Create a image node
	zimg = (struct Image*)malloc(sizeof(struct Image));
     zimg->delay = 0;
     zimg->x = 0;
     zimg->y = 0;
     zimg->next = NULL;
     zimg->width = xwidth;
     zimg->height = ywidth;
     zimg->img = zimg->mimg = NULL;
		
	for(y=0; y<ywidth; y++)
		for(x=0; x<xwidth; x++)
			memcpy(ibuff + y*xwidth*(_dbpp>>3) + x*(_dbpp>>3), 
					img->img->data + yindex[y]*img->width*(_dbpp>>3) + xindex[x]*(_dbpp>>3), (_dbpp>>3));

	zimg->img = XCreateImage(disp, CopyFromParent, depth, ZPixmap, 0,\
					ibuff, xwidth, ywidth, _dbpp, xwidth*(_dbpp>>3));

	*output = zimg;
	free(xindex);
	free(yindex);

	return 0;
}

int pimg_raw_zoom(struct RawImage **input, struct RawImage **output, 
											float xfact, float yfact, unsigned quality)
{
	unsigned char *ibuff;
	struct RawImage *img, *zimg;
	int depth, x, y;
	unsigned int *xindex, *yindex, xwidth, ywidth;

	if(input==NULL)
		return EBADIMG;

	img = *input;
	
	if(xfact<=0.0 || yfact<=0.0)
		return EBADVAL;

	xindex = build_index(img->width, xfact, &xwidth);
	yindex = build_index(img->height, yfact, &ywidth);

	// Allocate memory for the zoomed image
	ibuff = (unsigned char*)malloc(xwidth*(ywidth+1)*3);
	if(ibuff==NULL) {
		free(xindex);
		free(yindex);
		return ENOMEMR;
	}

	// Create a image node
	zimg = (struct RawImage*)malloc(sizeof(struct RawImage));
     zimg->delay = 0;
     zimg->x = 0;
     zimg->y = 0;
     zimg->next = NULL;
     zimg->width = xwidth;
     zimg->height = ywidth;
     zimg->img = zimg->mimg = NULL;
		
	for(y=0; y<ywidth; y++)
		for(x=0; x<xwidth; x++)
			memcpy(ibuff + y*xwidth*3 + x*3, 
					img->img + yindex[y]*img->width*3 + xindex[x]*3, 3);

	zimg->img = ibuff;

	*output = zimg;
	free(xindex);
	free(yindex);

	return 0;
}
