/*
 * $Log: MipMap.h,v $
 * Revision 1.1  2007/06/28 14:12:00  ian
 * Initial revision
 *
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MIP_PLANES 16

typedef struct Mipmap_t
	{
	NCCAPixmap plane[MAX_MIP_PLANES];
	} MipMap;


MipMap newMipMap(int w, int h, int spp, int bps);

void destroyMipMap( MipMap m);

void ZMIPfillQuad(MipMap pixmap, MipMap zbuffer,
						float x0, float y0,float z0,NCCAPixel c0,
						float x1, float y1,float z1,NCCAPixel c1,
						float x2, float y2,float z2,NCCAPixel c2,
						float x3, float y3,float z3,NCCAPixel c3);
void flattenZMip(MipMap m, MipMap z);


#ifdef __cplusplus
} /* extern "C" */
#endif


