#$Log: Makefile,v $
#Revision 1.2  2000/11/11 18:21:35  ian
#add upm
#
#Revision 1.1  2000/09/12 18:04:30  ian
#Initial revision
#
#

TARGET=cheerlights

include ../../makedefs/defs${PLATFORM}


INCLUDES=-I../../pixmap
LIBS=  -L../../libs/${PLATFORM} -L/usr/local/lib -lNCCAPixmap -ltiff -ljpeg ${EXRLIB} 

CFLAGS= ${SYSFLAGS} ${WINFLAGS} ${INCLUDES} 

loop :
	make PLATFORM=`uname` all

all:	${TARGET} ${TARGET}-large
	./${TARGET}


${TARGET} : ${TARGET}.o
	${CCLINK} ${CFLAGS} -o ${TARGET} \
		${TARGET}.o\
		${LIBS} ${WINLIBS} ${MATHLIB}

${TARGET}-large : ${TARGET}-large.o
	${CCLINK} ${CFLAGS} -o ${TARGET}-large \
		${TARGET}-large.o\
		${LIBS} ${WINLIBS} ${MATHLIB}

${TARGET}-large.o : ${TARGET}.c
	${CC} ${CFLAGS} -DLARGE -c ${TARGET}.c -o ${TARGET}-large.o
clean :
	rm -f *.o *.exe ${TARGET} ${TARGET}-large


