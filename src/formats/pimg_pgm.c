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
// Image Library

#include "pimg_pgm.h"

int _save_pgm_image(char *filename, Display *disp, struct Image **img)
{
	int depth, x, y, bytes;
	struct Image *simg = *img;
	unsigned char r, g, b, ch;
	unsigned t;
	unsigned long p;
	FILE *fp;

	if(simg==NULL)
		return EBADIMG;

	fp = fopen(filename, "w");
	if(fp==NULL)
		return ENOFILE;
			
	fprintf(fp, "P5\n%d %d\n255\n", simg->width, simg->height);

	depth = DefaultDepth(disp, DefaultScreen(disp));

	bytes = _dbpp>>3;
	for(y=0; y<simg->height; y++) {
		for(x=0; x<simg->width; x++) {
			memcpy(&t, simg->img->data + y*simg->width*bytes + x*bytes, bytes);

			r = ((t&_rmask)>>_rshft)<<(8-_rbits); 
			g = ((t&_gmask)>>_gshft)<<(8-_gbits); 
			b = ((t&_bmask)>>_bshft)<<(8-_bbits);

			p = r*11 + g*16 + b*5;
			ch = (p>>5)&0xff;
			fprintf(fp, "%c", ch);
		}
	}

	fclose(fp);

	return 0;
}
