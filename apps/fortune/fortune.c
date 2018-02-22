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
#define WIDTH 1280
#define FONT 2
#define SKIP 25
#else
#define HEIGHT 180
#define WIDTH 640
#define FONT 3
#define SKIP 18
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


        c.r=0.0;
        c.g=0.0;
        c.b=0.0;

	//Put time across bottom in large mode?
	char fortuneString[4096];
	selectFont(DCTFonts[FONT]);
	FILE *f=popen("fortune","r");
	int y=SKIP;
	while(fgets(fortuneString,4096,f))
		{
		int i;
		for(i=0;i<strlen(fortuneString);i++)
			{
			if(fortuneString[i]<' ')
				fortuneString[i]=' ';
			}
		blitString(img,fortuneString,SKIP,y,c);
		y+=SKIP;
		}
	fclose(f);


        savePixmap(img,tmpFilename);
        //Needed to so that the write is atomic!
        rename(tmpFilename,realFilename);
        sleep(60);
    }
	return 0;
}
