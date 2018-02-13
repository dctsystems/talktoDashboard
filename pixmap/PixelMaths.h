 /**********************************************************************
 *
 * (c) Ian Stephenson, ian@dctsystems.freeserve.co.uk
 * $Log: PixelMaths.h,v $
 * Revision 1.4  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
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
#ifdef __cplusplus
extern "C" {
#endif

NCCAPixel pixelAverage(NCCAPixel p,NCCAPixel q);
NCCAPixel pixelSum(NCCAPixel p,NCCAPixel q);
NCCAPixel pixelDifference(NCCAPixel p,NCCAPixel q);

NCCAPixel pixelScale(NCCAPixel p,float q);
NCCAPixel pixelAdd(NCCAPixel p,float q);

NCCAPixel pixelLerp(NCCAPixel p,NCCAPixel q, float a);
#ifdef __cplusplus
} /* extern "C" */
#endif

