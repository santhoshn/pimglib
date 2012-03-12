# Makefile
# By Santhosh N

CC= gcc
LIBS= -L/usr/X11R6/lib -lX11 -ljpeg -lXpm -lpng -lungif -lz -lm
CFLAGS= -g

palbum: pmain.o
	$(CC) $^ -o $@ $(LIBS) -L../src/.libs -lpimg

# Clean
clean:
	rm -f *.o palbum
