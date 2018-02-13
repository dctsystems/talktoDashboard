/*/
 * $LOG$
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef NeXT
#include <libc.h>
#endif

#include <assert.h>

#include "tiff.h"
#include "tiffio.h"

#include "NCCAPixmap.h"
#include "Tiled.h"

static TiledImage loadMipLevel(TIFF *tp,int l)
	{
	TiledImage t;
	int i;
	uint32 h,w;
        uint16 samplesperpixel;
        uint16 bitspersample;

	t.file=tp;
	t.level=l;
	TIFFSetDirectory(t.file,t.level);

	if(!TIFFIsTiled(t.file))
		{
		fprintf(stderr,"Tiff not tiled\n");
		t.file=NULL;
		t.next=NULL;
		return t;
		}

	(void) TIFFGetField(t.file, TIFFTAG_TILEWIDTH, &t.tileWidth);
	(void) TIFFGetField(t.file, TIFFTAG_TILELENGTH, &t.tileHeight);
        TIFFGetField(t.file, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(t.file, TIFFTAG_IMAGELENGTH, &h);
	t.width=w;
	t.height=h;
        TIFFGetField(t.file, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
        t.spp=samplesperpixel;
        TIFFGetField(t.file, TIFFTAG_BITSPERSAMPLE, &bitspersample);
        t.bps=bitspersample;
	t.bufSize=TIFFTileSize(t.file);

	//printf("image %dx%d\n",t.width,t.height);
	//printf("per tile %dx%d\n",t.tileWidth,t.tileHeight);
	if(t.width>=t.tileWidth)
		{
		t.tilesPerRow=t.width/t.tileWidth;
		assert(t.tilesPerRow*t.tileWidth==t.width);
		}
	else
		t.tilesPerRow=1;

	if(t.height>=t.tileHeight)
		{
		t.rowsPerImage=h/t.tileHeight;
		assert(t.rowsPerImage*t.tileHeight==h);
		}
	else
		t.rowsPerImage=1;

	t.tile=malloc(t.tilesPerRow*t.rowsPerImage*sizeof(NCCAPixmap *));

	//printf("%dx%d\n",t.tilesPerRow,t.rowsPerImage);

	for(i=0;i<t.tilesPerRow*t.rowsPerImage;i++)
		t.tile[i]=NULL;
		
	if(TIFFSetDirectory(t.file,t.level+1))
		{
		t.next=malloc(sizeof(TiledImage));
		*(TiledImage *)t.next=loadMipLevel(t.file,t.level+1);
		}
	else
		t.next=NULL;
	return t;
	}

TiledImage loadTiledImage(char *filename)
	{
	TIFF *tp;
	tp=TIFFOpen(filename, "r");
	if(tp==NULL)
		{
		TiledImage n={NULL,0,NULL,
				0,0,0,0,
				0,0,0,0,0,
				NULL};
		fprintf(stderr,"%s doesn't exist tiled\n",filename);
		return n;
		}
	return loadMipLevel(tp,0);
	}

NCCAPixel getPixelT(TiledImage t,float x, float y)
	{
	NCCAPixel p={0,0,0,0};
	int row,col;
	float xx,yy;
	int whichTile;
	col=x/t.tileWidth;
	row=y/t.tileHeight;

	if(col>=t.tilesPerRow)
		col=t.tilesPerRow-1;
	if(row>=t.rowsPerImage)
		row=t.rowsPerImage-1;
	if(col<0)
		col=0;
	if(row<0)
		row=0;

	whichTile=row*t.tilesPerRow+col;
	assert(whichTile>=0 && whichTile<t.tilesPerRow*t.rowsPerImage);

	if(t.tile[whichTile]==NULL)
		{
		//Load Tile
		//printf("Load Tile %d,%d\n",col,row);
		t.tile[whichTile]=malloc(sizeof(NCCAPixmap));
		*t.tile[whichTile]=newPixmap(t.tileWidth,t.tileHeight,t.spp,t.bps);
		assert(t.tile[whichTile]->data);
		TIFFSetDirectory(t.file,t.level);
		TIFFReadEncodedTile(t.file, whichTile, t.tile[whichTile]->data, t.bufSize);
		}
	xx=x-(t.tileWidth*col);
	yy=y-(t.tileHeight*row);
	if(t.tile[whichTile])
		p=getAAPixel(*t.tile[whichTile],xx,yy);

	return p;
	}

NCCAPixel getPixelM(TiledImage tt,float s, float t, float ds, float dt)
	{
	NCCAPixel p;
	
	if(tt.next && (ds*tt.width>2 || ds*tt.width>2))
		p=getPixelM(*(TiledImage *)tt.next,s,t,ds,dt);
	else
		p=getPixelT(tt,s*(tt.width-1),t*(tt.height-1));
	return p;
	}

void destroyTiledImage(TiledImage t)
{
int i;
if(t.next)
	destroyTiledImage(*(TiledImage *)t.next);

for(i=0;i<t.tilesPerRow*t.rowsPerImage;i++) 
	{
	if(t.tile[i]!=NULL)
		{
		destroyPixmap(*t.tile[i]);
		free(t.tile[i]);
		}
	}
free(t.tile);

if(t.level==0)
	TIFFClose(t.file);
}
