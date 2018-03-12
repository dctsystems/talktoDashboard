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

    FILE *f=popen("mosquitto_sub -t button","r");

    while(1)
    {
	char fortuneString[4096];

	if(!fgets(fortuneString,4096,f))
		exit(1);
	//puts(fortuneString);

	float val=atof(fortuneString);

        NCCAPixel c;
		c.r=1-val;
		c.g=0.0;
		c.b=0.0;
		c.a=1;

	int i;
	for(i=0;i<HEIGHT;i++)
		{
		drawLine(img,0,i,WIDTH,i,c);
		}

        c.r=1;
        c.g=0;
        c.b=0;
	drawLine(img,0,0,WIDTH-1,0,c);
	drawLine(img,WIDTH-1,0,WIDTH-1,HEIGHT-1,c);
	drawLine(img,WIDTH-1,HEIGHT-1,0,HEIGHT-1,c);
	drawLine(img,0,HEIGHT-1,0,0,c);


        savePixmap(img,tmpFilename);
        //Needed to so that the write is atomic!
        rename(tmpFilename,realFilename);
    }
    fclose(f);
    return 0;
}
