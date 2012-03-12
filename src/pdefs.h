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
#include <X11/Xlib.h>
#include <string.h>
#include "pimglib.h"

#define MAX_WIDTH 	3264
#define MAX_HEIGHT 	2448

extern unsigned _rshft, _gshft, _bshft, _dbpp;
extern unsigned _rmask, _gmask, _bmask, _rbits, _gbits, _bbits;

int _get_visinfo(Display *disp);
void _pimg_imglst_add(struct Image **start, struct Image *node);
void _pimg_rawimglst_add(struct RawImage **start, struct RawImage *node);
