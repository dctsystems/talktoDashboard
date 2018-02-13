/***************************************\
 * $Log: viewer.c,v $
 * Revision 1.2  2000/11/26 18:48:50  ian
 * add wait parameter to Catch_X_Events
 *
 * Revision 1.1  2000/09/12 18:04:30  ian
 * Initial revision
 *
 * 
\***************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>

#include <assert.h>
#include <NCCAPixmap.h>
#include <PixDrawing.h>
#include <PixBlit.h>
#include <PixFileIO.h>

int main(int argc, char*argv[])
{
	NCCAPixmap img;
    char tmpFilename[1024];
    char realFilename[1024];
    int pid=getpid();
    sprintf(tmpFilename,"X%d.jpg",pid);
    sprintf(realFilename,"%d.jpg",pid);

	img=newPixmap(320,180,3,8);
	if(img.data==NULL)
		{
		exit(-1);
		}
    int i;
    for(i=0;i<60;i++)
    {
        NCCAPixel c;
        c.r=0.8;
        c.g=0.8;
        c.b=0.8;
        c.a=1;
        pixmapClearGrey(img,0.6);
        c.r=1;
        c.g=0;
        c.b=0;


	drawLine(img,0,0,319,0,c);
	drawLine(img,319,0,319,179,c);
	drawLine(img,319,179,0,179,c);
	drawLine(img,0,179,0,0,c);

        drawLine(img,160,90,160+80*sin((i*6.0/360.0)*2*M_PI),90-80*cos((i*6.0/360.0)*2*M_PI),c);
        savePixmap(img,tmpFilename);
        //Needed to so that the write is atomic!
        rename(tmpFilename,realFilename);
        sleep(1);
    }
	return 0;
}
