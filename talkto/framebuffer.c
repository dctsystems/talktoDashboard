#ifndef WINDOWING
//This is only for non-windowed versions on Linux
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <NCCAPixmap.h>
#include "framebuffer.h"

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <signal.h>

static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static uint8_t *screenbuffer;
static long int screensize = 0;

void cleanup(void) {
     int fbfd = 0;
    fbfd = open("/dev/tty0", O_RDWR);
    if(fbfd)
	{
	ioctl(fbfd, KDSETMODE, KD_TEXT);
	close(fbfd);
	}
}

void signalCatcher()
{
	exit(0);
}

Window FBwindowInit(int w, int h, char *title, int flags)
{
     int fbfd = 0;
    signal(SIGINT,(__sighandler_t)signalCatcher);
    atexit(cleanup);
    fbfd = open("/dev/tty0", O_RDWR);
    if(fbfd)
	{
	ioctl(fbfd, KDSETMODE, KD_GRAPHICS);
	close(fbfd);
	fbfd=0;
	}


    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (!fbfd) {
      fprintf(stderr,"Error: cannot open framebuffer device.\n");
      exit(-1);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
      printf("Error reading variable information.\n");
    }
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
      printf("Error reading fixed information.\n");
    }

    //printf("Original %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );
    if(w>vinfo.xres || h>vinfo.yres)
	{
	fprintf(stderr,"%dx%d framebuffer required\n",w,h);
	exit(0);
	}
    if(vinfo.bits_per_pixel != 16 && vinfo.bits_per_pixel != 32)
	{
	fprintf(stderr,"%d bit mode framebuffer not supported\n",vinfo.bits_per_pixel);
	exit(0);
	}

    // map fb to user mem 
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    screenbuffer = (uint8_t *)mmap(0, 
              screensize, 
              PROT_READ | PROT_WRITE, 
              MAP_SHARED, 
              fbfd, 
              0);
  
  //SN_displayX=vinfo.xres;
  //SN_displayY=vinfo.yres;
  return NULL;
}

void Xframebuffer_setPixel()
{
}


void FBcatchEvents(int wait)
{
}

void FBscreenDraw(Window window, NCCAPixmap img)
{
assert(img.spp==3);
assert(img.bps==8);

int h=img.height;
int w=img.width;
if(w>vinfo.xres)
	w=vinfo.xres;
if(h>vinfo.yres)
	w=vinfo.yres;

int x,y;

if(vinfo.bits_per_pixel != 16 && vinfo.bits_per_pixel != 32)
	{
	for(y=0;y<h;y++)
		{
		uint8_t *srcptr;
		uint16_t *destptr;
		srcptr=img.data+y*3*img.width;
		destptr=(uint16_t *) (screenbuffer+y*finfo.line_length);
		for(x=0;x<w;x++)
			{
			uint8_t r=(*(srcptr++))>>3;
			uint8_t g=(*(srcptr++))>>2;
			uint8_t b=(*(srcptr++))>>3;
			uint16_t c=r;
			c<<=6;
			c|=g;
			c<<=5;
			c|=b;
			*(destptr++)=c;
			}
		}
	}
else
        {
        for(y=0;y<h;y++)
                {
                uint8_t *srcptr;
                uint32_t *destptr;
                srcptr=img.data+y*3*img.width;
                destptr=(uint32_t *) (screenbuffer+y*finfo.line_length);
                for(x=0;x<w;x++)
                        {
                        uint8_t r=(*(srcptr++));
                        uint8_t g=(*(srcptr++));
                        uint8_t b=(*(srcptr++));
                        uint32_t c=r;
                        c<<=8;
                        c|=g;
                        c<<=8;
                        c|=b;
                        *(destptr++)=c;
                        }
                }
        }

}

#endif
