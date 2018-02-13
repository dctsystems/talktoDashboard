 /**********************************************************************
 *
 * (c) Ian Stephenson, ian@dctsystems.freeserve.co.uk
 * $Log: PixelMaths.c,v $
 * Revision 1.3  2001/01/08 20:14:20  ian
 * add pixelLerp
 *
 * Revision 1.2  2001/01/01 14:16:23  ian
 * added pixelAdd to add a scaler to a pixel
 *
 * Revision 1.1  2000/12/28 19:10:00  ian
 * Initial revision
 *
 *
 **********************************************************************/

#include "NCCAPixmap.h"


NCCAPixel pixelAverage(NCCAPixel p,NCCAPixel q)
	{
	NCCAPixel r;
	r.r=(p.r+q.r)/2;
	r.g=(p.g+q.g)/2;
	r.b=(p.b+q.b)/2;
	r.a=(p.a+q.a)/2;
	return r;
	}

NCCAPixel pixelSum(NCCAPixel p,NCCAPixel q)
	{
	NCCAPixel r;
	r.r=(p.r+q.r);
	r.g=(p.g+q.g);
	r.b=(p.b+q.b);
	r.a=(p.a+q.a);
	return r;
	}

NCCAPixel pixelDifference(NCCAPixel p,NCCAPixel q)
	{
	NCCAPixel r;
	r.r=(p.r-q.r);
	r.g=(p.g-q.g);
	r.b=(p.b-q.b);
	r.a=(p.a-q.a);
	return r;
	}


NCCAPixel pixelScale(NCCAPixel p,float q)
	{
	NCCAPixel r;
	r.r=(p.r*q);
	r.g=(p.g*q);
	r.b=(p.b*q);
	r.a=(p.a*q);
	return r;
	}
NCCAPixel pixelAdd(NCCAPixel p,float q)
	{
	NCCAPixel r;
	r.r=(p.r+q);
	r.g=(p.g+q);
	r.b=(p.b+q);
	r.a=(p.a+q);
	return r;
	}

NCCAPixel pixelLerp(NCCAPixel p,NCCAPixel q, float a)
	{
	NCCAPixel r;
	r.r=p.r+a*(q.r-p.r);
	r.g=p.g+a*(q.g-p.g);
	r.b=p.b+a*(q.b-p.b);
	r.a=p.a+a*(q.a-p.a);
	return r;
	}
