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
#include<fcntl.h>
#include<unistd.h>
#include <netinet/in.h>

#if 1
int pimg_scrshot(Display *disp, struct Image **simg)
{
#ifdef __arm__
	int bytes=0, bytes_read=0, fd, i=0, j=0, z=0, idx;
	struct Image *img;
	unsigned char *ibuff, *tbuff;
	
	if(_get_visinfo(disp))
		return EBADDEP;
	//printf("rs = %d gs = %d bs = %d rc = %d gc = %d bc = %d rm = %x gm = %x bm = %x dbpp = %d\n", _rshft, _gshft, _bshft, 
	//											_rbits, _gbits, _bbits, _rmask, _gmask, _bmask, _dbpp);
	// Create the Image and add to the list
	img = (struct Image*)malloc(sizeof(struct Image));
	img->info = img->delay = img->x = img->y = 0;
	img->width = 240;
	img->height = 320;
	img->next = NULL;
	img->img = img->mimg = NULL;

	bytes = img->width * img->height * 2;
	
	ibuff = (unsigned char*)malloc(img->width*img->height*2);
	if(ibuff==NULL)
		return ENOMEMR;
	memset(ibuff, 0, img->width*img->height*2);
	
	fd = open("/dev/fb0", O_RDONLY);
	if(fd>0) {
		while(bytes_read < bytes)
			bytes_read += read(fd, ibuff+bytes_read, bytes);
		close(fd);
	} else
		return ENOFILE;

	// Rotate image by 90 degrees for Amida10k
	tbuff = (unsigned char*)malloc(img->width*img->height*2);
	if(tbuff) {
		for(i=0; i<img->height; i++) {
			for(j=0; j<img->width; j++) {
				idx = (img->height*img->width - (img->height-1 + (img->height*j)-i))*2;
				tbuff[z++] = ibuff[idx-2];
				tbuff[z++] = ibuff[idx-1];
			}
		}
		memcpy(ibuff, tbuff, img->width*img->height*2);
		free(tbuff);
	}
	img->img = XCreateImage(disp, CopyFromParent, _dbpp, ZPixmap, 0,\
					ibuff, img->width, img->height, 16, img->width*2);

	_pimg_imglst_add(simg, img);

	return 0;
#else
	return ENOSUPP;
#endif
}
#endif
