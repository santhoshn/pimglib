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
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/xpm.h>

#include <pimglib.h>

Display *disp;
struct Image *simg;

void show_usage()
{
	fprintf(stderr, "Usage: 	scrshot [-f 0/1 -h]\n");
	fprintf(stderr, "		-f 	0 - PNG(Default)\n");
	fprintf(stderr, "			1 - JPEG\n");
}

int main(int ac, char **av)
{
	int format=0, c;

	//Parse the commandline options
	opterr=0;
	while((c=getopt(ac, av, "f:h"))!=-1) {
     	switch(c) {
			case 'f':
				format=atoi(optarg);
				if(format!=0 && format!=1) {
					show_usage();
					exit(1);
				}
				break;
			case 'h':
				show_usage();
				exit(1);
			case '?':
				fprintf(stdout, "Unknown option `-%c'. Use -h option for help.\n", optopt);
				fflush(stdout);
				exit(1);
		      default:
				abort();
		}
	}

	// Open display
	disp = XOpenDisplay(NULL);
	if(disp==NULL) {
		fprintf(stderr,"Error opening display.\n");
		exit(1);
	}

	pimg_scrshot(disp, &simg);

	if(format==0) {
		pimg_save("/tmp/scrshot.png", disp, &simg, FORMAT_PNG, 100);
		printf("Screenshot saved as /tmp/scrshot.png\n");
	} else if(format==1) {
		pimg_save("/tmp/scrshot.jpg", disp, &simg, FORMAT_JPEG, 85);
		printf("Screenshot saved as /tmp/scrshot.jpg\n");
	} else {
		show_usage();
	}
	
	XCloseDisplay(disp);
	
	return 0;
}
