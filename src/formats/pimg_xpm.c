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

#include "pimg_xpm.h"

int load_xpm_image(char *file, FILE *fp, Display *disp, struct Image **simg, unsigned short iattr)
{
	int comment=0, quote=0, w, h, cols, cpp, i=0, ret, depth;
	char c=' ', pc, line[30];
	struct Image *img=NULL;
	XpmAttributes attr;

	depth = DefaultDepth(disp, DefaultScreen(disp));

	// Check for the maximum image dimension supported
	while(!feof(fp)) {
		pc = c;
		c = getc(fp);
		
		if(!quote) {
			if(pc=='/' && c=='*')
				comment = 1;
			else if(pc=='*' && c=='/' && comment)
				comment = 0;
		}
		
		if(!comment) {
			if(!quote && c=='"') {
				quote = 1;
				i = 0;
				continue;
			} else if(quote && c=='"') {
				line[i] = 0;
				quote = 0;
				// Get the image dimension, colors and chars/pixel
				sscanf(line, "%d %d %d %d", &w, &h, &cols, &cpp);
				if(cols>65000 || cpp>5)
					return EBADIMG;
				if(w*h > MAX_WIDTH*MAX_HEIGHT)
					return EBIGIMG;

				// Create a node
				img = (struct Image*)malloc(sizeof(struct Image));
				img->delay = 0;
				img->x = img->y = 0;
				img->width = w;
				img->height = h;
				img->next = NULL;
				img->img = img->mimg = NULL;
				break;
			}

			if(quote) {
				line[i++] = c;
			}
		}
	}

	memset(&attr, 0x00, sizeof(XpmAttributes));
	attr.valuemask |= XpmRGBCloseness;
	attr.closeness = 80000;
	attr.red_closeness = 40000;
	attr.green_closeness = 40000;
	attr.blue_closeness = 40000;
		
	if(iattr & SHAPE_MASK) {
		ret = XpmReadFileToImage(disp, file, &img->img, &img->mimg, &attr);
	} else {
		ret = XpmReadFileToImage(disp, file, &img->img, NULL, &attr);
	}

	if(ret==XpmFileInvalid) {
		free(img);
		return EBADIMG;
	}
	if(ret==XpmNoMemory) {
		free(img);
		return ENOMEMR;
	}
	if(ret==XpmOpenFailed || ret==XpmColorFailed) {
		free(img);
		return ENOSUPP;
	}
	// Add the Image node to the list
	_pimg_imglst_add(simg, img);

	return 0;
}
