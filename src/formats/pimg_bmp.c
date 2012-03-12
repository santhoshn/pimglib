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

#include "pimg_bmp.h"

typedef unsigned short u16b;
typedef unsigned int u32b;

typedef struct BITMAPFILEHEADER
{
	u16b bfType;
	u32b bfSize;
	u16b bfReserved1;
	u16b bfReserved2;
	u32b bfOffBits;
} BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER
{
	u32b biSize;
	u32b biWidth;
	u32b biHeight;
	u16b biPlanes;
	u16b biBitCount;
	u32b biCompresion;
	u32b biSizeImage;
	u32b biXPelsPerMeter;
	u32b biYPelsPerMeter;
	u32b biClrUsed;
	u32b biClrImportand;
} BITMAPINFOHEADER;

typedef struct RGBQUAD
{
	unsigned char b, g, r;
} RGBQUAD;

static unsigned char get_byte(FILE *fp)
{
	return (getc(fp) & 0xFF);
}

static void rd_u16b(FILE *fp, u16b *ip)
{
	(*ip) = get_byte(fp);
	(*ip) |= ((u16b)(get_byte(fp)) << 8);
}

static void rd_u32b(FILE *fp, u32b *ip)
{
	(*ip) = get_byte(fp);
	(*ip) |= ((u32b)(get_byte(fp)) << 8);
	(*ip) |= ((u32b)(get_byte(fp)) << 16);
	(*ip) |= ((u32b)(get_byte(fp)) << 24);
}

int _load_bmp_rawimage(FILE *fp, struct RawImage **rimg)
{
	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;
	unsigned int bpp, x, y, i, lsize, lpos, cols, bit, bc, iw, ih;
	unsigned char c, buff[20], rgb[5];
	RGBQUAD clr[256];
	struct RawImage *img=NULL;

	// Read the BITMAPFILEHEADER
	rd_u16b(fp, &(fileheader.bfType));
	rd_u32b(fp, &(fileheader.bfSize));
	rd_u16b(fp, &(fileheader.bfReserved1));
	rd_u16b(fp, &(fileheader.bfReserved2));
	rd_u32b(fp, &(fileheader.bfOffBits));

	// Read the BITMAPINFOHEADER
	rd_u32b(fp, &(infoheader.biSize));
	rd_u32b(fp, &(infoheader.biWidth));
	rd_u32b(fp, &(infoheader.biHeight));
	rd_u16b(fp, &(infoheader.biPlanes));
	rd_u16b(fp, &(infoheader.biBitCount));
	rd_u32b(fp, &(infoheader.biCompresion));
	rd_u32b(fp, &(infoheader.biSizeImage));
	rd_u32b(fp, &(infoheader.biXPelsPerMeter));
	rd_u32b(fp, &(infoheader.biYPelsPerMeter));
	rd_u32b(fp, &(infoheader.biClrUsed));
	rd_u32b(fp, &(infoheader.biClrImportand));

	bpp = infoheader.biBitCount; 
	if(bpp!=1 && bpp!=4 && bpp!=8 && bpp!=24) {
		return EBADIMG;
	}

	iw = infoheader.biWidth;
	ih = infoheader.biHeight;
	
	// If image dimension is greater than max, return
	if(iw*ih > MAX_WIDTH*MAX_HEIGHT) {
		return EBIGIMG;
	}

	// Verify the header
	if (feof(fp) || (fileheader.bfType != 19778) || (infoheader.biSize != 40))
		return EBADIMG;

	// Presently compression not handled
	if(infoheader.biCompresion) {
		fprintf(stderr,"BMP Image decompression not handled.\n");
		return ENOSUPP;
	}

	// Build the color table
	if(bpp==1 || bpp==4 || bpp==8) {
		cols = infoheader.biClrUsed;
		if(!cols)
			cols = (unsigned int)pow(2, bpp);
		for(i=0; i<cols; i++) {
			fread(buff, 1, 4, fp);
			clr[i].b = buff[0];
			clr[i].g = buff[1];
			clr[i].r = buff[2];
			if(feof(fp))
				return EBADIMG;
		}
	}

	// Jump to image data
	fseek(fp, fileheader.bfOffBits, SEEK_SET);
	
	if(feof(fp))
		return EBADIMG;
	
	// Create a node
	img = (struct RawImage*)malloc(sizeof(struct RawImage));
	if(img==NULL)
		return ENOMEMR;
	img->mimg = NULL;
	img->delay = 0;
	img->x = 0;
	img->y = 0;
	img->next = NULL;
	img->width = iw;
	img->height = ih;
	img->img = (unsigned char*)malloc(iw*(ih+1)*3);
	if(img->img==NULL) {
		free(img);
		return ENOMEMR;
	}

	lsize = (iw * bpp + 31)/32*4;
	switch(bpp) {
		case 1:
			for(y=0; y<ih; y++) {
				lpos = 0;

				for(i=0, x=0; i<iw/8; i++, x+=24) {
					c = getc(fp);
					if(feof(fp)) {
						free(img->img);
						free(img);
						return EBADIMG;
					}
					lpos++;

					for(bit=0x80, bc=0; bc<24; bit>>=1,bc+=3) {
						rgb[0] = clr[(c&bit)?1:0].r; 
						rgb[1] = clr[(c&bit)?1:0].g; 
						rgb[2] = clr[(c&bit)?1:0].b;
						memcpy(img->img + (ih-y)*iw*3 + x + bc, rgb, 3);
					}
				}

				while(lpos<lsize) {
					getc(fp);
					lpos++;
				}
			}
			break;
		case 4:
			for(y=0; y<ih; y++) {
				lpos = 0;

				for(i=0, x=0; i<iw/2; i++, x+=6) {
					c = getc(fp);
					if(feof(fp)) {
						free(img->img);
						free(img);
						return EBADIMG;
					}
					lpos++;

					// Nibble 1
					rgb[0] = clr[c&0x0f].r; 
					rgb[1] = clr[c&0x0f].g; 
					rgb[2] = clr[c&0x0f].b;
					memcpy(img->img + (ih-y)*iw*3 + x + 3, rgb, 3);
					
					// Nibble 2
					rgb[0] = clr[c>>4].r; 
					rgb[1] = clr[c>>4].g; 
					rgb[2] = clr[c>>4].b;
					memcpy(img->img + (ih-y)*iw*3 + x, rgb, 3);
				}

				while(lpos<lsize) {
					getc(fp);
					lpos++;
				}
			}
			break;
		case 8:
			for(y=0; y<ih; y++) {
				lpos = 0;

				for(i=0, x=0; i<iw; i++, x+=3) {
					c = getc(fp);
					if(feof(fp)) {
						free(img->img);
						free(img);
						return EBADIMG;
					}
					lpos++;
					rgb[0] = clr[c].r; rgb[1] = clr[c].g; rgb[2] = clr[c].b;
					memcpy(img->img + (ih-y)*iw*3 + x, rgb, 3);
				}

				while(lpos<lsize) {
					getc(fp);
					lpos++;
				}
			}
			break;
		case 24:
			for(y=0; y<ih; y++) {
				lpos = 0;

				for(i=0, x=0; i<iw; i++, x+=3) {
					rgb[2] = getc(fp); rgb[1] = getc(fp); rgb[0] = getc(fp);
					if(feof(fp)) {
						free(img->img);
						free(img);
						return EBADIMG;
					}
					lpos += 3;
					memcpy(img->img + (ih-y)*iw*3 + x, rgb, 3);
				}

				while(lpos<lsize) {
					getc(fp);
					lpos++;
				}
			}
			break;
	}

	_pimg_rawimglst_add(rimg, img);
		
	return 0;
}
