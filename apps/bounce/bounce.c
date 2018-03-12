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

#ifdef LARGE
#define HEIGHT 360
#define WIDTH 640
#else
#define HEIGHT 180
#define WIDTH 320
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


    float startX=100;
    float startY=101;
    float endX=200;
    float endY=150;
    float deltaStartX=2;
    float deltaStartY=4.1;
    float deltaEndX=-2.6;
    float deltaEndY=-1.88;
    NCCAPixel bg={0.0,0.0,0.0,1};
    NCCAPixel fg={1,0,0,1};
    NCCAPixel deltaFg={0.01,0.02,0.015,0.0};
    NCCAPixel deltaBg={0.01,0.02,0.015,0.0};

#ifdef LARGE
	deltaStartX*=2;
	deltaStartY*=2;
	deltaEndX*=2;
	deltaEndY*=2;
#endif
    while(1)
    {

	if(random()%100==1)
		{
		int x,y;
		for(x=0;x<WIDTH;x++)
			for(y=0;y<HEIGHT;y++)
				{
				setPixelColor(img,x,y,bg);

				}
		bg.r+=deltaBg.r;
		if(bg.r<0 || bg.r>1)
			deltaBg.r*=-1;
		bg.g+=deltaBg.g;
		if(bg.g<0 || bg.g>1)
			deltaBg.g*=-1;
		bg.b+=deltaBg.b;
		if(bg.b<0 || fg.b>1)
			deltaBg.b*=-1;
		}

        fg.r+=deltaFg.r;
	if(fg.r<0 || fg.r>1)
		deltaFg.r*=-1;
        fg.g+=deltaFg.g;
	if(fg.g<0 || fg.g>1)
		deltaFg.g*=-1;
        fg.b+=deltaFg.b;
	if(fg.b<0 || fg.b>1)
		deltaFg.b*=-1;



	startX+=deltaStartX;
	if(startX<0 || startX>WIDTH)
		deltaStartX*=-1;

	startY+=deltaStartY;
	if(startY<0 || startY>HEIGHT)
		deltaStartY*=-1;

	endX+=deltaEndX;
        if(endX<0 || endX>WIDTH)
                deltaEndX*=-1;

	endY+=deltaEndY;
        if(endY<0 || endY>HEIGHT)
                deltaEndY*=-1;

	drawLine(img,startX,startY,endX,endY,fg);


	drawLine(img,0,0,WIDTH-1,0,fg);
	drawLine(img,WIDTH-1,0,WIDTH-1,HEIGHT-1,fg);
	drawLine(img,WIDTH-1,HEIGHT-1,0,HEIGHT-1,fg);
	drawLine(img,0,HEIGHT-1,0,0,fg);

        savePixmap(img,tmpFilename);

        //Needed to so that the write is atomic!
        rename(tmpFilename,realFilename);
        usleep(400000);
    }
	return 0;
}
