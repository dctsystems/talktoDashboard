/*
 * $Log: MipMap1.c,v $
 * Revision 1.1  2007/06/28 14:12:00  ian
 * Initial revision
 *
 *
 */
#include <stdio.h>

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


#include "NCCAPixmap.h"
#include "MipMap.h"
#include "PixDrawing.h"
#include "PixelMaths.h"

MipMap newMipMap(int w, int h, int spp, int bps)
{
int i;
MipMap m;
m.plane[0]=newPixmap(w,h,spp,bps);
for(i=1;i<MAX_MIP_PLANES;i++)
	{
	w=(w+1)>>1;
	h=(h+1)>>1;
	if(w>1 && h>1)
		{
		m.plane[i]=newPixmap(w,h,spp,bps);
		}
	else
		{
		break;
		}
	}
for(;i<MAX_MIP_PLANES;i++)
	{
	m.plane[i].data=NULL;
	}
	
return m;
}

void destroyMipMap( MipMap m)
	{
	int i;
	for(i=0;i<MAX_MIP_PLANES;i++)
		{
		if(m.plane[i].data)
			destroyPixmap(m.plane[i]);
		}
	}

static void ZMIPSetPixel(MipMap pixmap,MipMap zbuffer,
		int x,int y, float z,
		NCCAPixel C)
	{
	int i;

	for(i=0;i<MAX_MIP_PLANES;i++)
		{
		float oldZ;
		NCCAPixel oldC;
		if(pixmap.plane[i].data==NULL)
			return;
		if(C.a==0)
			return;

		oldZ=getPixelGrey(zbuffer.plane[i],x,y);
		oldC=getPixel(pixmap.plane[i],x,y);
		//printf("%d:%d,%d:%f,%f\n",i,x,y,z,oldZ);
		if(z<oldZ)
				{

				setPixelGrey(zbuffer.plane[i],x,y, z);
				if(pixmap.plane[i].data)
					setPixelColour(pixmap.plane[i],x,y, C);
				
				if(C.a==1)
					return;

				C=oldC;
				z=oldZ;
				}
		else
			{
			if(oldC.a==1)
				return;
			}
		if((x&1)||(y&1))
			return;
		x=x>>1;
		y=y>>1;
		}
	}
NCCAPixel getFlattenedPixel(MipMap m,int p, int x, int y)
	{
	NCCAPixel C,D,tmp;
	float scale;
	C=getPixel(m.plane[p],x,y);

	if(p==MAX_MIP_PLANES-1 ||m.plane[p+1].data==NULL)
		return C;

	if((x&1)&&(y&1))
		{
		D=getFlattenedPixel(m,p+1,x>>1,y>>1);
		tmp=getFlattenedPixel(m,p+1,(x+1)>>1,(y+1)>>1);
		D=pixelSum(D,tmp);
		tmp=getFlattenedPixel(m,p+1,(x)>>1,(y+1)>>1);
		D=pixelSum(D,tmp);
		tmp=getFlattenedPixel(m,p+1,(x+1)>>1,(y)>>1);
		D=pixelSum(D,tmp);
		scale=4;
		}
	else if((x&1))
		{
		D  =getFlattenedPixel(m,p+1,x>>1,y>>1);
		tmp=getFlattenedPixel(m,p+1,(x+1)>>1,y>>1);
		D=pixelSum(D,tmp);
		scale=2;
		}
	else if((y&1))
		{
		D=getFlattenedPixel(m,p+1,x>>1,y>>1);
		tmp=getFlattenedPixel(m,p+1,x>>1,(y+1)>>1);
		D=pixelSum(D,tmp);
		scale=2;
		}
	else
		{
		D=getFlattenedPixel(m,p+1,x>>1,y>>1);
		scale=1;
		}

	C.r+=(1-C.a)*D.r/scale;
	C.g+=(1-C.a)*D.g/scale;
	C.b+=(1-C.a)*D.b/scale;
	C.a+=(1-C.a)*D.a/scale;
	return C;
	}

void flattenZMip(MipMap m)
	{
	int x,y,p;
	
	for(p=MAX_MIP_PLANES-1;m.plane[p].data==NULL;p--)
		/*Find first Real Plane*/;

	for(;p>0;p--)
		{
		for(x=0;x<m.plane[p-1].width;x++)
			{
			for(y=0;y<m.plane[p-1].height;y++)
				{
				NCCAPixel D,tmp;
				NCCAPixel C=getPixel(m.plane[p-1],x,y);

				//if(C.a==0 || C.a==1)
				//	continue; //Don't comp unnecessary pixels!
				
				D=getPixel(m.plane[p],x>>1,y>>1);
				tmp=getPixel(m.plane[p],(x+1)>>1,(y+1)>>1);
				D=pixelSum(D,tmp);
				tmp=getPixel(m.plane[p],(x)>>1,(y+1)>>1);
				D=pixelSum(D,tmp);
				tmp=getPixel(m.plane[p],(x+1)>>1,(y)>>1);
				D=pixelSum(D,tmp);
				
				C.r+=(1-C.a)*D.r/4;
				C.g+=(1-C.a)*D.g/4;
				C.b+=(1-C.a)*D.b/4;
				C.a+=(1-C.a)*D.a/4;
				setPixelColour(m.plane[p-1],x,y,C);
				}
			}
		}
	}

static void doZSpan(MipMap pixmap,MipMap zbuffer,
								float l, float r,
								float lz, float rz,
								NCCAPixel lc,NCCAPixel rc,
								int y
								)
{
	int ll,rr;
	if(y<0 || y>=pixmap.plane[0].height)
		return;

//	if(y%mbPasses!=mbPass)
//		return;

	ll=ceil(l);
	rr=ceil(r);

	switch(rr-ll)
		{
		case 0:
			return;
		case 1:
			{
			if(ll>=0 && ll<pixmap.plane[0].width)
				{
				float v=(ll-l)/(r-l);
				float z=lz+v*(rz-lz);
				NCCAPixel Cs;
				Cs.r=lc.r+v*(rc.r-lc.r);
				Cs.g=lc.g+v*(rc.g-lc.g);
				Cs.b=lc.b+v*(rc.b-lc.b);
				Cs.a=lc.a+v*(rc.a-lc.a);
				ZMIPSetPixel(pixmap,zbuffer,ll,y,z,Cs);
				}
			return;
			}
		case -1:
			{
			if(rr>=0 && rr<pixmap.plane[0].width)
				{
				float v=(rr-r)/(l-r);
				float z=rz+v*(lz-rz);
				NCCAPixel Cs;
				Cs.r=rc.r+v*(lc.r-rc.r);
				Cs.g=rc.g+v*(lc.g-rc.g);
				Cs.b=rc.b+v*(lc.b-rc.b);
				Cs.a=rc.a+v*(lc.a-rc.a);
				
				ZMIPSetPixel(pixmap,zbuffer,rr,y,z,Cs);
				}
			return;
			}
		default:
			{
			int x,end;
			float newz;
			NCCAPixel Cs;
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
		
			if(rr<0 || ll>=pixmap.plane[0].width)
				return;
		
			if(rr>pixmap.plane[0].width)
				end=pixmap.plane[0].width;
			else
				end=rr;

			x=ll;
			if(x<0)
				x=0;
			
			oowidth=1.0/(r-l);
			mz=(rz-lz)*oowidth;
			mC.a=(rc.a-lc.a)*oowidth;
			mC.r=(rc.r-lc.r)*oowidth;
			mC.g=(rc.g-lc.g)*oowidth;
			mC.b=(rc.b-lc.b)*oowidth;
			
			startDelta=x-l;
			newz=lz+startDelta*mz;

			Cs.r=lc.r+startDelta*mC.r;
			Cs.g=lc.g+startDelta*mC.g;
			Cs.b=lc.b+startDelta*mC.b;					
			Cs.a=lc.a+startDelta*mC.a;

			for(;x<end;x++)
				{
				ZMIPSetPixel(pixmap,zbuffer,x,y,newz,Cs);
				
				newz+=mz;
				Cs.a+=mC.a;
				Cs.r+=mC.r;
				Cs.g+=mC.g;
				Cs.b+=mC.b;
				}
			}
		}
	}

void ZMIPfillTriangle(MipMap pixmap,MipMap zbuffer,
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

	if(yy2<0 || yy0 >=pixmap.plane[0].height)
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


		for(y=yy0;y<yy1 && y<pixmap.plane[0].height;y++)
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

		for(y=yy1;y<yy2 && y<pixmap.plane[0].height;y++)
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


void ZMIPfillQuad(MipMap pixmap,MipMap zbuffer,
						float x0, float y0,float z0,NCCAPixel c0,
						float x1, float y1,float z1,NCCAPixel c1,
						float x2, float y2,float z2,NCCAPixel c2,
						float x3, float y3,float z3,NCCAPixel c3)
	{
	ZMIPfillTriangle(pixmap,zbuffer,
						x0+0.5, y0+0.5, z0, c0,
						x1+0.5, y1+0.5, z1, c1,
						x2+0.5, y2+0.5, z2, c2);
	ZMIPfillTriangle(pixmap,zbuffer,
						x1+0.5, y1+0.5, z1, c1,
						x2+0.5, y2+0.5, z2, c2,
						x3+0.5, y3+0.5, z3, c3);
	}	
