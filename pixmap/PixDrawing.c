 /**********************************************************************
 *
 * (c) Ian Stephenson, ian@dctsystems.freeserve.co.uk
 * $Log: PixDrawing.c,v $
 * Revision 1.18  2007/06/28 14:12:00  ian
 * Use improved function names
 *
 * Revision 1.17  2002/04/25 15:05:06  ian
 * add matte functions
 *
 * Revision 1.16  2001/07/04 11:49:31  ian
 * add setInterlace functions to reduce supoprt Motion blur in Angel
 *
 * Revision 1.15  2000/12/18 20:18:32  ian
 * Getting better... use fabs properly to handle left and right lines
 *
 * Revision 1.14  2000/12/18 19:15:31  ian
 * avoid division by zero
 *
 * Revision 1.13  2000/10/03 21:28:30  ian
 * Fix (and optimise) ZFillTriangle
 *
 * Revision 1.12  2000/10/02 18:50:39  ian
 * finally (?) fix edges in drawTriangle (2d version)
 *
 * Revision 1.11  2000/10/01 18:56:03  ian
 * remove lots of erroneous/duplicate spans
 *
 * Revision 1.10  2000/10/01 16:24:09  ian
 * Don't draw the right hand pixel of each span
 *
 * Revision 1.9  2000/08/25 18:27:47  ian
 * include math.h and use fabs to fix drawLine
 *
 * Revision 1.8  2000/04/16 16:08:36  ian
 * Try and improve performance in doZSpan
 * Fixed bug which was causing white spots when lerping alpha
 *
 * Revision 1.7  2000/03/21 20:41:35  ian
 * add check to doZSpan in case pixmap.data
 * This happens when we're rendering a shadow map, but
 * we still need to call doSpan as we need to write the ZBuffer
 *
 * Revision 1.6  2000/03/01 18:24:35  ian
 * NCCAPixmap functions now take a range between 0 and 1 so
 * we can get rid of lots of nasty 255's
 *
 * Revision 1.5  2000/02/22 21:49:03  ian
 * Use random() to try and support Alpha with Z Buffer
 *
 * Revision 1.4  2000/02/10 19:29:22  ian
 * Increase stability of code - now works on BSD
 * Avoids divide by zero's
 *
 * Revision 1.3  99/12/07  21:10:21  ian
 * add checks to bail out when a triangle is offscreen
 * 
 * Revision 1.2  99/12/02  12:17:48  ian
 * add smooth shading to zfill functions
 * 
 * Revision 1.1  99/12/02  11:13:58  ian
 * Initial revision
 * 
 *
 **********************************************************************/
#include "NCCAPixmap.h"
#include "PixDrawing.h"
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#ifdef NeXT
#include <libc.h>
#endif

#ifdef WIN32
#define random rand
#endif

#ifdef MAC
#define random rand
#endif

static int mbPasses=1;
static int mbPass=0;
void setInterlace(int which,int of)
	{
	mbPass=which;
	mbPasses=of;
	}

static void doSpan(NCCAPixmap pixmap,
								float l, float r,
								int y,
								NCCAPixel Cs)
	{
	int x,end;

	if(y<0 || y>=pixmap.height)
		return;

	if(y%mbPasses!=mbPass)
		return;

	if(l>r)
		{
		float t;
		t=l;
		l=r;
		r=t;
		}

	if(r>pixmap.width)
		end=pixmap.width;
	else
		end=ceil(r);

	x=ceil(l);
	if(x<0)
		x=0;
		
	for(;x<end;x++)
		setPixelColor(pixmap,x,y,Cs);
	}

static void doZSpan(NCCAPixmap pixmap,NCCAPixmap zbuffer,
								float l, float r,
								float lz, float rz,
								NCCAPixel lc,NCCAPixel rc,
								int y
								)
	{
	int ll,rr;
	if(y<0 || y>=pixmap.height)
		return;

	if(y%mbPasses!=mbPass)
		return;

	ll=ceil(l);
	rr=ceil(r);

	switch(rr-ll)
		{
		case 0:
			return;
		case 1:
			{
			if(ll>=0 && ll<pixmap.width)
				{
				float oldz;
				float v=(ll-l)/(r-l);
				float a=lc.a+v*(rc.a-lc.a);
				float z=lz+v*(rz-lz);
				oldz=getPixelGrey(zbuffer,ll,y);
				if(z<oldz && (random()%255)/255.0<a)
					{
					NCCAPixel Cs;
					Cs.r=(lc.r+v*(rc.r-lc.r))/a;
					Cs.g=(lc.g+v*(rc.g-lc.g))/a;
					Cs.b=(lc.b+v*(rc.b-lc.b))/a;
					Cs.a=1;
					
					setPixelGrey(zbuffer,ll,y, z);
					if(pixmap.data)
						setPixelColor(pixmap,ll,y, Cs);
					}
				}
			return;
			}
		case -1:
			{
			if(rr>=0 && rr<pixmap.width)
				{
				float oldz;
				float v=(rr-r)/(l-r);
				float a=rc.a+v*(lc.a-rc.a);
				float z=rz+v*(lz-rz);
				oldz=getPixelGrey(zbuffer,rr,y);
				if(z<oldz && (random()%255)/255.0<a)
					{
					NCCAPixel Cs;
					Cs.r=(rc.r+v*(lc.r-rc.r))/a;
					Cs.g=(rc.g+v*(lc.g-rc.g))/a;
					Cs.b=(rc.b+v*(lc.b-rc.b))/a;
					Cs.a=1;
					
					setPixelGrey(zbuffer,rr,y, z);
					if(pixmap.data)
						setPixelColor(pixmap,rr,y, Cs);
					}
				}
			return;
			}
		default:
			{
			int x,end;
			float a;
			float newz;
			NCCAPixel Cs;
			float ma;
			float mz;
			NCCAPixel mC;
			float startDelta;
			float oowidth;
			
			if(ll>rr)
				{
				float t;
				int tt;
				float tz;
				NCCAPixel tc;
				tt=ll;t=l;tz=lz;tc=lc;
				ll=rr;l=r;lz=rz;lc=rc;
				rr=tt;r=t;rz=tz;rc=tc;
				}
		
			if(rr<0 || ll>=pixmap.width)
				return;
		
			if(rr>pixmap.width)
				end=pixmap.width;
			else
				end=rr;

			x=ll;
			if(x<0)
				x=0;
			
			oowidth=1.0/(r-l);
			mz=(rz-lz)*oowidth;
			ma=(rc.a-lc.a)*oowidth;
			mC.r=(rc.r-lc.r)*oowidth;
			mC.g=(rc.g-lc.g)*oowidth;
			mC.b=(rc.b-lc.b)*oowidth;
			
			startDelta=x-l;
			newz=lz+startDelta*mz;
			a   =lc.a+startDelta*ma;
			Cs.r=lc.r+startDelta*mC.r;
			Cs.g=lc.g+startDelta*mC.g;
			Cs.b=lc.b+startDelta*mC.b;					
			Cs.a=1;

			for(;x<end;x++)
				{
				float oldz;

				oldz=getPixelGrey(zbuffer,x,y);
				if(newz<oldz && (random()%255)/255.0<a)
					{
					NCCAPixel C;
					C.r=Cs.r/a;
					C.g=Cs.g/a;
					C.b=Cs.b/a;
					C.a=1;
					setPixelGrey(zbuffer,x,y, newz);
					if(pixmap.data)
						setPixelColor(pixmap,x,y, C);
					}
				
				newz+=mz;
				a   +=ma;
				Cs.r+=mC.r;
				Cs.g+=mC.g;
				Cs.b+=mC.b;
				}
			}
		}
	}

static void matteZSpan(NCCAPixmap pixmap,NCCAPixmap zbuffer,
								float l, float r,
								float lz, float rz,
								float coverage,
								int y
								)
	{
	int ll,rr;
	if(y<0 || y>=pixmap.height)
		return;

	if(y%mbPasses!=mbPass)
		return;

	ll=ceil(l);
	rr=ceil(r);

	switch(rr-ll)
		{
		case 0:
			return;
		case 1:
			{
			if(ll>=0 && ll<pixmap.width)
				{
				float oldz;
				float v=(ll-l)/(r-l);
				float z=lz+v*(rz-lz);
				oldz=getPixelGrey(zbuffer,ll,y);
				if(z<oldz && (random()%255)/255.0<coverage)
					{
					NCCAPixel Cs={0,0,0,0};
					
					setPixelGrey(zbuffer,ll,y, z);
					if(pixmap.data)
						setPixelColor(pixmap,ll,y, Cs);
					}
				}
			return;
			}
		case -1:
			{
			if(rr>=0 && rr<pixmap.width)
				{
				float oldz;
				float v=(rr-r)/(l-r);
				float z=rz+v*(lz-rz);
				oldz=getPixelGrey(zbuffer,rr,y);
				if(z<oldz && (random()%255)/255.0<coverage)
					{
					NCCAPixel Cs={0,0,0,0};
					
					setPixelGrey(zbuffer,rr,y, z);
					if(pixmap.data)
						setPixelColor(pixmap,rr,y, Cs);
					}
				}
			return;
			}
		default:
			{
			int x,end;
			float newz;
			NCCAPixel Cs={0,0,0,0};
			float mz;
			float startDelta;
			float oowidth;
			
			if(ll>rr)
				{
				float t;
				int tt;
				float tz;
				tt=ll;t=l;tz=lz;
				ll=rr;l=r;lz=rz;
				rr=tt;r=t;rz=tz;
				}
		
			if(rr<0 || ll>=pixmap.width)
				return;
		
			if(rr>pixmap.width)
				end=pixmap.width;
			else
				end=rr;

			x=ll;
			if(x<0)
				x=0;
			
			oowidth=1.0/(r-l);
			mz=(rz-lz)*oowidth;
			
			startDelta=x-l;
			newz=lz+startDelta*mz;

			for(;x<end;x++)
				{
				float oldz;

				oldz=getPixelGrey(zbuffer,x,y);
				if(newz<oldz && (random()%255)/255.0<coverage)
					{
					setPixelGrey(zbuffer,x,y, newz);
					if(pixmap.data)
						setPixelColor(pixmap,x,y, Cs);
					}
				
				newz+=mz;
				}
			}
		}
	}




void fillTriangle(NCCAPixmap pixmap, float x0, float y0,
								float x1, float y1,
								float x2, float y2,
								NCCAPixel Cs)
	{
	int y;
	int yy0,yy1,yy2;
	float m1,m2;
	float l,r;
	if(y0>y1)
		{
		float tx,ty;
		tx=x0;ty=y0;
		x0=x1;y0=y1;
		x1=tx;y1=ty;
		}
	if(y0>y2)
		{
		float tx,ty;
		tx=x0;ty=y0;
		x0=x2;y0=y2;
		x2=tx;y2=ty;
		}
	if(y1>y2)
		{
		float tx,ty;
		tx=x1;ty=y1;
		x1=x2;y1=y2;
		x2=tx;y2=ty;
		}
	
	yy0=ceil(y0);
	yy1=ceil(y1);
	yy2=ceil(y2);

	if(yy0==yy2)
		{
		//do a span?
		return;
		}

	m2=(x2-x0)/(y2-y0);
	l=x0+(yy0-y0)*m2;

	if(yy0!=yy1)
		{
		m1=(x1-x0)/(y1-y0);
		r=x0+(yy0-y0)*m1;
		for(y=yy0;y<yy1;y++)
			{
			doSpan(pixmap,l,r,y,Cs);
			l+=m2;
			r+=m1;
			}
		}

	if(yy1!=yy2)
		{
		m1=(x2-x1)/(y2-y1);
		r=x1+(yy1-y1)*m1;

		for(y=yy1;y<yy2;y++)
			{
			doSpan(pixmap,l,r,y,Cs);
			l+=m2;
			r+=m1;
			}
		}
	}

void fillQuad(NCCAPixmap pixmap,float x0, float y0,
								float x1, float y1,
								float x2, float y2,
								float x3, float y3,
								NCCAPixel Cs)
	{
	fillTriangle(pixmap,x0+0.5,y0+0.5,
						x1+0.5,y1+0.5,
						x2+0.5,y2+0.5, Cs);
	fillTriangle(pixmap,x1+0.5,y1+0.5,
						x2+0.5,y2+0.5,
						x3+0.5,y3+0.5, Cs);
	}



void ZfillTriangle(NCCAPixmap pixmap,NCCAPixmap zbuffer,
					float x0, float y0,float z0,NCCAPixel c0,
					float x1, float y1,float z1,NCCAPixel c1,
					float x2, float y2,float z2,NCCAPixel c2)
	{
	int y;
	int yy0,yy1,yy2;
	float m1,m2;
	float zm1,zm2;
	float l,r;
	float lz,rz;
	NCCAPixel cm1,cm2;
	NCCAPixel lc,rc;
	
	if(y0>y1)
		{
		float tx,ty;
		float tz;
		NCCAPixel tc;
		tx=x0;ty=y0;tz=z0;tc=c0;
		x0=x1;y0=y1;z0=z1;c0=c1;
		x1=tx;y1=ty;z1=tz;c1=tc;
		}
	if(y0>y2)
		{
		float tx,ty;
		float tz;
		NCCAPixel tc;
		tx=x0;ty=y0;tz=z0;tc=c0;
		x0=x2;y0=y2;z0=z2;c0=c2;
		x2=tx;y2=ty;z2=tz;c2=tc;
		}
	if(y1>y2)
		{
		float tx,ty;
		float tz;
		NCCAPixel tc;
		tx=x1;ty=y1;tz=z1;tc=c1;
		x1=x2;y1=y2;z1=z2;c1=c2;
		x2=tx;y2=ty;z2=tz;c2=tc;
		}		

	yy0=ceil(y0);
	yy1=ceil(y1);
	yy2=ceil(y2);

	if(yy2<0 || yy0 >=pixmap.height)
		return;

	if(yy0==yy2)
		{
		return;
		}

	{
	float startDelta=yy0-y0;
	float oowidth=1/(y2-y0);

	m2=(x2-x0)*oowidth;
	zm2=(z2-z0)*oowidth;
	cm2.r=(c2.r-c0.r)*oowidth;
	cm2.g=(c2.g-c0.g)*oowidth;
	cm2.b=(c2.b-c0.b)*oowidth;
	cm2.a=(c2.a-c0.a)*oowidth;

	l=x0+startDelta*m2;
	lz=z0+startDelta*zm2;
	lc.r=c0.r+startDelta*cm2.r;
	lc.g=c0.g+startDelta*cm2.g;
	lc.b=c0.b+startDelta*cm2.b;
	lc.a=c0.a+startDelta*cm2.a;
	}

	if(y0!=y1)
		{
		float startDelta=yy0-y0;
		float oowidth=1/(y1-y0);
		m1=(x1-x0)*oowidth;
		zm1=(z1-z0)*oowidth;
		cm1.r=(c1.r-c0.r)*oowidth;
		cm1.g=(c1.g-c0.g)*oowidth;
		cm1.b=(c1.b-c0.b)*oowidth;
		cm1.a=(c1.a-c0.a)*oowidth;

		r=x0+startDelta*m1;
		rz=z0+startDelta*zm1;
		rc.r=c0.r+startDelta*cm1.r;
		rc.g=c0.g+startDelta*cm1.g;
		rc.b=c0.b+startDelta*cm1.b;
		rc.a=c0.a+startDelta*cm1.a;


		for(y=yy0;y<yy1 && y<pixmap.height;y++)
			{
			doZSpan(pixmap,zbuffer,l,r,lz,rz,lc,rc,y);
			l+=m2;
			r+=m1;
			lz+=zm2;
			rz+=zm1;
			lc.r+=cm2.r;
			lc.g+=cm2.g;
			lc.b+=cm2.b;
			lc.a+=cm2.a;
			rc.r+=cm1.r;
			rc.g+=cm1.g;
			rc.b+=cm1.b;
			rc.a+=cm1.a;
			}
		}
	
	if(yy1!=yy2)
		{
		float startDelta=yy1-y1;
		float oowidth=1/(y2-y1);
		m1=(x2-x1)*oowidth;
		zm1=(z2-z1)*oowidth;
		cm1.r=(c2.r-c1.r)*oowidth;
		cm1.g=(c2.g-c1.g)*oowidth;
		cm1.b=(c2.b-c1.b)*oowidth;
		cm1.a=(c2.a-c1.a)*oowidth;

		r=x1+startDelta*m1;
		rz=z1+startDelta*zm1;
		rc.r=c1.r+startDelta*cm1.r;
		rc.g=c1.g+startDelta*cm1.g;
		rc.b=c1.b+startDelta*cm1.b;
		rc.a=c1.a+startDelta*cm1.a;

		for(y=yy1;y<yy2 && y<pixmap.height;y++)
			{
			doZSpan(pixmap,zbuffer,l,r,lz,rz,lc,rc,y);
			l+=m2;
			r+=m1;
			lz+=zm2;
			rz+=zm1;
			lc.r+=cm2.r;
			lc.g+=cm2.g;
			lc.b+=cm2.b;
			lc.a+=cm2.a;
			rc.r+=cm1.r;
			rc.g+=cm1.g;
			rc.b+=cm1.b;
			rc.a+=cm1.a;
			}
		}
	}

void ZfillQuad(NCCAPixmap pixmap,NCCAPixmap zbuffer,
						float x0, float y0,float z0,NCCAPixel c0,
						float x1, float y1,float z1,NCCAPixel c1,
						float x2, float y2,float z2,NCCAPixel c2,
						float x3, float y3,float z3,NCCAPixel c3)
	{
	ZfillTriangle(pixmap,zbuffer,
						x0+0.5, y0+0.5, z0, c0,
						x1+0.5, y1+0.5, z1, c1,
						x2+0.5, y2+0.5, z2, c2);
	ZfillTriangle(pixmap,zbuffer,
						x1+0.5, y1+0.5, z1, c1,
						x2+0.5, y2+0.5, z2, c2,
						x3+0.5, y3+0.5, z3, c3);
	}	


void ZmatteQuad(NCCAPixmap pixmap,NCCAPixmap zbuffer,
						float x0, float y0,float z0,
						float x1, float y1,float z1,
						float x2, float y2,float z2,
						float x3, float y3,float z3,float coverage)
	{
	ZmatteTriangle(pixmap,zbuffer,
						x0+0.5, y0+0.5, z0,
						x1+0.5, y1+0.5, z1,
						x2+0.5, y2+0.5, z2, coverage);
	ZmatteTriangle(pixmap,zbuffer,
						x1+0.5, y1+0.5, z1,
						x2+0.5, y2+0.5, z2,
						x3+0.5, y3+0.5, z3, coverage);
	}	

void ZmatteTriangle(NCCAPixmap pixmap,NCCAPixmap zbuffer,
						float x0, float y0,float z0,
						float x1, float y1,float z1,
						float x2, float y2,float z2,float coverage)
	{
	int y;
	int yy0,yy1,yy2;
	float m1,m2;
	float zm1,zm2;
	float l,r;
	float lz,rz;
	
	if(y0>y1)
		{
		float tx,ty;
		float tz;
		tx=x0;ty=y0;tz=z0;
		x0=x1;y0=y1;z0=z1;
		x1=tx;y1=ty;z1=tz;;
		}
	if(y0>y2)
		{
		float tx,ty;
		float tz;
		tx=x0;ty=y0;tz=z0;
		x0=x2;y0=y2;z0=z2;
		x2=tx;y2=ty;z2=tz;
		}
	if(y1>y2)
		{
		float tx,ty;
		float tz;
		tx=x1;ty=y1;tz=z1;
		x1=x2;y1=y2;z1=z2;
		x2=tx;y2=ty;z2=tz;
		}		

	yy0=ceil(y0);
	yy1=ceil(y1);
	yy2=ceil(y2);

	if(yy2<0 || yy0 >=pixmap.height)
		return;

	if(yy0==yy2)
		{
		return;
		}

	{
	float startDelta=yy0-y0;
	float oowidth=1/(y2-y0);

	m2=(x2-x0)*oowidth;
	zm2=(z2-z0)*oowidth;

	l=x0+startDelta*m2;
	lz=z0+startDelta*zm2;
	}

	if(y0!=y1)
		{
		float startDelta=yy0-y0;
		float oowidth=1/(y1-y0);
		m1=(x1-x0)*oowidth;
		zm1=(z1-z0)*oowidth;

		r=x0+startDelta*m1;
		rz=z0+startDelta*zm1;


		for(y=yy0;y<yy1 && y<pixmap.height;y++)
			{
			matteZSpan(pixmap,zbuffer,l,r,lz,rz,coverage,y);
			l+=m2;
			r+=m1;
			lz+=zm2;
			rz+=zm1;
			}
		}
	
	if(yy1!=yy2)
		{
		float startDelta=yy1-y1;
		float oowidth=1/(y2-y1);
		m1=(x2-x1)*oowidth;
		zm1=(z2-z1)*oowidth;

		r=x1+startDelta*m1;
		rz=z1+startDelta*zm1;

		for(y=yy1;y<yy2 && y<pixmap.height;y++)
			{
			matteZSpan(pixmap,zbuffer,l,r,lz,rz,coverage,y);
			l+=m2;
			r+=m1;
			lz+=zm2;
			rz+=zm1;
			}
		}
	}




void drawLine(NCCAPixmap pixmap,float x0, float y0, float x1, float y1, NCCAPixel Cs)
	{
	float x,y;
	float dy,dx,m;
		
	if(x1==x0 && y1==y0)
		return;

	dx=x1-x0;
	dy=y1-y0;

	if(fabs(dx)>fabs(dy))
		{
		if(dx<0)
			{
			float tx,ty;
			tx=x0;ty=y0;
			x0=x1;y0=y1;
			x1=tx;y1=ty;

			dx=-dx;
			dy=-dy;
			}
		y=y0;
		m=dy/dx;

		for(x=x0;x<x1;x++)
			{
			if(x>=0 && x<pixmap.width && y>=0 && y<pixmap.height)
				setPixelColor(pixmap,x,y,Cs);
			y+=m;
			}
		}
	else
		{
		if(dy<0)
			{
			float tx,ty;
			tx=x0;ty=y0;
			x0=x1;y0=y1;
			x1=tx;y1=ty;

			dx=-dx;
			dy=-dy;
			}
		x=x0;
		m=dx/dy;

		for(y=y0;y<y1;y++)
			{
			if(x>=0 && x<pixmap.width && y>=0 && y<pixmap.height)
				setPixelColor(pixmap,x,y,Cs);
			x+=m;
			}
		}
	}
