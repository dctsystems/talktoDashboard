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
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <dirent.h>


#include <assert.h>
#include <NCCAPixmap.h>
#include <PixBlit.h>
#include <PixDrawing.h>
#include <PixFileIO.h>


#ifdef WINDOWING
#include "Windowing.h"
#else
#include "framebuffer.h"
#define windowInit FBwindowInit
#define screenDraw FBscreenDraw
#endif

#include "watch.h"

#define BlockSizeX 320
#define BlockSizeY 180

//Limit screen size to 1080p
#define MAXBLOCKS 6
int blockPID[MAXBLOCKS][MAXBLOCKS];
//use 4x4 for testing in Windowed mode
int BlocksX=4;
int BlocksY=4;

#define DIRECTORY "/tmp/LIVE"

NCCAPixmap frameBuffer;

void resetAll()
{
    int x,y;
    for(y=0;y<BlocksY;y++)
        for(x=0;x<BlocksX;x++)
            blockPID[x][y]=0;
}

void garbageCollect()
{
    //Free up areas of the screen which don't have image files any more
    int x,y;
    for(y=0;y<BlocksY;y++)
        for(x=0;x<BlocksX;x++)
        {
            if(blockPID[x][y])
            {
                char filename[1024];
                sprintf(filename,"%s/%d.jpg",DIRECTORY,blockPID[x][y]);
                if( access( filename, F_OK ) ==0 )
                    continue;
                sprintf(filename,"%s/%d.tiff",DIRECTORY,blockPID[x][y]);
                if( access( filename, F_OK ) ==0 )
                    continue;
                sprintf(filename,"%s/%d.bmp",DIRECTORY,blockPID[x][y]);
                if( access( filename, F_OK ) ==0 )
                    continue;
                blockPID[x][y]=0;
            }
       }
}
void draw(int pid,char *filename)
{
    int x,y;
    NCCAPixmap img;
    //printf("draw %s \n",filename);
    img=loadPixmap(filename);
    int imgW=(img.width+(BlockSizeX-1))/BlockSizeX;
    int imgH=(img.height+(BlockSizeY-1))/BlockSizeY;
    //Just in case its not an exact block size...
    int xPos=BlockSizeX*imgW*0.5-img.width*0.5;
    int yPos=BlockSizeY*imgH*0.5-img.height*0.5;

    if(img.data==NULL)
    {
        //This can happen - we have a race condition, when files are deleted just afer they're scanned
        //That's OK!
        printf("load (%s) failed",filename);
        return;;
    }

    //Stage 1 - if we've drawn it before, draw it in the same place...
        for(y=0;y<BlocksY;y++)
            for(x=0;x<BlocksX;x++)
           {
               if(blockPID[x][y]==pid)
               {
	 	   xPos+=x*BlockSizeX;
	 	   yPos+=y*BlockSizeY;
                   pixmapInsert(img,xPos,yPos,frameBuffer);
                   destroyPixmap(img);
                   return;
               }
           }

    //Stage 2 - find a new space big enough...

    for(y=0;y<BlocksY+1-imgH;y++)
        for(x=0;x<BlocksX+1-imgW;x++)
        {
            int deltaX,deltaY;
            int goodBlock=1;
            for(deltaX=0;deltaX<imgW;deltaX++)
                for(deltaY=0;deltaY<imgH;deltaY++)
                    if(blockPID[x+deltaX][y+deltaY]!=0)
                        goodBlock=0;
            if(goodBlock)
            {
                for(deltaX=0;deltaX<imgW;deltaX++)
                    for(deltaY=0;deltaY<imgH;deltaY++)
                        blockPID[x+deltaX][y+deltaY]=pid;
	 	   xPos+=x*BlockSizeX;
	 	   yPos+=y*BlockSizeY;
                   pixmapInsert(img,xPos,yPos,frameBuffer);
                destroyPixmap(img);
                //printf("draw %s at %d,%d\n",filename,x,y);
                return;
            }
        }

    //Stage 3 - No space found... Clear up dead spaces
    printf("Failed to find space for %s size:%d,%d\n",filename,imgW,imgH);
    resetAll();
    destroyPixmap(img);
    draw(pid,filename);
}

void drawGrid()
{
    pixmapClearGrey(frameBuffer,0.3);
    
    int x,y;
    NCCAPixel c={0.28,0.28,0.28,1};
    
    for(y=0;y<BlocksY;y++)
        for(x=0;x<BlocksX;x++)
        {
            {
                drawLine(frameBuffer,
                         x*BlockSizeX,y*BlockSizeY,
                         (x+1)*BlockSizeX-1,y*BlockSizeY,
                         c);
                drawLine(frameBuffer,
                         (x+1)*BlockSizeX-1,y*BlockSizeY,
                         (x+1)*BlockSizeX-1,(y+1)*BlockSizeY-1,
                         c);
                drawLine(frameBuffer,
                         (x+1)*BlockSizeX-1,(y+1)*BlockSizeY-1,
                         (x+0)*BlockSizeX,(y+1)*BlockSizeY-1,
                         c);
                drawLine(frameBuffer,
                         (x+0)*BlockSizeX,(y+1)*BlockSizeY-1,
                         (x+0)*BlockSizeX,(y+0)*BlockSizeY,
                         c);
            }
        }

}
void scan()
{
    static DIR *dirPtr;
    dirPtr=opendir(DIRECTORY);
    if(!dirPtr)
    {
        perror("Can't open Directory");
        exit(-1);
    }
          
    struct dirent *dp;
    while ((dp = readdir(dirPtr)) != NULL)
        {
            int pid;
            if(sscanf(dp->d_name,"%d",&pid))
            {
                char filename[1024];
                sprintf(filename,"%s/%s",DIRECTORY,dp->d_name);
	    	draw(pid,filename);
                if(pid>100)
			{
			int ret=kill(pid, 0);
			if(ret==-1 && errno==ESRCH)
			    unlink(filename);
			}
            }
        }
    closedir(dirPtr);
}

int main(int argc, char*argv[])
{
	Window imgWindow;
	int UserQuit=0;

	imgWindow=windowInit(BlockSizeX*BlocksX, BlockSizeY*BlocksY,
                         "TalkTo TestMode",NCCA_RESIZE);
#ifndef WINDOWING
	//We _REQUESTED_ a screen size, but we probably don't get it...
	//So check actuall screen size
	int w=windowWidth(imgWindow);
	int h=windowHeight(imgWindow);
	//printf("%d:%d\n",w,h);
	BlocksX=w/BlockSizeX;
	BlocksY=h/BlockSizeY;
	if(BlocksX>MAXBLOCKS)
		BlocksX=MAXBLOCKS;
	if(BlocksY>MAXBLOCKS)
		BlocksY=MAXBLOCKS;
	//printf("%d:%d\n",BlocksX,BlocksY);
#endif
	frameBuffer=newPixmap(BlockSizeX*BlocksX, BlockSizeY*BlocksY,3,8);
    
	watchInit();
    
	while(!UserQuit)
		{
#ifdef WINDOWING
		struct NCCAEvent_t event;
		event=catchEvents(0);

		if(event.type==EVENTTYPE_KEYBOARD && event.val=='q')
			UserQuit=1;

		if(event.type==EVENTTYPE_QUIT)
			UserQuit=1;
#endif

        drawGrid();
        garbageCollect();
        scan();
        screenDraw(imgWindow,frameBuffer);

	watchWait();
	}
	return 0;
}
