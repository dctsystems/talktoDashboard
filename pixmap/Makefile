# $Log: Makefile,v $
# Revision 1.6  2000/11/26 18:56:35  ian
# loads of changes ....
#
# Revision 1.5  2000/04/18 17:48:43  ian
# This is now totally(!) portable
#
# Revision 1.4  2000/04/18 11:51:26  ian
# Make portable, using the loopback to get the uname, remove WINOBJ
#
# Revision 1.3  2000/03/25 21:08:18  ian
# Make Pixmap a library
# add PixBlit, PixDrawing and Windowing
#
#Revision 1.2  99/07/04  17:11:33  ian
#added TIFFSUPPORT flag
#
# Revision 1.1  1998/02/23 14:47:39  istephen
# Initial revision
#

include ../makedefs/defs${PLATFORM}

CFLAGS=${SYSFLAGS} -I../include ${WINFLAGS}

loop : 
	${MAKE} PLATFORM=`uname` all

all : libNCCAPixmap.a
	cp libNCCAPixmap.a ../libs/${PLATFORM}
	${RANLIB} ../libs/${PLATFORM}/libNCCAPixmap.a

libNCCAPixmap.a : NCCAPixmap.o PixDrawing.o PixBlit.o PixFileIO.o Windowing.o PixelMaths.o MipMap.o Tiled.o PixText.o
	${AR} libNCCAPixmap.a NCCAPixmap.o PixDrawing.o PixBlit.o  PixFileIO.o Windowing.o PixelMaths.o MipMap.o Tiled.o PixText.o
	${RANLIB} libNCCAPixmap.a

Windowing.o : Windowing.c Win_X.c Win_MAC.c Win_NeXT.c Win_NT.c

clean :
	rm -f *.o *.x libNCCAPixmap.a
