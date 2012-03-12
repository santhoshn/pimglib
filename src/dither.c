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

static unsigned char dmatrix4[4][4] = {
	{0, 8, 2, 10},
	{12, 4, 14, 6},
	{3, 11, 1, 9},
	{15, 7, 13, 5}
};

//#define dither_int(component, value, shift) ((((241*component)>>8)+value)>>shift)
static unsigned char dither_int(unsigned short value, unsigned short component, unsigned shift)
{
	return ((241*component+(value<<8))>>8) >> shift;
}

int pimg_raw_dither(int depth, struct RawImage **rimg)
{
	int i, j, width, height;
	unsigned char *tbuff;

	if(depth!=4 && depth!=12 && depth!=16)
		return EBADDEP;

	tbuff = (*rimg)->img;
	width = (*rimg)->width;
	height = (*rimg)->height;
	
	switch(depth) {
		case 4:
			for(i=0;i<height;i++) {
				for(j=0;j<width;j++) {
					tbuff[0] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[0], 4) << 4;
					tbuff[1] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[1], 4) << 4;
					tbuff[2] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[2], 4) << 4;
					tbuff+=3;
				}
			}
			break;
		case 12:
			for(i=0;i<height;i++) {
				for(j=0;j<width;j++) {
					tbuff[0] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[0], 4) << 4;
					tbuff[1] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[1], 4) << 4;
					tbuff[2] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[2], 4) << 4;
					tbuff+=3;
				}
			}
			break;
		case 16:
			for(i=0;i<height;i++) {
				for(j=0;j<width;j++) {
#ifdef __arm__
					tbuff[0] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[0], 2) << 2;
					tbuff[1] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[1], 3) << 3;
					tbuff[2] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[2], 3) << 3;
#else
					tbuff[0] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[0], 3) << 3;
					tbuff[1] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[1], 2) << 2;
					tbuff[2] = dither_int(dmatrix4[i&0x3][j&0x3], tbuff[2], 3) << 3;
#endif
					tbuff+=3;
				}
			}
			break;
	}

	return 0;
}
