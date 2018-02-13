 /**********************************************************************
 *
 * (c) Ian Stephenson, ian@dctsystems.freeserve.co.uk
 * $Log: PixBlit.h,v $
 * Revision 1.3  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
 * Revision 1.2  2000/09/17 14:01:54  ian
 * add CopyPixmap
 *
 * Revision 1.1  2000/03/25 21:06:18  ian
 * Initial revision
 *
 *
 **********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void pixmapClearGrey(NCCAPixmap p, float val);
void pixmapInsert(NCCAPixmap src,int xPos, int yPos, NCCAPixmap dest);
void pixmapInsertAdd(NCCAPixmap src,int xPos, int yPos, NCCAPixmap dest);
NCCAPixmap pixmapExtract(NCCAPixmap src,int xPos, int yPos, int width, int height);
NCCAPixmap convertPixmapToGreyscale(NCCAPixmap src);

#define NCCA_BOXFILTER 0
#define NCCA_TRIANGLEFILTER 1
NCCAPixmap reducePixmap(NCCAPixmap src, int destBPS,
							int xScale, int yScale, int filter);
NCCAPixmap convertPixmap(NCCAPixmap src, int destBPS,
							float gain, float gamma);

NCCAPixmap copyPixmap(NCCAPixmap src);


#ifdef __cplusplus
} /* extern "C" */
#endif


