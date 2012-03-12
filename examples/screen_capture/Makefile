# Makefile
# By Santhosh N

CC= gcc
LIBS= -L/usr/X11R6/lib -lX11 -ljpeg -lXpm -lpng -lungif -lz -lm -lpimg
CFLAGS= -g

scrshot: pmain.o
	$(CC) $^ -o $@ $(LIBS)

# Clean
clean:
	rm -f *.o scrshot
