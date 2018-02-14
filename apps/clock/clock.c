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
#include <time.h>

#include <assert.h>
#include <NCCAPixmap.h>
#include <PixDrawing.h>
#include <PixBlit.h>
#include <PixFileIO.h>

#ifdef LARGE
#define HEIGHT 360
#define WIDTH 320
#define LEN 150
#else
#define HEIGHT 180
#define WIDTH 320
#define LEN 80
#endif

#define SLEN (LEN*0.9)
#define MLEN (LEN*1.0)
#define HLEN (LEN*0.6)

int main(int argc, char*argv[])
{
	NCCAPixmap img;
    char tmpFilename[1024];
    char realFilename[1024];
    int pid=getpid();
    sprintf(tmpFilename,"X%d.jpg",pid);
    sprintf(realFilename,"%d.jpg",pid);

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
        pixmapClearGrey(img,0.6);

        c.r=1;
        c.g=0;
        c.b=0;
	drawLine(img,0,0,WIDTH-1,0,c);
	drawLine(img,WIDTH-1,0,WIDTH-1,HEIGHT-1,c);
	drawLine(img,WIDTH-1,HEIGHT-1,0,HEIGHT-1,c);
	drawLine(img,0,HEIGHT-1,0,0,c);


	time_t current_time;
	struct tm * time_info;

	time(&current_time);
	time_info = localtime(&current_time);

        c.r=0.8;
        c.g=0.8;
        c.b=0.8;
        drawLine(img,WIDTH/2,HEIGHT/2,
			WIDTH/2+SLEN*sin((time_info->tm_sec*6.0/360.0)*2*M_PI),
			HEIGHT/2-SLEN*cos((time_info->tm_sec*6.0/360.0)*2*M_PI),c);

        c.r=1;
        c.g=1;
        c.b=1;
        drawLine(img,WIDTH/2,HEIGHT/2,
			WIDTH/2+MLEN*sin((time_info->tm_min*6.0/360.0)*2*M_PI),
			HEIGHT/2-MLEN*cos((time_info->tm_min*6.0/360.0)*2*M_PI),c);


	float hAng=(time_info->tm_hour+(time_info->tm_min/60.0))*30.0/360.0;
        drawLine(img,WIDTH/2,HEIGHT/2,
			WIDTH/2+HLEN*sin(hAng*2*M_PI),
			HEIGHT/2-HLEN*cos(hAng*2*M_PI),c);


#ifdef LARGE
	//Put time across bottom in large mode?
	//strftime(SS_clockValue, 20, "%d/%m/%Y", time_info);
#endif
        savePixmap(img,tmpFilename);
        //Needed to so that the write is atomic!
        rename(tmpFilename,realFilename);
        sleep(1);
    }
	return 0;
}
