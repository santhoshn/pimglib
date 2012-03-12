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

int pimg_change_col(Display *disp, struct Image **input, struct Image **output, 
											unsigned char *st, unsigned char *dt, int npix)
{	
	int depth, i, x, y, iw, ih, t, *stab, *dtab;
	struct Image *simg, *img=NULL;
	unsigned char *ibuff;

	if(input==NULL)
		return EBADVAL;

	depth = DefaultDepth(disp, DefaultScreen(disp));

	if(depth!=4 && depth!=12 && depth!=16)
		return EBADDEP;

	simg = *input;
	iw = simg->width;
	ih = simg->height;

	// Allocate memory for the image
	if(depth==4) {
		ibuff = (unsigned char*)malloc(iw*(ih+1));
	} else {
		ibuff = (unsigned char*)malloc(iw*(ih+1)*2);
	}
	if(ibuff==NULL)
		return ENOMEMR;
	
	img = (struct Image*)malloc(sizeof(struct Image));
     img->delay = 0;
     img->x = 0;
     img->y = 0;
     img->next = NULL;
     img->width = iw;
     img->height = ih;
     img->img = img->mimg = NULL;

	stab = (int*)malloc(npix*sizeof(unsigned int));
	dtab = (int*)malloc(npix*sizeof(unsigned int));
	
	switch(depth) {
		case 12:
			for(i=0; i<npix; i++) {
				stab[i] = ((st[i*3]>>4)<<8) + ((st[i*3+1]>>4)<<4) + (st[i*3+2]>>4);
				dtab[i] = ((dt[i*3]>>4)<<8) + ((dt[i*3+1]>>4)<<4) + (dt[i*3+2]>>4);
			}

			for(y=0; y<ih; y++) {
				for(x=0; x<iw; x++) {
					memcpy(&t, simg->img->data + y*iw*2 + x*2, 2);
					t = t&0x0fff;

					for(i=0; i<npix; i++)
						if(stab[i]>=(t-50) && stab[i]<=(t+50))
							break;

					if(i!=npix) {
						memcpy(ibuff + y*iw*2 + x*2, &dtab[i], 2); 
					} else {
						memcpy(ibuff + y*iw*2 + x*2, simg->img->data + y*iw*2 + x*2, 2);
					}
				}
			}
			break;
		case 16:
			_get_visinfo(disp);
			
			if(_rmask==0x00f800 && _gmask==0x0007e0 && _bmask==0x00001f) {
				for(i=0; i<npix; i++) {
					stab[i] = ((st[i*3]>>3)<<11) + ((st[i*3+1]>>2)<<5) + (st[i*3+2]>>3); 
					dtab[i] = ((dt[i*3]>>3)<<11) + ((dt[i*3+1]>>2)<<5) + (dt[i*3+2]>>3); 
				}
			} else {
				for(i=0; i<npix; i++) {
					stab[i] = ((st[i*3]>>2)<<10) + ((st[i*3+1]>>3)<<5) + (st[i*3+2]>>3); 
					dtab[i] = ((dt[i*3]>>2)<<10) + ((dt[i*3+1]>>3)<<5) + (dt[i*3+2]>>3); 
				}
			}

			for(y=0; y<ih; y++) {
				for(x=0; x<iw; x++) {
					memcpy(&t, simg->img->data + y*iw*2 + x*2, 2);
					t = t&0xffff;

					for(i=0; i<npix; i++)
						if(stab[i]>=(t-50) && stab[i]<=(t+50))
							break;

					if(i!=npix) {
						memcpy(ibuff + y*iw*2 + x*2, &dtab[i], 2); 
					} else {
						memcpy(ibuff + y*iw*2 + x*2, simg->img->data + y*iw*2 + x*2, 2);
					}
				}
			}
			break;
	}

	// Create the XImage
	if(depth==4) {
		img->img = XSubImage(simg->img, 0, 0, iw, ih);
	} else {
		img->img = XCreateImage(disp, CopyFromParent, depth, ZPixmap, 0,\
	     					ibuff, img->width, img->height, 16, img->width*2);
	}
	
	// If mask is present in the input image, put it in output
	if(simg->mimg)
		img->mimg = XSubImage(simg->mimg, 0, 0, iw, ih);

	*output = img;

	free(stab);
	free(dtab);

	return 0;
}
