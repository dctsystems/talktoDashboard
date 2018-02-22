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
#include <sys/stat.h>

#include <assert.h>
#include <NCCAPixmap.h>
#include <PixDrawing.h>
#include <PixBlit.h>
#include <PixFileIO.h>

#ifdef LARGE
const char *cmd="avconv -loglevel panic -i $HOME/PreludeS.mp4 -f image2 -r 10 -s 640x360";
#else
const char *cmd="avconv -loglevel panic -i $HOME/PreludeS.mp4 -f image2 -r 10 -s 320x180";
#endif




int main(int argc, char*argv[])
{
    char tmpFilename[1024];
    char realFilename[1024];
    int pid=getpid();
    sprintf(realFilename,"/tmp/LIVE/%d.jpg",pid);

    char tmpDir[1024];
    sprintf(tmpDir,"/tmp/LIVE/X%d",pid);
    mkdir(tmpDir,0777);

    
    char fullCommand[1024];
    sprintf(fullCommand,"%s %s/%%d.jpg &",cmd,tmpDir);
    system(fullCommand);
    
    sleep(2); 

    int frame;
    frame=1;
    while(1)
    {
	sprintf(tmpFilename,"%s/%d.jpg",tmpDir,frame);
	//puts(tmpFilename);
        int err=rename(tmpFilename,realFilename);
	if(err)
		{
		rmdir(tmpDir);		
		exit(0);
		}
	frame++;
        usleep(100000);
    }
	return 0;
}
