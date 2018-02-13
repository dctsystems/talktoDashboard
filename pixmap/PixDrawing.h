 /**********************************************************************
 *
 * (c) Ian Stephenson, ian@dctsystems.freeserve.co.uk
 * $Log: PixDrawing.h,v $
 * Revision 1.9  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
 * Revision 1.8  2002/04/25 15:05:06  ian
 * add matte functions
 *
 * Revision 1.7  2001/07/04 11:49:31  ian
 * add setInterlace functions to reduce supoprt Motion blur in Angel
 *
 * Revision 1.6  2000/10/03 21:28:30  ian
 * Fix (and optimise) ZFillTriangle
 *
 * Revision 1.5  2000/10/02 18:50:39  ian
 * finally (?) fix edges in drawTriangle (2d version)
 *
 * Revision 1.4  2000/10/01 18:55:12  ian
 * fill triangle parameters are fixed
 *
 * Revision 1.3  1999/12/04 19:11:57  ian
 * export Triangle routines for poly drawing
 *
 * Revision 1.2  99/12/02  12:18:05  ian
 * add smooth shading to zfill functions
 * 
 * Revision 1.1  99/12/02  11:14:19  ian
 * Initial revision
 * 
 *
 **********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void drawLine(NCCAPixmap pixmap,float x0, float y0,
								float x1, float y1,
								NCCAPixel Cs);
void fillQuad(NCCAPixmap pixmap,float x0, float y0,
								float x1, float y1,
								float x2, float y2,
								float x3, float y3,
								NCCAPixel Cs);
void fillTriangle(NCCAPixmap pixmap, float x0, float y0,
								float x1, float y1,
								float x2, float y2,
								NCCAPixel Cs);

void ZfillQuad(NCCAPixmap pixmap,NCCAPixmap zbuffer,
						float x0, float y0,float z0,NCCAPixel c0,
						float x1, float y1,float z1,NCCAPixel c1,
						float x2, float y2,float z2,NCCAPixel c2,
						float x3, float y3,float z3,NCCAPixel c3);
void ZfillTriangle(NCCAPixmap pixmap,NCCAPixmap zbuffer,
						float x0, float y0,float z0,NCCAPixel c0,
						float x1, float y1,float z1,NCCAPixel c1,
						float x2, float y2,float z2,NCCAPixel c2);

void ZmatteQuad(NCCAPixmap pixmap,NCCAPixmap zbuffer,
						float x0, float y0,float z0,
						float x1, float y1,float z1,
						float x2, float y2,float z2,
						float x3, float y3,float z3,float coverage);
void ZmatteTriangle(NCCAPixmap pixmap,NCCAPixmap zbuffer,
						float x0, float y0,float z0,
						float x1, float y1,float z1,
						float x2, float y2,float z2,float coverage);

void setInterlace(int which,int of);

#ifdef __cplusplus
} /* extern "C" */
#endif

