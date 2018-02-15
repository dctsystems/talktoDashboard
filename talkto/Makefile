#$Log: Makefile,v $
#Revision 1.2  2000/11/11 18:21:35  ian
#add upm
#
#Revision 1.1  2000/09/12 18:04:30  ian
#Initial revision
#
#

include ../makedefs/defs${PLATFORM}


INCLUDES=-I../pixmap
LIBS=  -L../libs/${PLATFORM} -L/usr/local/lib -lNCCAPixmap -ltiff -lgif -ljpeg ${EXRLIB} 

CFLAGS= ${SYSFLAGS} ${WINFLAGS} ${INCLUDES} 

loop :
	make PLATFORM=`uname` all

all:	talkto 
	./talkto


talkto : talkto.o framebuffer.o
	${CCLINK} ${CFLAGS} -o talkto \
		talkto.o framebuffer.o\
		${LIBS} ${WINLIBS} ${MATHLIB}

clean :
	rm -f *.o *.exe talkto 


