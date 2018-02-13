/*
 * $Log: NCCAPixmap.c,v $
 * Revision 1.32  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
 * Revision 1.31  2007/06/28 14:11:00  ian
 * Make funciton names more consistant
 *
 * Revision 1.30  2002/07/30 09:01:27  ian
 * add extra support in setPixelGrey
 *
 * Revision 1.29  2001/09/10 17:15:49  ian
 * add support for 1 and 2 channel images in getPixel
 *
 * Revision 1.28  2001/06/29 16:26:03  ian
 * add clamping to setPixelGrey
 *
 * Revision 1.27  2001/01/08 20:15:13  ian
 * remove old assertion from newPixmapLike
 *
 * Revision 1.26  2000/09/10 16:11:42  ian
 * move the file handling to PixFileIO
 *
 * Revision 1.25  2000/09/05 19:28:19  ian
 * add JPEG support (at last!)
 *
 * Revision 1.24  2000/03/25 21:07:20  ian
 * Split out the area functions into PixBlit
 * Optimize some of the functions a little
 *
 * Revision 1.23  2000/03/02 20:46:15  ian
 * add convertPixmap to do gamma correction and dithering
 * add support for 16bps
 *
 * Revision 1.22  2000/03/01 18:21:38  ian
 * get/set pixel functions now take a value between 0 and 1
 * Allows depth to be changed with less impact
 *
 * Revision 1.21  10/.0/.0  .1:.0:.1  ian
 * add reducePixmap function
 * 
 * Revision 1.20  99/12/08  18:12:25  ian
 * Add limited support for fp image depths
 * (For Angel)
 * 
 * Revision 1.19  99/08/17  21:37:54  ian
 * added pixmapInsert (blits src into dest at given postion)
 * needed for SLander bucketing
 * 
 * Revision 1.18  99/08/16  19:56:23  ian
 * avoid crashes when saving to a filename without an extension
 * 
 * Revision 1.17  1999/08/12 20:10:23  ian
 * Open files in binary mode under WIN32
 * 
 * Revision 1.16  99/08/08  15:44:28  ian
 * added bounds check to getPixel
 * 
 * Revision 1.15  99/07/04  17:09:55  ian
 * added ifdef TIFFSUPPORT to allow TIFFS to be turned off when the
 * library is not available (ie NT)
 * 
 * Revision 1.14  99/05/28  16:51:50  ian
 * added basic getAAPixel
 * (basically smoothes over four points)
 * 
 * Revision 1.13  1999/05/21 18:15:26  ian
 * add tiff support
 * 
 * Revision 1.12  99/05/09  15:26:39  ian
 * remove a couple of redundant includes - fixes WIN32
 * 
 * Revision 1.11  99/05/01  20:05:24  ian
 * add support for pic format
 * 
 * Revision 1.10  99/05/01  19:37:48  ian
 * fix savePPM
 * convert to use stdio
 * remove unused vars
 * (for WIN32 version)
 * 
 * Revision 1.9  99/04/17  23:03:27  ian
 * only free pixmap if its currently allocated
 * 
Revision 1.8  1999/04/07 10:15:29  istephen
removed some diagnostic messages

Revision 1.7  1998/03/27 19:30:14  amarsh
added isSeq sequence test

Revision 1.6  1998/03/26 23:53:19  selliott
added getGlobalPixel and newPixmaptohold untested

Revision 1.5  1998/03/26 21:52:06  lcaldeco
unfinished .pgm save

Revision 1.4  1998/03/10 16:19:48  selliott
added maintain displacement field integrity

Revision 1.3  1998/02/24 16:16:58  amarsh
add alpha support

Revision 1.2  1998/02/24 16:01:10  amarsh
tewst

Revision 1.1  1998/02/23 14:50:00  istephen
Initial revision

*/

/* test */
#include "NCCAPixmap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef NeXT
#include <libc.h>
#endif

#include <assert.h>



NCCAPixmap newPixmap(int w, int h, int spp, int bps)
	{
	NCCAPixmap newPix;
	
	newPix.height = h;
	newPix.width = w;
	newPix.bps = bps;
	newPix.spp = spp;

	newPix.data = (unsigned char *)malloc((newPix.height
						* newPix.width
						* newPix.bps
						* newPix.spp)/8);
	
	assert(newPix.data);
	return newPix;
	}

NCCAPixmap newPixmapLike(NCCAPixmap orig)
{
	NCCAPixmap newPix;	

	newPix=newPixmap(orig.width, orig.height, orig.spp,orig.bps);

	return newPix;
}

void destroyPixmap( NCCAPixmap p)
{
	if(p.data)
		free(p.data);
}

void setPixelGrey( NCCAPixmap p, int x, int y, float color)
{
	switch(p.bps)
	{
	case 8:
		{
		int icol=(color*255)+0.5;
		unsigned char *ptr=p.data+(y*p.width + x) *p.spp;
		if(icol>255) icol=255;
		if(icol<0) icol=0;
		switch(p.spp)
			{
			case 1: 
				*ptr=icol;
				break;
			case 2:
				*ptr=icol;
				*(ptr+1)=255;
				break;
			case 3:
				{
				*ptr=icol;*(ptr+1)=icol;
				*(ptr+2)=icol;
				}
				break;
			case 4:
				{
				*ptr=icol;*(ptr+1)=icol;*(ptr+2)=icol;
				*(ptr+3)=255;
				}
				break;
			default:
				assert(p.spp>0 && p.spp<5);
			}
		}
		break;
	case 16:
		{
		unsigned short *sdata=((unsigned short *)p.data);
		int icol=color*65535+0.5;
		if(icol>65535) icol=65535;
		if(icol<0) icol=0;
		switch(p.spp)
			{
			case 1: 
				sdata[(y*p.width + x) *p.spp]=icol;
				break;
			case 2:
				sdata[(y*p.width + x) *p.spp]=icol;
				sdata[(y*p.width + x) *p.spp+1]=65535;
				break;
			case 3:
				{
				sdata[(y*p.width + x) *p.spp]=icol;
				sdata[(y*p.width + x) *p.spp+1]=icol;
				sdata[(y*p.width + x) *p.spp+2]=icol;
				}
				break;
			case 4:
				{
				sdata[(y*p.width + x) *p.spp]=icol;
				sdata[(y*p.width + x) *p.spp+1]=icol;
				sdata[(y*p.width + x) *p.spp+2]=icol;
				sdata[(y*p.width + x) *p.spp+3]=65535;
				}
				break;
			default:
				assert(p.spp>0 && p.spp<5);
			}
		}
		break;
	case sizeof(float)*8:
		{
		switch(p.spp)
			{
			case 1: 
				((float*)p.data)[(y*p.width + x)]=color;
				break;
			case 2: 
				((float*)p.data)[(y*p.width + x)*2]=color;
				((float*)p.data)[(y*p.width + x)*2+1]=1;
				break;
			case 3: 
				((float*)p.data)[(y*p.width + x)*3]=color;
				((float*)p.data)[(y*p.width + x)*3+1]=color;
				((float*)p.data)[(y*p.width + x)*3+2]=color;
				break;
			case 4: 
				((float*)p.data)[(y*p.width + x)*4]=color;
				((float*)p.data)[(y*p.width + x)*4+1]=color;
				((float*)p.data)[(y*p.width + x)*4+2]=color;
				((float*)p.data)[(y*p.width + x)*4+3]=1;
				break;
			default:
				assert(p.spp==1);
			}
		}
	default:
		assert(p.bps==8 || p.bps==16 || p.bps == sizeof(float)*8);
	}
}


void setPixelColor( NCCAPixmap p, int x, int y, NCCAPixel colour)
{
	assert(x>=0 && x <p.width);
	assert(y>=0 && y <p.height);

	switch(p.bps)
		{
		case 8:
			{
			unsigned char *ptr=p.data+(y*p.width + x) *p.spp;
			if(colour.r > 1) colour.r=1;
			if(colour.g > 1) colour.g=1;
			if(colour.b > 1) colour.b=1;
			if(colour.a > 1) colour.a=1;
			if(colour.r < 0) colour.r=0;
			if(colour.g < 0) colour.g=0;
			if(colour.b < 0) colour.b=0;
			if(colour.a < 0) colour.a=0;
		
			switch(p.spp)
				{
				case 1:
					*ptr= (0.4*colour.r+ 0.35*
						colour.g+ 0.25*colour.b)*255+0.5;
					break;
				case 2: 
					*ptr= (0.4*colour.r+ 0.35*
						colour.g+ 0.25*colour.b)*255+0.5;
					*(ptr+1)=colour.a*255+0.5;
					break;
				case 3:
					{
					*ptr=colour.r*255+0.5;
					*(ptr+1)=colour.g*255+0.5;
					*(ptr+2)=colour.b*255+0.5;
					}
					break;
				case 4:
					{
					*ptr=colour.r*255+0.5;
					*(ptr+1)=colour.g*255+0.5;
					*(ptr+2)=colour.b*255+0.5;
					*(ptr+3)=colour.a*255+0.5;
					}
					break;
				default:
					assert(p.spp>0 && p.spp<5);
				}
			}
			break;
		case 16:
			{
			unsigned short *sdata=((unsigned short *)p.data);
			if(colour.r > 1) colour.r=1;
			if(colour.g > 1) colour.g=1;
			if(colour.b > 1) colour.b=1;
			if(colour.a > 1) colour.a=1;
			if(colour.r < 0) colour.r=0;
			if(colour.g < 0) colour.g=0;
			if(colour.b < 0) colour.b=0;
			if(colour.a < 0) colour.a=0;
		
			switch(p.spp)
				{
				case 1:
					sdata[(y*p.width + x) *p.spp]= (0.4*colour.r+ 0.35*
						colour.g+ 0.25*colour.b)*65535+0.5;
					break;
				case 2: 
					sdata[(y*p.width + x) *p.spp]= (0.4*colour.r+ 0.35*
						colour.g+ 0.25*colour.b)*65535+0.5;
					sdata[(y*p.width + x) *p.spp+1]=colour.a*65536+0.5;
					break;
				case 3:
					{
					sdata[(y*p.width + x) *p.spp]=colour.r*65535+0.5;
					sdata[(y*p.width + x) *p.spp+1]=colour.g*65535+0.5;
					sdata[(y*p.width + x) *p.spp+2]=colour.b*65535+0.5;
					}
					break;
				case 4:
					{
					sdata[(y*p.width + x) *p.spp]=colour.r*65535+0.5;
					sdata[(y*p.width + x) *p.spp+1]=colour.g*65535+0.5;
					sdata[(y*p.width + x) *p.spp+2]=colour.b*65535+0.5;
					sdata[(y*p.width + x) *p.spp+3]=colour.a*65535+0.5;
					}
					break;
				default:
					assert(p.spp>0 && p.spp<5);
				}
			}
			break;
		case 8*sizeof(float):
			{
			float *fdata=((float *)p.data);
			fdata+=(y*p.width + x) *p.spp;
			switch(p.spp)
			{
				case 1:
					*fdata= 0.4*colour.r+ 0.35*
						colour.g+ 0.25*colour.b;
					break;
				case 2: 
					*fdata= 0.4*colour.r+ 0.35*
						colour.g+ 0.25*colour.b;
					*(fdata+1)=colour.a;
					break;
				case 3:
					{
					*fdata=colour.r;
					*(fdata+1)=colour.g;
					*(fdata+2)=colour.b;
					}
					break;
				case 4:
					{
					*fdata=colour.r;
					*(fdata+1)=colour.g;
					*(fdata+2)=colour.b;
					*(fdata+3)=colour.a;
					}
					break;
				default:
					assert(p.spp>0 && p.spp<5);
				}
			}
			break;
		default:
			{
			assert(p.bps ==8 || p.bps ==8*sizeof(float));
			}
		}
}


NCCAPixel getAAPixel(NCCAPixmap pix, float xOffset, float yOffset)
{
	NCCAPixel pixel;
	int x,y;
	float xFrac;
	float yFrac;
	NCCAPixel left;
	NCCAPixel right;
	NCCAPixel top;
	NCCAPixel bottom;
	
	x=xOffset;
	y=yOffset;
	xFrac=xOffset-x;
	yFrac=yOffset-y;
	
	if(xFrac == 0)
		{
		bottom=getPixelColor(pix,x,y);
		}
	else
		{
		left=getPixelColor(pix,x,y);
		right=getPixelColor(pix,x+1,y);
		bottom.r=left.r*(1-xFrac)+right.r*xFrac;
		bottom.g=left.g*(1-xFrac)+right.g*xFrac;
		bottom.b=left.b*(1-xFrac)+right.b*xFrac;
		bottom.a=left.a*(1-xFrac)+right.a*xFrac;
		}
	
	if(yOffset==0)
		{
		pixel=bottom;
		}
	else
		{
		if(xFrac == 0)
			{
			top=getPixelColor(pix,x,y+1);
			}
		else
			{
			left=getPixelColor(pix,x,y+1);
			right=getPixelColor(pix,x+1,y+1);
			top.r=left.r*(1-xFrac)+right.r*xFrac;
			top.g=left.g*(1-xFrac)+right.g*xFrac;
			top.b=left.b*(1-xFrac)+right.b*xFrac;
			top.a=left.a*(1-xFrac)+right.a*xFrac;
			}
		pixel.r=bottom.r*(1-yFrac)+top.r*yFrac;
		pixel.g=bottom.g*(1-yFrac)+top.g*yFrac;
		pixel.b=bottom.b*(1-yFrac)+top.b*yFrac;
		pixel.a=bottom.a*(1-yFrac)+top.a*yFrac;
		}
	
	return pixel;
}


float getAAPixelGrey(NCCAPixmap pix, float xOffset, float yOffset)
	{
	float pixel;
	int x,y;
	float xFrac;
	float yFrac;
	float left;
	float right;
	float top;
	float bottom;
	
	x=xOffset;
	y=yOffset;
	xFrac=xOffset-x;
	yFrac=yOffset-y;
	
	if(xFrac == 0)
		{
		bottom=getPixelGrey(pix,x,y);
		}
	else
		{
		left=getPixelGrey(pix,x,y);
		right=getPixelGrey(pix,x+1,y);
		bottom=left*(1-xFrac)+right*xFrac;
		}
	
	if(yOffset==0)
		{
		pixel=bottom;
		}
	else
		{
		if(xFrac == 0)
			{
			top=getPixelGrey(pix,x,y+1);
			}
		else
			{
			left=getPixelGrey(pix,x,y+1);
			right=getPixelGrey(pix,x+1,y+1);
			top=left*(1-xFrac)+right*xFrac;
			}
		pixel=bottom*(1-yFrac)+top*yFrac;
		}
	
	return pixel;
}

float getPixelGrey(NCCAPixmap pixmap, int xOffset, int yOffset)
	{
	assert(pixmap.spp==1 || pixmap.spp==2);

	if(xOffset<0) xOffset=0;
	if(yOffset<0) yOffset=0;
	if(xOffset>pixmap.width-1) xOffset=pixmap.width-1;
	if(yOffset>pixmap.height-1) yOffset=pixmap.height-1;

	switch(pixmap.bps)
	{
	case 8:
		return
			((float)pixmap.data[(yOffset*pixmap.width + xOffset)*pixmap.spp])
															/255;
	case 16:
		return ((float)((unsigned short*)pixmap.data)
						[(yOffset*pixmap.width + xOffset)*pixmap.spp])
															/65535;
	case sizeof(float)*8:
		return ((float*)pixmap.data)
							[(yOffset*pixmap.width + xOffset) *pixmap.spp];
	default:
		assert(pixmap.bps==8 || pixmap.bps==16 || pixmap.bps==sizeof(float)*8);
	}
	return 0;
	}

NCCAPixel getPixelColor( NCCAPixmap pixmap, int xOffset, int yOffset)
{
	NCCAPixel pixel;
	assert(pixmap.bps==8 || pixmap.bps==16 || pixmap.bps==8*sizeof(float));
	assert(pixmap.spp>0 && pixmap.spp<=4);

	if(xOffset<0) xOffset=0;
	if(yOffset<0) yOffset=0;
	if(xOffset>pixmap.width-1) xOffset=pixmap.width-1;
	if(yOffset>pixmap.height-1) yOffset=pixmap.height-1;

	if(pixmap.spp==1 || pixmap.spp==2)
		{
		int element=(yOffset*pixmap.width + xOffset)*pixmap.spp;
		switch(pixmap.bps)
			{
			case 8:
				pixel.r=pixel.g=pixel.b=
						((float)pixmap.data[element])/255;
				if(pixmap.spp==2)
					pixel.a= ((float)pixmap.data[element+1])/255;
				else
					pixel.a= 1;
				return pixel;
			case 16:
				pixel.r=pixel.g=pixel.b=
						 ((float)((unsigned short*)pixmap.data)[element])/65535;
				if(pixmap.spp==2)
					pixel.a= ((float)((unsigned short*)pixmap.data)[element+1])/65535;
				else
					pixel.a= 1;
				return pixel;
			case sizeof(float)*8:
				pixel.r=pixel.g=pixel.b=((float*)pixmap.data)[element];
				if(pixmap.spp==2)
					pixel.a=((float*)pixmap.data)[element+1];
				else
					pixel.a=1;
				return pixel;
			}
		}
	else
		{
		switch(pixmap.bps)
		{
		case 8:
			{
			pixel.r=pixmap.data[(yOffset*pixmap.width + xOffset) *pixmap.spp];
			pixel.g=pixmap.data[(yOffset*pixmap.width + xOffset) *pixmap.spp+1];
			pixel.b=pixmap.data[(yOffset*pixmap.width + xOffset) *pixmap.spp+2];
			
			pixel.r /=255;
			pixel.g /=255;
			pixel.b /=255;
		
			if(pixmap.spp==3)
			{
			pixel.a=1;
			}
			else
				{
				pixel.a=pixmap.data[(yOffset*pixmap.width + xOffset) *pixmap.spp+3];
				pixel.a /=255;
				}
			return pixel;
			}
		case 16:
			{
			unsigned short *sdata=(unsigned short *)pixmap.data;
			pixel.r=sdata[(yOffset*pixmap.width + xOffset) *pixmap.spp];
			pixel.g=sdata[(yOffset*pixmap.width + xOffset) *pixmap.spp+1];
			pixel.b=sdata[(yOffset*pixmap.width + xOffset) *pixmap.spp+2];
			
			pixel.r /=65535;
			pixel.g /=65535;
			pixel.b /=65535;
		
			if(pixmap.spp==3)
			{
			pixel.a=1;
			}
			else
				{
				pixel.a=sdata[(yOffset*pixmap.width + xOffset) *pixmap.spp+3];
				pixel.a /=65535;
				}
			return pixel;
			}
		case 8*sizeof(float):
			{
			float *fdata=(float *)pixmap.data;
			pixel.r=fdata[(yOffset*pixmap.width + xOffset) *pixmap.spp];
			pixel.g=fdata[(yOffset*pixmap.width + xOffset) *pixmap.spp+1];
			pixel.b=fdata[(yOffset*pixmap.width + xOffset) *pixmap.spp+2];
			
			if(pixmap.spp==3)
			{
			pixel.a=1;
			}
			else
				{
				pixel.a=fdata[(yOffset*pixmap.width + xOffset) *pixmap.spp+3];
				}
			return pixel;
			}
		}
		}
	assert(0);
	exit(-1);
}



