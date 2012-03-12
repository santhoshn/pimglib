SUBDIRS = formats

lib_LTLIBRARIES = libpimg.la
libpimg_la_SOURCES = plib.c zoom.c winimg.c subimage.c dither.c changecol.c scrshot.c
libpimg_la_LIBADD= $(ARM_LIBS) -ljpeg -lungif -lpng formats/libformats.la @X_LIBS@ -lX11 -lXpm
                                                                                                                
include_HEADERS = pimglib.h
