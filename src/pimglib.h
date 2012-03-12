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

#ifndef __PIMG_
#define __PIMG_

// Return values of pimg_load procedure
#define ENOFILE 	-1
#define ENOTIMG 	-2
#define EBIGIMG 	-3
#define EBADIMG 	-4
#define ENOMEMR 	-5
#define ENOSUPP 	-6
#define EBADVAL 	-7
#define EBADDEP	-8

// Format of Image to be saved, using pimg_save procedure
// Value returned by the pimg_load function
#define FORMAT_JPEG		1
#define FORMAT_JPG		1
#define FORMAT_GIF		2
#define FORMAT_BMP		3
#define FORMAT_PNG		4
#define FORMAT_XPM		5
#define FORMAT_PNM		6
#define FORMAT_PGM		7

// Attribute for the pimg_load procedure
#define SHAPE_MASK 			0x01
#define DITHER				0x02	
#define PIMG_SHAPE_MASK	 	0x01
#define PIMG_DITHER			0x02	

// Zoom quality
#define ZOOM_LOW_QUALITY		1
#define ZOOM_HIGH_QUALITY	2

struct Image {
	// Image information and data
	unsigned short width, height;	// Dimension of image
	XImage *img, *mimg;			// Image and its mask(if available)
	unsigned char info;			// Miscellaneous information about image
	
	// The following attributes are only valid for animated GIFs
	unsigned short delay;		// 1/100 sec delay. Only for GIF imgs
	unsigned short x, y;		// Image anchor point within screen
	unsigned char disposal;		// Image disposal method for GIF imgs

	// If not NULL, points to next image, in animated GIFs
	struct Image *next;
};

struct RawImage {
	// Image information and data
	unsigned short width, height;	// Dimension of image
	unsigned char *img, *mimg;	// Image and its mask(if available)
	unsigned char info;			// Miscellaneous information about image
	
	// The following attributes are only valid for animated GIFs
	unsigned short delay;		// 1/100 sec delay. Only for GIF imgs
	unsigned short x, y;		// Image anchor point within screen
	unsigned char disposal;		// Image disposal method for GIF imgs

	// If not NULL, points to next image, in animated GIFs
	struct RawImage *next;
};

int pimg_load(char *filename, Display *disp, struct Image **ximg, unsigned short iattr);
int pimg_save(char *filename, Display *disp, struct Image **ximg, int format, unsigned quality);
void pimg_free(struct Image **ximg);
int pimg_zoom(Display *disp, struct Image **input, struct Image **output, float xfact, float yfact, unsigned quality);
int pimg_win_img(Display *disp, Drawable win, struct Image **output);
int pimg_win_subimg(Display *disp, Drawable win, struct Image **output, int ix, int iy, int iwidth, int iheight);
int pimg_subimg(Display *disp, struct Image **input, struct Image **output, unsigned x, unsigned y, unsigned width, unsigned height);
int pimg_scrshot(Display *disp, struct Image **ximg);
int pimg_change_col(Display *disp, struct Image **input, struct Image **output, unsigned char *st, unsigned char *dt, int npix);

int pimg_raw_load(char *file, struct RawImage **rimg, unsigned short iattr);
int pimg_raw_save(char *filename, struct RawImage **ximg, int format, unsigned quality);
void pimg_raw_free(struct RawImage **rimg);
int pimg_raw_dither(int depth, struct RawImage **rimg);
int pimg_raw_zoom(struct RawImage **input, struct RawImage **output, float xfact, float yfact, unsigned quality);

int pimg_raw_copyto_x(Display *disp, struct RawImage **rimg, struct Image **ximg);
int pimg_raw_moveto_x(Display *disp, struct RawImage **rimg, struct Image **ximg);

char *pimg_errmsg(int err_val);
#endif
