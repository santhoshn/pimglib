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

#include "pimg_gif.h"

int load_gif_image(FILE *fp, Display *disp, struct Image **simg, unsigned short iattr)
{
	GifFileType *gif;
	GifRecordType rec;
	GifPixelType *row=NULL;
	ColorMapObject *cmap;
	int transpval=-1, x, y, ioff[]={0,4,2,1}, ijmp[]={8,8,4,2}, tdisp;
	unsigned int t, il, it, iw, ih, i, niter, cn, delay=0, depth, mbytes, bytes, mbpl=0;
	unsigned char r, g, b, *ibuff=NULL, *imask=NULL;
	struct Image *img=NULL;
	unsigned char bm[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

	depth = DefaultDepth(disp, DefaultScreen(disp));

	gif = DGifOpenFileHandle(fileno(fp));
	if(gif==NULL)
		return EBADIMG;

	if(gif->SWidth<1 || gif->SHeight<1) {
		DGifCloseFile(gif);
		return EBADIMG;
	}

	// Process GIF records one by one
	do {
		if(DGifGetRecordType(gif, &rec)==GIF_ERROR) {
			DGifCloseFile(gif);
			return EBADIMG;
		}

		if(rec==IMAGE_DESC_RECORD_TYPE) {
			if(DGifGetImageDesc(gif)==GIF_ERROR) {
				DGifCloseFile(gif);
				return EBADIMG;
			}

			iw = gif->Image.Width;
			ih = gif->Image.Height;
			il = gif->Image.Left;
			it = gif->Image.Top;

			// Compare the image dimension for maximum limit
			if(iw+il > gif->SWidth || ih+it > gif->SHeight || iw*ih > MAX_WIDTH*MAX_HEIGHT) {
				DGifCloseFile(gif);
				return EBADIMG;
			}

			// Get the colormap
			cmap = (gif->Image.ColorMap)?gif->Image.ColorMap:gif->SColorMap;

			if(row)
				free(row);
			row = malloc(iw*sizeof(GifColorType));
			if(row==NULL) {
				DGifCloseFile(gif);
				return ENOMEMR;
			}

			ibuff = (unsigned char*)malloc(iw*(ih+1)*(_dbpp>>3));
			if(ibuff==NULL) {
				free(row);
				DGifCloseFile(gif);
				return ENOMEMR;
			}
	
			// Allocate memory for the image mask
			if(iattr & SHAPE_MASK) {
				mbpl = iw/8+((iw%8)?1:0);
				mbytes = mbpl*ih;
				imask = (unsigned char*)malloc(mbytes);
				memset(imask, 0x00, mbytes);
			}

			// Create a node
			img = (struct Image*)malloc(sizeof(struct Image));
			img->next = NULL;
			img->delay = delay;
			img->x = il;
			img->y = it;
			img->width = iw;
			img->height = ih;
			img->img = img->mimg = NULL;

			if(gif->Image.Interlace) {
				niter = 4;
			} else {
				niter = 1;
				ioff[0] = 0;
				ijmp[0] = 1;
			}

			bytes = _dbpp>>3;
			for(i=0; i<niter; i++) {
				for(y=ioff[i]; y<ih; y+=ijmp[i]) {
					if(DGifGetLine(gif, row, iw)==GIF_ERROR) {
						 free(row);
						 free(img);
						 free(ibuff);
						 if(iattr & SHAPE_MASK)
							free(imask);
						 DGifCloseFile(gif);
						 return EBADIMG;
					}
			
					for(x=0; x<iw; x++) {
						cn = row[x];
						r=cmap->Colors[cn].Red; g=cmap->Colors[cn].Green; b=cmap->Colors[cn].Blue;
						
						t = ((r>>(8-_rbits))<<_rshft) + ((g>>(8-_gbits))<<_gshft) + ((b>>(8-_bbits))<<_bshft);
						
						memcpy(ibuff + y*iw*bytes + x*bytes, &t, bytes);
					    
						// Write to the shape mask
						if((iattr&SHAPE_MASK) && cn!=transpval)
							*(imask + y*mbpl + x/8) |= bm[x%8];
					}
				}
			}

			// Add the Image node to the list
			img->img = XCreateImage(disp, CopyFromParent, depth, ZPixmap, 0,\
							ibuff, img->width, img->height, _dbpp, img->width*(_dbpp>>3));
			
			// If image shape mask required, put it
			if(iattr & SHAPE_MASK) {
				img->mimg = XCreateImage(disp, CopyFromParent, 1, ZPixmap, 0,\
                                        imask, img->width, img->height, 8, mbpl);
				img->mimg->red_mask = _rmask;
				img->mimg->green_mask = _gmask;
				img->mimg->blue_mask = _bmask;
			}
			
			_pimg_imglst_add(simg, img);
		} else if(rec==EXTENSION_RECORD_TYPE) {
			int ecode;
			GifByteType *ext=NULL;

			DGifGetExtension(gif, &ecode, &ext);
			if(ecode==GRAPHICS_EXT_FUNC_CODE) {
				// Block Size should be 4
				if(ext[0]==4) {
					// Read delay 
					delay = (ext[3]<<8)|ext[2];
				
					// Read disposal method
					tdisp = (ext[1]&0x1c)>>2;

					// Read transperant value, if any
					transpval = -1;
					if(ext[1]&1) {
						transpval = (int)ext[4];
					}
				}	
			}
			
			while(ext) {
				if(DGifGetExtensionNext(gif, &ext)==GIF_ERROR) {
					DGifCloseFile(gif);
					return EBADIMG;
				}
			}
		}
	} while(rec!=TERMINATE_RECORD_TYPE);

	if(row!=NULL)
		free(row);
	DGifCloseFile(gif);

	return 0;
}
