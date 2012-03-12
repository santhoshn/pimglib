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

#include "pimg_jpeg.h"

struct my_error_mgr {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

// Exit error handler
METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message)(cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

// Load JPEG image
int _load_jpeg_rawimage(FILE *fp, struct RawImage **rimg)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	JSAMPARRAY buffer;
	unsigned int row_stride, x, i, iw, ih;
	struct RawImage *img = NULL;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if(setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		return EBADIMG;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	iw = cinfo.output_width;
	ih = cinfo.output_height;
	
	// If image dimension is greater than max, return
	if(iw*ih > MAX_WIDTH*MAX_HEIGHT) {
		jpeg_destroy_decompress(&cinfo);
		return EBIGIMG;
	}

	row_stride = iw * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	// Create Raw Image
	img = (struct RawImage*)malloc(sizeof(struct RawImage));
	if(img==NULL) {
		jpeg_destroy_decompress(&cinfo);
		return ENOMEMR;
	}
	img->x = img->y = img->delay = 0;
	img->mimg = NULL;
	img->next = NULL;
	img->width = iw;
	img->height = ih;
	img->img = (unsigned char*)malloc(iw*(ih+1)*3);
	if(img->img==NULL) {
		jpeg_destroy_decompress(&cinfo);
		free(img);
		return ENOMEMR;
	}

	// Read the image data into the memory
	while(cinfo.output_scanline < ih) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		for(i=0, x=0; i<row_stride; i+=cinfo.output_components, x+=3)
			memcpy(img->img + (cinfo.output_scanline-1)*iw*3 + x, buffer[0]+i, 3);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	_pimg_rawimglst_add(rimg, img);
	
	return 0;
}

int _save_jpeg_image(char *filename, Display *disp, struct Image **tsimg, unsigned quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *fp;
	JSAMPROW row_pointer[1];
	unsigned char *ibuff;
	int i, x, depth, bytes;
	unsigned t;
	struct Image *simg = *tsimg;

	if(simg==NULL)
		return EBADIMG;

	depth = DefaultDepth(disp, DefaultScreen(disp));
	
	if((fp = fopen(filename, "wb"))==NULL) {
		return -1;
	}
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = simg->width;
	cinfo.image_height = simg->height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	ibuff = malloc(simg->width*3);
	if(ibuff==NULL) {
		fclose(fp);
		jpeg_destroy_compress(&cinfo);
		return -1;
	}

	bytes = _dbpp>>3;
	while(cinfo.next_scanline<cinfo.image_height) {
		for(x=0; x<simg->width; x++) {
			memcpy(&t, simg->img->data + cinfo.next_scanline*simg->width*bytes + x*bytes, bytes);
	
			ibuff[x*3]   = ((t&_rmask)>>_rshft)<<(8-_rbits); 
			ibuff[x*3+1] = ((t&_gmask)>>_gshft)<<(8-_gbits); 
			ibuff[x*3+2] = ((t&_bmask)>>_bshft)<<(8-_bbits);
		}
		row_pointer[0] = ibuff;
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	free(ibuff);

	jpeg_finish_compress(&cinfo);
	fclose(fp);
	jpeg_destroy_compress(&cinfo);
	
	return 0;
}

int _save_jpeg_rawimage(char *filename, struct RawImage **tsimg, unsigned quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *fp;
	JSAMPROW row_pointer[1];
	unsigned char *ibuff;
	int x;
	struct RawImage *simg = *tsimg;

	if(simg==NULL)
		return EBADIMG;

	if((fp = fopen(filename, "wb"))==NULL) {
		return -1;
	}
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = simg->width;
	cinfo.image_height = simg->height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	ibuff = (unsigned char*)malloc(simg->width*3);
	if(ibuff==NULL) {
		fclose(fp);
		jpeg_destroy_compress(&cinfo);
		return -1;
	}

	while(cinfo.next_scanline<cinfo.image_height) {
		int t = cinfo.next_scanline*simg->width*3;

		for(x=0; x<simg->width; x++, t+=3) {
			ibuff[x*3] = simg->img[t]; 
			ibuff[x*3+1] = simg->img[t + 1]; 
			ibuff[x*3+2] = simg->img[t + 2]; 
		}
		row_pointer[0] = ibuff;
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	free(ibuff);

	jpeg_finish_compress(&cinfo);
	fclose(fp);
	jpeg_destroy_compress(&cinfo);
	
	return 0;
}
