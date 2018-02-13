/*
 * $Log: NCCAPixmap.h,v $
 * Revision 1.14  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
 * Revision 1.13  2007/06/28 14:11:00  ian
 * Make funciton names more consistant
 *
 * Revision 1.12  2000/09/10 16:11:42  ian
 * move the file handling to PixFileIO
 *
 * Revision 1.11  2000/03/25 21:07:20  ian
 * Split out the area functions into PixBlit
 *
 * Revision 1.10  2000/03/02 20:46:15  ian
 * add convertPixmap to do gamma correction and dithering
 * add support for 16bps
 * 
 * Revision 1.9  10/.0/.0  .1:.0:.2  ian
 * add reducePixmap function
 * 
 * Revision 1.8  99/12/08  18:13:07  ian
 * Add limited support for fp image depths
 * (For Angel)
 * 
 * Revision 1.7  99/08/17  21:38:37  ian
 * added pixmapInsert (blits src into dest at given postion)
 * needed for SLander bucketing
 * 
 * Revision 1.6  99/05/28  16:52:56  ian
 * added getAApixel
 * 
 * Revision 1.5  1998/03/27 19:30:40  amarsh
 * added isSeq sequence test

Revision 1.4  1998/03/27 17:09:37  selliott
added prototype for getGlobalpixel

Revision 1.3  1998/03/10 16:18:59  selliott
added displacement field

Revision 1.2  1998/02/24 16:16:58  amarsh
add alpha support

Revision 1.1  1998/02/23 14:50:24  istephen
Initial revision

*/
#ifdef __cplusplus
extern "C" {
#endif


typedef struct NCCAPixmap_t
	{
	int height;
	int width;

	unsigned char *data;

	int bps;
	int spp;

	} NCCAPixmap;

typedef struct NCCAPixel_t
	{
	float r, g, b, a;
	} NCCAPixel;
	

#define NCCA_PIXMAP_GRAY 1
#define NCCA_PIXMAP_GRAYA 2
#define NCCA_PIXMAP_RGB 3
#define NCCA_PIXMAP_RGBA 4



NCCAPixmap newPixmap(int w, int h, int spp, int bps);
NCCAPixmap newPixmapLike( NCCAPixmap p1);

void destroyPixmap( NCCAPixmap p);

float getPixelGrey(NCCAPixmap, int xOffset, int yOffset);
NCCAPixel getPixelColor(NCCAPixmap, int xOffset, int yOffset);
float getAAPixelGrey(NCCAPixmap pix, float xOffset, float yOffset);
NCCAPixel getAAPixel(NCCAPixmap, float xOffset, float yOffset);


void setPixelGrey(NCCAPixmap, int x, int y, float colour);
void setPixelColor(NCCAPixmap p, int x, int y,  NCCAPixel colour);

#ifdef __cplusplus
} /* extern "C" */
#endif

