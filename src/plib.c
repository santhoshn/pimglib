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

unsigned _rshft=0, _gshft=0, _bshft=0, _rmask=0, _gmask=0, _bmask=0, _rbits=0, _gbits=0, _bbits=0, _dbpp=0;

int _get_visinfo(Display *disp)
{
	XVisualInfo *vi_in=NULL, vi_out;
     int nvis, i, j, n, ret=0;
     unsigned depth, tmp;

	if(!_rshft && !_gshft && !_bshft) {
		vi_out.screen = DefaultScreen(disp);
		vi_out.depth = DefaultDepth(disp, vi_out.screen);
	     vi_in = XGetVisualInfo(disp, VisualScreenMask|VisualDepthMask, &vi_out, &nvis);

		if(vi_in) {
			for(i=0; i<nvis; i++) {
				XPixmapFormatValues *pf = XListPixmapFormats(disp, &n);
				if(!pf)
					continue;
				
				depth = vi_in[i].depth;
				_dbpp = 0;
				for(j=0; j<n; j++) {
					if(pf[j].depth == depth) {
						_dbpp = pf[j].bits_per_pixel;
						break;
					}
				}

				if(j==n)
					continue;

				_rmask = vi_in[i].red_mask;
				for(_rshft=0, tmp=_rmask; !(tmp&0x1); _rshft++, tmp>>=1);
				for(_rbits=0, tmp=_rmask; tmp; _rbits++) tmp &= tmp-1;
			
				_gmask = vi_in[i].green_mask;
				for(_gshft=0, tmp=_gmask; !(tmp&0x1); _gshft++, tmp>>=1);
				for(_gbits=0, tmp=_gmask; tmp; _gbits++) tmp &= tmp-1;
				
				_bmask = vi_in[i].blue_mask;
				for(_bshft=0, tmp=_bmask; !(tmp&0x1); _bshft++, tmp>>=1);
				for(_bbits=0, tmp=_bmask; tmp; _bbits++) tmp &= tmp-1;
				
				break;
			}
     	} else
			ret = 1;
	}

	if(vi_in)
		XFree(vi_in);

	return ret;
}

void _pimg_imglst_add(struct Image **start, struct Image *node)
{
	struct Image *t;

	t = *start;

	if(t==NULL) {
		*start = node;
	} else {
		while(t->next != NULL)
			t = t->next;
		t->next = node;
	}
}

void _pimg_rawimglst_add(struct RawImage **start, struct RawImage *node)
{
	struct RawImage *t;

	t = *start;

	if(t==NULL) {
		*start = node;
	} else {
		while(t->next != NULL)
			t = t->next;
		t->next = node;
	}
}

int pimg_raw_moveto_x(Display *disp, struct RawImage **rimg, struct Image **ximg)
{
	struct RawImage *t, *q;
	struct Image *img;
	int depth, x, y, i, p, llen;
	unsigned char r, g, b, *tmp;

	depth = DefaultDepth(disp, DefaultScreen(disp));

	if(_get_visinfo(disp))
		return EBADDEP;

     t = *rimg;
	*rimg = NULL;
	while(t) {
		if(_dbpp<=16) {
			for(y=0; y < t->height; y++) {
				for(x=0, i=0; x < t->width*3; i+=2, x+=3) {
					llen = y*t->width*3+x;
					r = t->img[llen]; g = t->img[llen+1]; b = t->img[llen+2];
					p = ((r>>(8-_rbits))<<_rshft) + ((g>>(8-_gbits))<<_gshft) + ((b>>(8-_bbits))<<_bshft);
					memcpy(t->img + y*t->width*2 + i, &p, 2);
				}
			}
		
			tmp = (unsigned char*)realloc(t->img, t->width*(t->height+1)*2);
			if(tmp)
				t->img = tmp;
		} else {
			tmp = (unsigned char*)realloc(t->img, t->width*(t->height+1)*4);
			if(tmp)
				t->img = tmp;
			else
				return ENOMEMR;
		
			for(y=t->height-1; y>=0; y--) {
				for(x=(t->width-1)*3, i=(t->width-1)*4; x>=0; i-=4, x-=3) {
					llen = y*t->width*3+x;
					r = 0; g = 0xff; b = 0xff;
					r = t->img[llen]; g = t->img[llen+1]; b = t->img[llen+2];
					p = ((r>>(8-_rbits))<<_rshft) + ((g>>(8-_gbits))<<_gshft) + ((b>>(8-_bbits))<<_bshft);
					memcpy(t->img + y*t->width*4 + i, &p, 4);
				}
			}
		}

		// Create the Image and add to the list
		img = (struct Image*)malloc(sizeof(struct Image));
		img->info = t->info;
		img->delay = t->delay;
		img->x = t->x;
		img->y = t->y;
		img->width = t->width;
		img->height = t->height;
		img->next = NULL;
		img->img = img->mimg = NULL;
		
		img->img = XCreateImage(disp, CopyFromParent, depth, ZPixmap, 0,\
							(char*)t->img, img->width, img->height, _dbpp, img->width*(_dbpp>>3));
	
		if(t->mimg) {
		}

		_pimg_imglst_add(ximg, img);

		q = t;
		t = t->next;
		free(q);
	}

	return 0;
}

int pimg_raw_copyto_x(Display *disp, struct RawImage **rimg, struct Image **ximg)
{
	/*
	 * TODO
	 */
	return 0;
}

int pimg_raw_save(char *filename, struct RawImage **img, int image_format, unsigned quality)
{
	if(image_format==FORMAT_JPEG) {
		if(quality<1 || quality>100)
			return EBADVAL;
		return _save_jpeg_rawimage(filename, img, quality);
	}

	return ENOSUPP;
}

int pimg_load(char *file, Display *disp, struct Image **ximg, unsigned short iattr)
{
	unsigned char buff[15];
	int ret=ENOTIMG, tret=0;
	unsigned depth;
	FILE *fp;
	struct RawImage *rimg=NULL;

	depth = DefaultDepth(disp, DefaultScreen(disp));
			
	*ximg = NULL;

	fp = fopen(file, "rb");
	if(fp==NULL)
		return ENOFILE;

	if(_get_visinfo(disp))
		return EBADDEP;
	//printf("rs = %d gs = %d bs = %d rc = %d gc = %d bc = %d rm = %x gm = %x bm = %x dbpp = %d\n", _rshft, _gshft, _bshft, 
	//											_rbits, _gbits, _bbits, _rmask, _gmask, _bmask, _dbpp);

	// Read the initial few bytes, to figure out the image format
	fread(buff, 1, 9, fp);
	rewind(fp);

	switch(buff[0]) {
		case 0xff:	// For JPEG
			if(buff[1]==0xd8)
				ret = _load_jpeg_rawimage(fp, &rimg);

			if(ret==0) {
				ret = FORMAT_JPEG;
				if(iattr&DITHER)
					pimg_raw_dither(depth, &rimg);
				tret = pimg_raw_moveto_x(disp, &rimg, ximg);
				if(tret<0) {
					pimg_raw_free(&rimg);
					ret = tret;
				}
			}

			break;
		case 0x89:	// For PNG
			if(buff[1]=='P' && buff[2]=='N' && buff[3]=='G')
				ret = _load_png_rawimage(fp, &rimg, iattr);
			
			if(ret==0) {
				ret = FORMAT_PNG;
				if(iattr&DITHER)
					pimg_raw_dither(depth, &rimg);
				tret = pimg_raw_moveto_x(disp, &rimg, ximg);
				if(tret<0) {
					pimg_raw_free(&rimg);
					ret = tret;
				}
			}
			
			break;
		case 'B':		// For BMP
			buff[2] = 0;
			if(!strcmp("BM", (const char*)buff))
				ret = _load_bmp_rawimage(fp, &rimg);
			
			if(ret==0) {
				ret = FORMAT_BMP;
				if(iattr&DITHER)
					pimg_raw_dither(depth, &rimg);
				tret = pimg_raw_moveto_x(disp, &rimg, ximg);
				if(tret<0) {
					pimg_raw_free(&rimg);
					ret = tret;
				}
			}
			
			break;
		case 'G':		// For GIF
			buff[4] = 0;
			if(!strcmp("GIF8", (const char*)buff))
				ret = load_gif_image(fp, disp, ximg, iattr);
			
			if(ret==0)
				ret = FORMAT_GIF;
			
			break;
		case '/':		// For XPM
			buff[9] = 0;
			if(!strcmp("/* XPM */", (const char*)buff))
				ret = load_xpm_image(file, fp, disp, ximg, iattr);
			
			if(ret==0)
				ret = FORMAT_XPM;
			
			break;
		default:
			ret = ENOSUPP;
			break;
	}

	fclose(fp);

	return ret;
}

int pimg_save(char *filename, Display *disp, struct Image **img, int image_format, unsigned quality)
{
	if(_get_visinfo(disp))
		return EBADDEP;

	if(image_format==FORMAT_JPEG) {
		if(quality<1 || quality>100)
			return EBADVAL;
		return _save_jpeg_image(filename, disp, img, quality);
	}

	if(image_format==FORMAT_PNG) {
		return _save_png_image(filename, disp, img);
	}

	if(image_format==FORMAT_PNM) {
		return _save_pnm_image(filename, disp, img);
	}

	if(image_format==FORMAT_PGM) {
		return _save_pgm_image(filename, disp, img);
	}

	return ENOSUPP;
}

void pimg_free(struct Image **ximg)
{
	struct Image *t;

	while(*ximg!=NULL) {
		t = *ximg;
		*ximg = (*ximg)->next;
		XDestroyImage(t->img);
		if(t->mimg!=NULL)
			XDestroyImage(t->mimg);
		free(t);
	}
}

int pimg_raw_load(char *file, struct RawImage **rimg, unsigned short iattr)
{
	unsigned char buff[15];
	int ret=ENOTIMG;
	FILE *fp;

	*rimg = NULL;

	fp = fopen(file, "rb");
	if(fp==NULL)
		return ENOFILE;

	// Read the initial few bytes, to figure out the image format
	fread(buff, 1, 9, fp);
	rewind(fp);

	switch(buff[0]) {
		case 0xff:	// For JPEG
			if(buff[1]==0xd8)
				ret = _load_jpeg_rawimage(fp, rimg);

			if(ret==0)
				ret = FORMAT_JPEG;

			break;
		case 0x89:	// For PNG
			if(buff[1]=='P' && buff[2]=='N' && buff[3]=='G')
				ret = _load_png_rawimage(fp, rimg, iattr);
			
			if(ret==0)
				ret = FORMAT_PNG;
			
			break;
		case 'B':		// For BMP
			buff[2] = 0;
			if(!strcmp("BM", (const char*)buff))
				ret = _load_bmp_rawimage(fp, rimg);
			
			if(ret==0)
				ret = FORMAT_BMP;
			
			break;
		default:
			ret = ENOSUPP;
			break;
	}

	fclose(fp);

	return ret;
}

void pimg_raw_free(struct RawImage **rimg)
{
	struct RawImage *t;

	while(*rimg!=NULL) {
		t = *rimg;
		*rimg = (*rimg)->next;
		if(t->img);
			free(t->img);
		if(t->mimg)
			free(t->mimg);
		free(t);
	}
}

char *pimg_errmsg(int err_val)
{
	switch(err_val) {
		case EBADIMG:
			return "Bad Image";
		case EBIGIMG:
			return "Big Image";
		case ENOTIMG:
			return "Not an Image";
		case ENOFILE:
			return "File does not exist";
		case ENOMEMR:
			return "No enough memory";
		case ENOSUPP:
			return "Image format not supported";
		case EBADDEP:
			return "Invalid display depth";
		default:
			return "Unknown error value";
	}
}
