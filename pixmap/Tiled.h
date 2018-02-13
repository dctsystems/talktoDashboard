#ifdef __cplusplus
extern "C" {
#endif


typedef struct tiledImage_t
	{
	void *file;
	int level;
	void *next;

	int height;
	int width;
	int spp;
	int bps;
	
	int bufSize;
	int tilesPerRow;
	int rowsPerImage;
	int tileWidth;
	int tileHeight;

	NCCAPixmap **tile;

	} TiledImage;

TiledImage loadTiledImage(char *filename);
NCCAPixel getPixelT(TiledImage,float x, float y);
void destroyTiledImage(TiledImage);

NCCAPixel getPixelM(TiledImage tt,float s, float t, float ds, float dt) ;

#ifdef __cplusplus
} /* extern "C" */
#endif


