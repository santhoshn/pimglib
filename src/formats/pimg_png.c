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

#include "pimg_png.h"

#define BG_RED		255
#define BG_GREEN 	255
#define BG_BLUE	255

typedef unsigned short ush;
typedef unsigned char uch;

int _load_png_rawimage(FILE *fp, struct RawImage **rimg, unsigned short iattr)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	unsigned int bit_depth, color_type, interlace_type, rowbytes, channels, i;
	png_uint_32 iw, ih=0;
	png_color_16 my_background={0,BG_RED,BG_GREEN,BG_BLUE,0}, *image_background;
	png_bytep *row=NULL;
	struct RawImage *img=NULL;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr==NULL)
		return ENOMEMR;

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return ENOMEMR;
	}

	// Error Handler
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		if(row!=NULL)
			free(row);
		return EBADIMG;
	}

	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);
	
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)&iw, (png_uint_32*)&ih, &bit_depth,\
			&color_type, &interlace_type, NULL, NULL);

	// If image dimension is greater than max, return
	if(iw*ih > MAX_WIDTH*MAX_HEIGHT) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return EBIGIMG;
	}

	if(png_get_bKGD(png_ptr, info_ptr, &image_background))
		png_set_background(png_ptr, image_background,  PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	else
		png_set_background(png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
	    	png_set_expand(png_ptr);
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	    	png_set_expand(png_ptr);
	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	    	png_set_expand(png_ptr);
	if(bit_depth == 16)
	    	png_set_strip_16(png_ptr);
	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	    	png_set_gray_to_rgb(png_ptr);

	if(interlace_type == PNG_INTERLACE_ADAM7)
		png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	channels = (unsigned int)png_get_channels(png_ptr, info_ptr);

	// Create Raw Image
     img = (struct RawImage*)malloc(sizeof(struct RawImage));
	if(img==NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
          return ENOMEMR;
	}
     img->x = img->y = img->delay = 0;
     img->img = img->mimg = NULL;
     img->next = NULL;
     img->width = iw;
     img->height = ih;
	img->img = (unsigned char*)malloc((ih+1)*rowbytes);
     if(img->img==NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
          free(img);
          return ENOMEMR;
     }

	row = (png_bytep*)malloc(sizeof(png_bytep)*ih);
	if(row==NULL) {
		free(img->img);
		free(img);
		return ENOMEMR;
	}
	for(i=0; i<ih; i++)
		row[i] = (png_bytep)img->img + i*rowbytes; 
	png_read_image(png_ptr, row);

	png_read_end(png_ptr, info_ptr);

	free(row);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	
	_pimg_rawimglst_add(rimg, img);
	
	return 0;
}

int _save_png_image(char *filename, Display *disp, struct Image **tsimg)
{
	png_structp  png_ptr;
	png_infop  info_ptr;
	int x, y, depth, bytes;
	struct Image *simg = *tsimg;
	FILE *fp;
	unsigned char *ibuff;
	unsigned t;

	if(simg==NULL)
		return EBADIMG;

	depth = DefaultDepth(disp, DefaultScreen(disp));
	
	if((fp = fopen(filename, "wb"))==NULL)
		return -1;

	ibuff = (unsigned char*)malloc(simg->width * 3);
	if(ibuff==NULL) {
		fclose(fp);
		return ENOMEMR;
	}
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		          (png_voidp)NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
	if(!png_ptr)
		return ENOMEMR;
	
	info_ptr = png_create_info_struct(png_ptr);
     if(!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return ENOMEMR;
	}

	png_init_io(png_ptr, fp);

	png_set_compression_level(png_ptr, 5);

	png_set_IHDR(png_ptr, info_ptr, simg->width, simg->height, 8, PNG_COLOR_TYPE_RGB,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	bytes = _dbpp>>3;
	for(y=0; y<simg->height; y++) {
		for(x=0; x<simg->width; x++) {
			memcpy(&t, simg->img->data + y*simg->width*bytes + x*bytes, bytes);
		
			ibuff[x*3]   = ((t&_rmask)>>_rshft)<<(8-_rbits); 
			ibuff[x*3+1] = ((t&_gmask)>>_gshft)<<(8-_gbits); 
			ibuff[x*3+2] = ((t&_bmask)>>_bshft)<<(8-_bbits);
		}
		png_write_row(png_ptr, ibuff);
	}

	png_write_end(png_ptr, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	free(ibuff);

	fclose(fp);

	return 0;
}
