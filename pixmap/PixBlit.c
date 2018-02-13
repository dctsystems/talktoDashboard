 /**********************************************************************
 *
 * (c) Ian Stephenson, ian@dctsystems.freeserve.co.uk
 * $Log: PixBlit.c,v $
 * Revision 1.8  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
 * Revision 1.7  2007/06/28 14:12:00  ian
 * Use improved function names
 *
 * Revision 1.6  2002/07/30 09:04:31  ian
 * add a cast to shut SGI compiler up
 *
 * Revision 1.5  2001/12/29 16:13:46  ian
 * add more formats to reducePixmap
 *
 * Revision 1.4  2001/06/29 16:24:19  ian
 * add 16 bit support to clearPixmap
 * add greyscal support to reducePixmap
 *
 * Revision 1.3  2000/12/28 19:18:19  ian
 * add better depth support to insertPixmap and convertPixmap
 *
 * Revision 1.2  2000/09/17 14:01:54  ian
 * add CopyPixmap
 *
 * Revision 1.1  2000/03/25 21:06:18  ian
 * Initial revision
 *
 *
 **********************************************************************/

#include "NCCAPixmap.h"
#include "PixBlit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#ifdef NeXT
#include <libc.h>
#endif


void pixmapClearGrey(NCCAPixmap p, float val)
{
	assert(p.bps==8 ||p.bps==16 ||  p.bps==8*sizeof(float));
	
	if(p.bps==8)
		{
		int pval=val*255;
		unsigned char bval;
		int pcount=p.height*p.width*p.spp;
		int i;

		if(pval<0) pval=0;
		if(pval>255) pval=255;
		bval=pval;
		
		for(i=0;i<pcount;i++)
			{
			p.data[i]=bval;
			}
		}
	else if(p.bps==16)
		{
		int pval=val*65536;
		unsigned short *ptr=(unsigned short*)p.data;
		unsigned short sval;
		int pcount=p.height*p.width*p.spp;
		int i;

		if(pval<0) pval=0;
		if(pval>65535) pval=65535;
		sval=pval;
		
		for(i=0;i<pcount;i++)
			{
			ptr[i]=sval;
			}
		}
	else if(p.bps==8*sizeof(float))
		{
		int pcount=p.height*p.width*p.spp;
		float *ptr=(float*)p.data;
		int i;

		for(i=0;i<pcount;i++)
			{
			ptr[i]=val;
			}
		}
	else
		{
		assert(p.bps==8 || p.bps==16 || p.bps==8*sizeof(float));
		}
}


void pixmapInsert(NCCAPixmap src,int xPos, int yPos, NCCAPixmap dest)
{
	int i,j;
	int height,width;
	int srcXpos, srcYpos;
	
	
	srcXpos=0;
	srcYpos=0;

	width=src.width;
	if(xPos+width >dest.width)
		width=dest.width-xPos;
	height=src.height;
	if(yPos+height >dest.height)
		height=dest.height-yPos;

	if(xPos<0)
		{
		width +=xPos;
		srcXpos = -xPos;
		xPos=0;
		}
	if(yPos<0)
		{
		height +=yPos;
		srcYpos = -yPos;
		yPos=0;
		}

	if((dest.bps&0x7)==0 && dest.bps==src.bps && dest.spp==src.spp)
		{
		//The image is a multiple of 8 BPS
		//and the images have the same channels,
		//so use the fast version..
		unsigned int bytesPP;
		unsigned char *srcLineStart;
		unsigned char *destLineStart;
		unsigned char *srcData;
		unsigned char *destData;
		bytesPP=src.spp*src.bps/8;
		srcLineStart = src.data+srcYpos* src.width*bytesPP+
							bytesPP*srcXpos;
		destLineStart=dest.data+yPos *dest.width*bytesPP
							+bytesPP*xPos;
		
		width *=bytesPP;
		for(j=0;j<height;j++)
			{
			srcData=srcLineStart;
			destData=destLineStart;
			for(i=0;i<width;i++)
				{
				*destData++=*srcData++;
				}
			destLineStart+=dest.width*bytesPP;
			srcLineStart+=src.width*bytesPP;
			}
		}
	else
		{
		//Fall back to the generic/slow version
		for(j=0;j<height;j++)
			for(i=0;i<width;i++)
				{
				NCCAPixel P=getPixelColor(src,srcXpos+i,srcYpos+j);
				setPixelColor(dest,xPos+i,yPos+j,P);
				}
		}
}
void pixmapInsertAdd(NCCAPixmap src,int xPos, int yPos, NCCAPixmap dest)
{
	int i,j;
	int height,width;
	int srcXpos, srcYpos;
	
	
	srcXpos=0;
	srcYpos=0;

	width=src.width;
	if(xPos+width >dest.width)
		width=dest.width-xPos;
	height=src.height;
	if(yPos+height >dest.height)
		height=dest.height-yPos;

	if(xPos<0)
		{
		width +=xPos;
		srcXpos = -xPos;
		xPos=0;
		}
	if(yPos<0)
		{
		height +=yPos;
		srcYpos = -yPos;
		yPos=0;
		}

	//The generic/slow version
	for(j=0;j<height;j++)
		for(i=0;i<width;i++)
			{
			NCCAPixel P=getPixelColor(src,srcXpos+i,srcYpos+j);
			NCCAPixel Q=getPixelColor(dest,xPos+i,yPos+j);
			P.r+=Q.r;
			P.g+=Q.g;
			P.b+=Q.b;
			P.a+=Q.a;
			setPixelColor(dest,xPos+i,yPos+j,P);
			}
}
NCCAPixmap pixmapExtract(NCCAPixmap src,int xPos, int yPos, int width, int height)
	{
	NCCAPixmap r=newPixmap(width, height, src.spp, src.bps);
	int j;
        for(j=0;j<height;j++)
		{
		int i;
                for(i=0;i<width;i++)
                        {
                        NCCAPixel P=getPixelColor(src,xPos+i,yPos+j);
                        setPixelColor(r,i,j,P);
                        }
		}
	return r;
	}

NCCAPixmap convertPixmapToGreyscale(NCCAPixmap src)
        {
        NCCAPixmap r=newPixmap(src.width, src.height,1, src.bps);
        int j;
        for(j=0;j<src.height;j++)
                {
                int i;
                for(i=0;i<src.width;i++)
                        {
                        NCCAPixel P=getPixelColor(src,i,j);
                        setPixelColor(r,i,j,P);
                        }
                }
        return r;
        }



NCCAPixmap reducePixmap(NCCAPixmap src,int destBPS,
						int xScale, int yScale, int filter)
{
	NCCAPixmap newPix;
	int x,y,i,j;
	
	assert(src.bps==8 || src.bps==16 || src.bps==8*sizeof(float));
	assert(src.width % xScale==0);
	assert(src.height % yScale==0);
	
	newPix=newPixmap(src.width/xScale, src.height/yScale, src.spp, destBPS);

	switch(filter)
	{
	case NCCA_BOXFILTER:
		{
		float totWeight=xScale*yScale;
		if(src.bps==8)
			totWeight*=255;
		if(src.bps==16)
			totWeight*=65535;

		for(y=0;y<newPix.height;y++)
			for(x=0;x<newPix.width;x++)
				{
				NCCAPixel p;
				p.r=p.g=p.b=p.a=0;
				for(j=0;j<yScale;j++)
					{
					int yOffset=(y*yScale+j)*src.width;
					if(yOffset<0) yOffset=0;
					for(i=0;i<xScale;i++)
						{
						int offset;
						offset=(yOffset + x*xScale+i) *src.spp;

						if(src.bps==8)
							{
							switch(src.spp)
								{
								case 4:
									p.a+=src.data[offset+3];
								case 3:
									p.b+=src.data[offset+2];
								case 2:
									p.g+=src.data[offset+1];
								case 1:
									p.r+=src.data[offset];
								}
							}
						else if(src.bps==16)
							{
							unsigned short *sdata=(unsigned short*)src.data;
							switch(src.spp)
								{
								case 4:
									p.a+=sdata[offset+3];
								case 3:
									p.b+=sdata[offset+2];
								case 2:
									p.g+=sdata[offset+1];
								case 1:
									p.r+=sdata[offset];
								}
							}
						else
							{
							float *fdata=(float*)src.data;
							switch(src.spp)
								{
								case 4:
									p.a+=fdata[offset+3];
								case 3:
									p.b+=fdata[offset+2];
								case 2:
									p.g+=fdata[offset+1];
								case 1:
									p.r+=fdata[offset];
								}
							}
						}
					}
				switch(src.spp)
					{
					case 1:
						p.r/=totWeight;
						setPixelGrey(newPix,x,y,p.r);
						break;
					case 2:
						p.r/=totWeight;
						setPixelGrey(newPix,x,y,p.r);
						break;
					case 3:
						p.r/=totWeight;
						p.g/=totWeight;
						p.b/=totWeight;
						p.a=1;
						setPixelColor(newPix,x,y,p);
						break;
					case 4:
						p.r/=totWeight;
						p.g/=totWeight;
						p.b/=totWeight;
						p.a/=totWeight;
						setPixelColor(newPix,x,y,p);
						break;
					}
				}
		break;
		}
	case NCCA_TRIANGLEFILTER:
		{
		float totWeight=0;
		for(j=-yScale+1;j<yScale;j++)
			{
			float yweight=(((float)(yScale-abs(j)))/yScale);
			for(i=-xScale+1;i<xScale;i++)
				{
				float xweight=(((float)(xScale-abs(i)))/xScale);
				float weight=xweight<yweight?xweight:yweight;
				totWeight+=weight;
				}
			}

		if(src.bps==8)
			totWeight*=255;
		if(src.bps==16)
			totWeight*=65535;
		
		for(y=0;y<newPix.height;y++)
			for(x=0;x<newPix.width;x++)
				{
				NCCAPixel p;
				p.r=p.g=p.b=p.a=0;
				for(j=-yScale+1;j<yScale;j++)
					{
					int yOffset=(y*yScale+j)*src.width;
					float yweight=(((float)(yScale-abs(j)))/yScale);
					if(yOffset<0) yOffset=0;
					for(i=-xScale+1;i<xScale;i++)
						{
						int offset;
						float xweight=(((float)(xScale-abs(i)))/xScale);
						float weight=xweight<yweight?xweight:yweight;
						int xoffset=x*xScale+i;
						if(xoffset<0)
							xoffset=0;
						offset=(yOffset + xoffset) *src.spp;

						if(src.bps==8)
							{
							switch(src.spp)
								{
								case 4:
									p.a+=src.data[offset+3]*weight;
								case 3:
									p.b+=src.data[offset+2]*weight;
								case 2:
									p.g+=src.data[offset+1]*weight;
								case 1:
									p.r+=src.data[offset]*weight;
								}
							}
						else if(src.bps==16)
							{
							unsigned short *sdata=(unsigned short*)src.data;
							switch(src.spp)
								{
								case 4:
									p.a+=sdata[offset+3]*weight;
								case 3:
									p.b+=sdata[offset+2]*weight;
								case 2:
									p.g+=sdata[offset+1]*weight;
								case 1:
									p.r+=sdata[offset]*weight;
								}
							}
						else
							{
							float *fdata=(float*)src.data;
							switch(src.spp)
								{
								case 4:
									p.a+=fdata[offset+3]*weight;
								case 3:
									p.b+=fdata[offset+2]*weight;
								case 2:
									p.g+=fdata[offset+1]*weight;
								case 1:
									p.r+=fdata[offset]*weight;
								}
							}
						}
					}
				switch(src.spp)
					{
					case 1:
						p.r/=totWeight;
						setPixelGrey(newPix,x,y,p.r);
						break;
					case 2:
						p.r/=totWeight;
						setPixelGrey(newPix,x,y,p.r);
						break;
					case 3:
						p.r/=totWeight;
						p.g/=totWeight;
						p.b/=totWeight;
						p.a=1;
						setPixelColor(newPix,x,y,p);
						break;
					case 4:
						p.r/=totWeight;
						p.g/=totWeight;
						p.b/=totWeight;
						p.a/=totWeight;
						setPixelColor(newPix,x,y,p);
						break;
					}
				}
		break;
		}
	default:
		fprintf(stderr,"Unknown filter type %d\n",filter);
	}
	return newPix;
}

NCCAPixmap convertPixmap(NCCAPixmap src, int destBPS ,float gain, float gamma)
{
	NCCAPixmap newPix;
	int i,j;
	float oog=1/gamma;

	newPix=newPixmap(src.width, src.height, src.spp, destBPS);
	if(src.spp>=3)
		{
		if(destBPS>8)
			for(i=0;i<src.width;i++)
				for(j=0;j<src.height;j++)
					{
					NCCAPixel p;
					p=getPixelColor(src,i,j);
					p.r=pow(p.r*gain,oog);
					p.g=pow(p.g*gain,oog);
					p.b=pow(p.b*gain,oog);
					p.a=pow(p.a*gain,oog);
					setPixelColor(newPix,i,j,p);
					}
		else if(destBPS==8)
			for(i=0;i<src.width;i++)
				for(j=0;j<src.height;j++)
					{
#ifdef WIN32
#define random rand
#endif
#ifdef MAC
#define random rand
#endif
#define DITHER(X) (((((float)(random()&0xff))/255.0)-0.5)/X)
					NCCAPixel p;
					p=getPixelColor(src,i,j);

					p.r=pow(p.r*gain,oog)+DITHER(256);
					p.g=pow(p.g*gain,oog)+DITHER(256);
					p.b=pow(p.b*gain,oog)+DITHER(256);
					p.a=pow(p.a*gain,oog)+DITHER(256);

					setPixelColor(newPix,i,j,p);
					}
		else
			{
			assert(destBPS>=8);
			}
		}
	else
		{
		if(destBPS>8)
			for(i=0;i<src.width;i++)
				for(j=0;j<src.height;j++)
					{
					float p;
					p=getPixelGrey(src,i,j);
					p=pow(p*gain,oog);
					setPixelGrey(newPix,i,j,p);
					}
		else if(destBPS==8)
			for(i=0;i<src.width;i++)
				for(j=0;j<src.height;j++)
					{
					float p;
					p=getPixelGrey(src,i,j);
					p=pow(p*gain,oog)+DITHER(256);
					setPixelGrey(newPix,i,j,p);
					}
		else
			{
			assert(destBPS>=8);
			}
		}
	return newPix;
}

NCCAPixmap copyPixmap(NCCAPixmap src)
{
	NCCAPixmap newPix;
	int len=(src.height*src.width*src.spp*src.bps)/8;

	assert(src.bps%8==0);

	newPix=newPixmapLike(src);
	memcpy(newPix.data,src.data,len);

	return newPix;
}


