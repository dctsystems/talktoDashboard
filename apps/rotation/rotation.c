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
#include <string.h>

#include <assert.h>
#include <NCCAPixmap.h>
#include <PixDrawing.h>
#include <PixBlit.h>
#include <PixText.h>
#include <PixFileIO.h>

#ifdef LARGE
#define HEIGHT 360
#define WIDTH 320
#define LEN 140
#else
#define HEIGHT 180
#define WIDTH 320
#define LEN 80
#endif


int main(int argc, char*argv[])
{
	NCCAPixmap img;
    char tmpFilename[1024];
    char realFilename[1024];
    int pid=getpid();
    sprintf(tmpFilename,"/tmp/LIVE/X%d.jpg",pid);
    sprintf(realFilename,"/tmp/LIVE/%d.jpg",pid);

	img=newPixmap(WIDTH,HEIGHT,3,8);
	if(img.data==NULL)
		{
		exit(-1);
		}

    FILE *f=popen("mosquitto_sub -t rotation","r");

    while(1)
    {
        NCCAPixel c;
        c.r=0.8;
        c.g=0.8;
        c.b=0.8;
        c.a=1;
        pixmapClearGrey(img,1);

        c.r=1;
        c.g=0;
        c.b=0;
	drawLine(img,0,0,WIDTH-1,0,c);
	drawLine(img,WIDTH-1,0,WIDTH-1,HEIGHT-1,c);
	drawLine(img,WIDTH-1,HEIGHT-1,0,HEIGHT-1,c);
	drawLine(img,0,HEIGHT-1,0,0,c);


        c.r=1.0;
        c.g=0.0;
        c.b=0.0;

	char fortuneString[4096];

	if(!fgets(fortuneString,4096,f))
		exit(1);
	//puts(fortuneString);

	float angle=atof(fortuneString);
	//angle*=-1;
	angle+=45;

	int i;
	for(i=0;i<4;i++)
		{
		float x1,y1;
		float x2,y2;
		x1=(WIDTH/2);
		y1=(HEIGHT/2);
		x2=x1+LEN*cos(angle/360*2*M_PI);
		y2=y1+LEN*sin(angle/360*2*M_PI);
		drawLine(img,x1,y1, x2,y2,c);
		drawLine(img,x1+1,y1, x2+1,y2,c);
		drawLine(img,x1,y1+1, x2,y2+1,c);
		angle+=90;
		c.r=0.0;
		}


        savePixmap(img,tmpFilename);
        //Needed to so that the write is atomic!
        rename(tmpFilename,realFilename);
    }
    fclose(f);
    return 0;
}
