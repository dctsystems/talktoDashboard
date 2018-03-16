/*
 * $Log: PixFileIO.c,v $
 * Revision 1.7  2007/06/28 14:10:38  ian
 * Add GIF support
 * remove original dumb PIX format support
 *
 * Revision 1.6  2004/08/26 13:29:56  ian
 * Add correct line padding to BMP reading code
 *
 * Revision 1.5  2004/03/16 13:46:31  ian
 * add limited BMP support
 *
 * Revision 1.4  2002/05/27 19:13:12  ian
 * recognise uper case file extensions
 *
 * Revision 1.3  2001/09/10 17:16:49  ian
 * add support for Tiled TIFFS - aka BMRT mips
 *
 * Revision 1.2  2000/11/26 18:57:21  ian
 * make JPEG Support optional for MAC port
 *
 * Revision 1.1  2000/09/10 16:11:05  ian
 * Initial revision
 *
 *
 */
 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef NeXT
#include <libc.h>
#endif

#include "NCCAPixmap.h"
#include "PixFileIO.h"

#define TIFFSUPPORT
#define JPEGSUPPORT
//#define EXRSUPPORT

#ifdef WIN32
#undef EXRSUPPORT
#endif

#ifdef sun
#undef EXRSUPPORT
#endif


#ifdef TIFFSUPPORT
#include <tiffio.h>
#endif

#ifdef WIN32
#define STDIO_READ "rb"
#define STDIO_WRITE "wb"
#else
#define STDIO_READ "r"
#define STDIO_WRITE "w"
#endif

#include <assert.h>


#define PGM_MAGIC1 'P'
#define PGM_MAGIC2 '2'
#define RPGM_MAGIC2 '5'
#define PGM_FORMAT (PGM_MAGIC1 * 256 + PGM_MAGIC2)
#define RPGM_FORMAT (PGM_MAGIC1 * 256 + RPGM_MAGIC2)

#define PPM_MAGIC1 'P'
#define PPM_MAGIC2 '3'
#define RPPM_MAGIC2 '6'
#define PPM_FORMAT (PPM_MAGIC1 * 256 + PPM_MAGIC2)
#define RPPM_FORMAT (PPM_MAGIC1 * 256 + RPPM_MAGIC2)

static NCCAPixmap loadPNM(char *filename);
static NCCAPixmap loadPGM(char *filename);
static NCCAPixmap loadPPM(char *filename);
static NCCAPixmap loadTIFF(char *filename);
static NCCAPixmap loadJPG(char *filename);
static NCCAPixmap loadEXR(char *filename);
static NCCAPixmap loadBMP(char *filename);
static NCCAPixmap loadGIF(char *filename);
static void savePIC(NCCAPixmap p, char *filename);
static void savePGM(NCCAPixmap p, char *filename);
static void saveTIFF(NCCAPixmap p, char *filename);
static void saveJPG(NCCAPixmap p, char *filename);
static void saveEXR(NCCAPixmap p, char *filename);
static void saveGIF(NCCAPixmap p, char *filename);

NCCAPixmap loadPixmap (char *filename)
{
	NCCAPixmap newPix;
	char *pExt;	/* extension */
	
	/* get file extension from file name */
	pExt = strrchr(filename, '.');
	if(pExt == NULL)
	{
		pExt="";
	}
	
	/* call correct load function */
	if (strcmp(pExt, ".pnm") == 0)	/* PNM */
	{
		newPix = loadPNM(filename);
	}
	else if (strcmp(pExt, ".pgm") == 0)	/* PGM */
	{
		newPix = loadPGM(filename);
	}
	else if (strcmp(pExt, ".ppm") == 0)	/* PPM */
	{
		newPix = loadPPM(filename);
	}
	else if (strcmp(pExt, ".exr") == 0
		||strcmp(pExt, ".EXR")==0)	/* EXR */
		{
		newPix = loadEXR(filename);
		}
	else if (strcmp(pExt, ".gif") == 0
		||strcmp(pExt, ".GIF")==0)	/* gif */
		{
		newPix = loadGIF(filename);
		}
	else if (strcmp(pExt, ".bmp") == 0
		||strcmp(pExt, ".BMP")==0)	/* BMP */
		{
		newPix = loadBMP(filename);
		}
	else if (strncmp(pExt, ".tif",4) == 0
		||strncmp(pExt, ".TIF",4) == 0
		||strcmp(pExt,".mip")==0)	/* TIFF */
	{
		newPix = loadTIFF(filename);
	}
	else if (strcmp(pExt, ".jpg") == 0
		||strcmp(pExt, ".jpeg") == 0
		||strcmp(pExt, ".JPG") == 0
		||strcmp(pExt, ".JPEG") == 0
		)	/* JPEG */
	{
		newPix = loadJPG(filename);
	}

	else 
	{
		newPix.data=0;
		newPix.width=0;
		newPix.height=0;
		newPix.bps=0;
		newPix.spp=0;
	}
	
	return newPix;
}



NCCAPixmap loadPNM(char *filename)
{ 	
	FILE *fd;
	unsigned char bufs[256];
	int pixSize;
	NCCAPixmap newPix;
	int i;

	fd=fopen(filename,STDIO_READ);
	if(fd==NULL)
	{
		newPix.data=0;
		newPix.width=0;
		newPix.height=0;
		newPix.bps=0;
		newPix.spp=0;

		return newPix;
	}

	fscanf(fd,"%s\n",bufs);
	assert(bufs[0]=='P');
	assert(bufs[1]=='6');
	assert(bufs[2]==0);

	/*RPPM*/
	fscanf(fd,"%d\n",&newPix.width);
	fscanf(fd,"%d\n",&newPix.height);
	fscanf(fd,"%d\n",&pixSize);

	/*printf("PNM%d,%d\n",newPix.width,newPix.height);*/

	newPix=newPixmap(newPix.width,newPix.height,NCCA_PIXMAP_RGB,8);
	fread(newPix.data,newPix.width*newPix.height,3,fd);

	for(i=0;i<newPix.width*newPix.height*3;i++)
		{
		float pVal;
		pVal = (newPix.data[i]*  256.0)/(float)pixSize;
		if(pVal >255)
			pVal=255;
		newPix.data[i] =pVal;
		}
	
	return newPix;

}

NCCAPixmap loadPGM(char *filename)
{ 	
	FILE *fd;
	unsigned char bufs[256];
	int pixSize;
	NCCAPixmap newPix;
	int i;

	fd=fopen(filename,STDIO_READ);
	if(fd==NULL)
	{
		newPix.data=0;
		newPix.width=0;
		newPix.height=0;
		newPix.bps=0;
		newPix.spp=0;

		return newPix;
	}

	fscanf(fd,"%s\n",bufs);
	assert(bufs[0]=='P');
	assert(bufs[1]=='5');
	assert(bufs[2]==0);

	/*PGM or RPGM*/
	fscanf(fd,"%d\n",&newPix.width);
	fscanf(fd,"%d\n",&newPix.height);
	fscanf(fd,"%d\n",&pixSize);

	/*printf("PGM:%d,%d\n",newPix.width,newPix.height);*/

	newPix=newPixmap(newPix.width,newPix.height,NCCA_PIXMAP_GRAY,8);
	assert(newPix.data);

	fread(newPix.data,newPix.width*newPix.height,1,fd);

	for(i=0;i<newPix.width*newPix.height;i++)
		{
		float pVal;
		pVal = (newPix.data[i]*  256.0)/(float)pixSize;
		if(pVal >255)
			pVal=255;
		newPix.data[i] =pVal;
		}
	
	return newPix;

}

NCCAPixmap loadPPM(char *filename)
{ 	
	FILE *fd;
	unsigned char bufs[256];
	int pixSize;
	NCCAPixmap newPix;
	int i;

	fd=fopen(filename,STDIO_READ);
	if(fd==NULL)
	{
		newPix.data=0;
		newPix.width=0;
		newPix.height=0;
		newPix.bps=0;
		newPix.spp=0;
		return newPix;
	}

	fscanf(fd,"%s\n",bufs);
	assert(bufs[0]=='P');
	assert(bufs[1]=='6');
	assert(bufs[2]==0);

	/*/ RPPM*/
	fscanf(fd,"%d\n",&newPix.width);
	fscanf(fd,"%d\n",&newPix.height);
	fscanf(fd,"%d\n",&pixSize);

	/*printf("PPM%d,%d\n",newPix.width,newPix.height);*/
	

	newPix=newPixmap(newPix.width,newPix.height,NCCA_PIXMAP_RGB,8);
	fread(newPix.data,newPix.width*newPix.height,3,fd);

	for(i=0;i<newPix.width*newPix.height*3;i++)
		{
		float pVal;
		pVal = (newPix.data[i]*  256.0)/(float)pixSize;
		if(pVal >255)
			pVal=255;
		newPix.data[i] =pVal;
		}
	
	return newPix;
}
NCCAPixmap loadTIFF(char *filename)
{
        NCCAPixmap newPix;
#ifdef TIFFSUPPORT
	TIFF *in;
	uint32 w, h;
	uint16 samplesperpixel;
	uint16 bitspersample;

        uint16 config;
	int bytesPerLine;
	int row;

        in = TIFFOpen(filename, "r");
        if(in==NULL)
        {
                newPix.data=0;
                newPix.width=0;
                newPix.height=0;
                newPix.bps=0;
                newPix.spp=0;

                return newPix;
        }


        if(TIFFGetField(in, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel)!=1)
		{
		fprintf(stderr,"SPP Missing\n");
		samplesperpixel=1;
		}
	newPix.spp=samplesperpixel;
        TIFFGetField(in, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	newPix.bps=bitspersample;
        TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w);
	newPix.width=w;
        TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h);
	newPix.height=h;
        TIFFGetField(in, TIFFTAG_PLANARCONFIG, &config);
	if(newPix.spp>1 && config != PLANARCONFIG_CONTIG)
	{
		(void) TIFFClose(in);
		newPix.data=0;
		newPix.width=0;
		newPix.height=0;
		newPix.bps=0;
		newPix.spp=0;

		return newPix;
	}

	bytesPerLine=(newPix.width * newPix.bps * newPix.spp)/8;
	newPix.data = (unsigned char *)malloc(bytesPerLine*newPix.height);
	if( newPix.data==NULL)
		{
		return newPix;
		}

	if(TIFFIsTiled(in))
		{
		int bufSize=TIFFTileSize(in);
		unsigned char *buf=malloc(bufSize);
		int tw,tl;
		int row,col;
		int bytesPerTileLine;
		int bytesPerPixel;

		(void) TIFFGetField(in, TIFFTAG_TILEWIDTH, &tw);
		(void) TIFFGetField(in, TIFFTAG_TILELENGTH, &tl);

		//For now we're assuming that tiles fit perfectly
		//This may not always be true?
		assert(newPix.width%tw==0);
		assert(newPix.height%tl==0);

		bytesPerPixel=(newPix.bps * newPix.spp)/8;
		bytesPerTileLine=tw *bytesPerPixel;

		for(row=0;row<newPix.height;row+=tl)
			for(col=0;col<newPix.width;col+=tw)
			{
			int tr,tc;
			TIFFReadTile(in,buf,col,row,0,0);	
			for(tr=0;tr<tl;tr++)
				{
				unsigned char *destLine=newPix.data+(row+tr)*bytesPerLine+col*bytesPerPixel;
				unsigned char *srcLine=buf+tr*bytesPerTileLine;
				for(tc=0;tc<bytesPerTileLine;tc++)
					{
					*(destLine+tc)=*(srcLine+tc);
					}
				}
			}
		free(buf);
		}
	else
		{

		for (row = 0; row < newPix.height; row++)
			TIFFReadScanline(in, newPix.data+row*bytesPerLine, row, 0);
		}

        (void) TIFFClose(in);
	return newPix;
#else
	newPix.data=0;
	newPix.width=0;
	newPix.height=0;
	newPix.bps=0;
	newPix.spp=0;

	return newPix;
#endif
}


void savePixmap (NCCAPixmap p, char *filename)
{
char *pExt;
pExt = strrchr(filename, '.');
if(pExt == NULL)
	{
		pExt="";
	}

/* call correct save function */
if (strcmp(pExt, ".pic") ==0)
{
	 savePIC(p,filename);
}
else if (strcmp(pExt, ".exr") == 0
	||strcmp(pExt, "EXR")==0)	/* EXR */
	{
	saveEXR(p,filename);
	}
else if (strcmp(pExt, ".gif") == 0
	||strcmp(pExt, "GIF")==0)	/* gif */
	{
	saveGIF(p,filename);
	}
else if (strcmp(pExt, ".tiff") ==0 || strcmp(pExt, ".tif")==0
	|| strcmp(pExt, ".TIFF") ==0 || strcmp(pExt, ".TIF")==0)
{
	 saveTIFF(p,filename);
}
else if (strcmp(pExt, ".jpg") == 0
	||strcmp(pExt, ".jpeg") == 0
	||strcmp(pExt, ".JPG") == 0
	||strcmp(pExt, ".JPEG") == 0
	)	/* JPEG */
{
	saveJPG(p,filename);
}
else
{
	 savePGM(p, filename);
}
}


void savePIC(NCCAPixmap p, char *filename)
{
unsigned char lengths[16];
FILE *file;
int i;

	file=fopen(filename, STDIO_WRITE);
    if (!file)
		{
		fprintf(stderr,"File open error:%s\n", filename);
		return;
		}
			
	for (i=0; i<4; i++) {
        lengths[i]=p.width>>8*(3-i);
    }
    for (i=0; i<4; i++) {
        lengths[i+4]=p.height>>8*(3-i);
    }

	fwrite(	lengths,1, 8, file);
	fwrite(	p.data,1,(p.height * p.width * p.bps * p.spp)/8, file);
	
	
    if (fclose(file))
		{
		printf("File close error...\n");
		}
}


void savePGM(NCCAPixmap p, char *filename)
{
	FILE *fd;

	fd=fopen(filename, STDIO_WRITE);
	if(fd==NULL)
		{
		fprintf(stderr,"Can't save to file (%s)\n",filename);
		return;
		}

	fprintf(fd,"P6\n");
	fprintf(fd,"%d\n", p.width);
	fprintf(fd,"%d\n", p.height);
	fprintf(fd,"%d\n", 255);
	
	
	fwrite(	p.data,(p.height * p.width * p.bps * p.spp)/8, 1, fd);

	fclose(fd);
}

void saveTIFF(NCCAPixmap p, char *filename)
{
#ifdef TIFFSUPPORT
        uint32 row;
        tsize_t linebytes;

	TIFF *out = TIFFOpen(filename, "w");
        if (out == NULL)
                return;
        TIFFSetField(out, TIFFTAG_IMAGEWIDTH,  p.width);
        TIFFSetField(out, TIFFTAG_IMAGELENGTH, p.height);
        TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, p.spp);
        TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, p.bps);
        TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	if(p.spp<3)
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	else
                TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

        TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
        //TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
        linebytes = (p.spp * p.width *p.bps) /8;
        TIFFSetField(out, TIFFTAG_ROWSPERSTRIP,
            TIFFDefaultStripSize(out, -1));
        for (row = 0; row < p.height; row++) {
                if (TIFFWriteScanline(out, p.data+row*linebytes, row, 0) < 0)
			{
			fprintf(stderr,"failed a scanline write (%d)\n", (int)row);
                        break;
			}
        }
        (void) TIFFClose(out);
#endif
}

#ifdef EXRSUPPORT
#include <ImfCRgbaFile.h>
#endif

static NCCAPixmap loadEXR(char * filename)
{
NCCAPixmap newPix;
#ifdef EXRSUPPORT
	const ImfHeader *h;
	NCCAPixmap halfPixmap;
	int xmin,xmax,ymin,ymax;
	ImfInputFile *f=ImfOpenInputFile(filename);
	if(f==NULL)
		{
		newPix.data=0;
		newPix.width=0;
		newPix.height=0;
		newPix.bps=0;
		newPix.spp=0;

		return newPix;
		}

	h=ImfInputHeader(f);
#if 0
	//Sometimes images have broken display windows!
	ImfHeaderDataWindow(h,&xmin,&ymin,&xmax,&ymax);
	printf("%d,%d,%d,%d\n",xmin,ymin,xmax,ymax);
#endif
	ImfHeaderDisplayWindow(h,&xmin,&ymin,&xmax,&ymax);
	newPix    =newPixmap(xmax-xmin+1, ymax-ymin+1, 4, 32);
	//printf("%d,%d\n",newPix.width, newPix.height);

	halfPixmap=newPixmap(newPix.width, newPix.height, 4, 16);

	ImfInputSetFrameBuffer(f,
				(ImfRgba *)halfPixmap.data - xmin - ymin * newPix.width,
				1, newPix.width);
	ImfInputReadPixels (f,ymin,ymax);
	ImfCloseInputFile(f);
	ImfHalfToFloatArray(newPix.width*newPix.height*4,
			(const ImfHalf *)halfPixmap.data,
			(float *)newPix.data);
	destroyPixmap(halfPixmap);
#else
	fprintf(stderr,"Load EXR Not Supported\n");
	newPix.data=0;
	newPix.width=0;
	newPix.height=0;
	newPix.bps=0;
	newPix.spp=0;
#endif
	return newPix;
}

static void saveEXR(NCCAPixmap p,char * filename)
{
#ifdef EXRSUPPORT
	NCCAPixmap halfPixmap;
	ImfOutputFile *f;
	ImfHeader *h= ImfNewHeader();

	if(p.bps!=32 || p.spp !=4)
		{
		NCCAPixmap q;
		int i,j;
		q=newPixmap(p.width, p.height, 4,32);
		for(j=0;j<p.height;j++)
			for(i=0;i<p.width;i++)
				{
				NCCAPixel c;
				c=getPixelColor(p,i,j);
				setPixelColor(q,i,j,c);
				}
		saveEXR(q,filename);
		destroyPixmap(q);
		return;
		}

	halfPixmap=newPixmap(p.width, p.height, 4, 16);
	ImfFloatToHalfArray (p.width*p.height*4,
                                             (float *)p.data,
                                             (ImfHalf *)halfPixmap.data);
	
	ImfHeaderSetDisplayWindow(h,0,0,p.width-1,p.height-1);
	ImfHeaderSetDataWindow(h,0,0,p.width-1,p.height-1);
	f=ImfOpenOutputFile(filename,h,IMF_WRITE_RGBA);
	ImfOutputSetFrameBuffer (f,(ImfRgba *)halfPixmap.data,1,halfPixmap.width);
	ImfOutputWritePixels(f,halfPixmap.height);
	ImfCloseOutputFile(f);
	ImfDeleteHeader(h);
	destroyPixmap(halfPixmap);
#else
	fprintf(stderr,"Save EXR Not Supported\n");
#endif
}


NCCAPixmap loadGIF(char *filename)
{
NCCAPixmap p;
p.data=NULL;
return p;
}

void saveGIF (NCCAPixmap p, char * filename)
{}

#ifdef JPEGSUPPORT
#include "jpeglib.h"
#endif


NCCAPixmap loadJPG(char * filename)
{
  NCCAPixmap newPix;
#ifdef JPEGSUPPORT
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
static struct jpeg_error_mgr jpgem;

  /* More stuff */
  FILE * infile;		/* source file */
  JSAMPROW buffer[1];		/* Output row buffer */

  /* In this example we want to open the input file before doing anything else,
   * so that the setjmp() error recovery below can assume the file is open.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to read binary files.
   */

  if ((infile = fopen(filename, "rb")) == NULL)
	{
	newPix.data=0;
	newPix.width=0;
	newPix.height=0;
	newPix.bps=0;
	newPix.spp=0;

	return newPix;
	}

  /* Step 1: allocate and initialize JPEG decompression object */
  cinfo.err = jpeg_std_error(&jpgem);

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */

  newPix=newPixmap(cinfo.output_width,
		cinfo.output_height,
		cinfo.output_components,
		8);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    buffer[0]=newPix.data+cinfo.output_scanline*(newPix.spp*newPix.width);
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
  }

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  fclose(infile);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
#else
  newPix.data=NULL;
#endif
  return newPix;
}

static int JPGCompression=100;
void setJPGCompression(int percent)
{
	JPGCompression=percent;
}
void saveJPG (NCCAPixmap p, char * filename)
{
#ifdef JPEGSUPPORT
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  assert(p.bps==8);
  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = p.width; 	/* image width and height, in pixels */
  cinfo.image_height = p.height;
  cinfo.input_components = p.spp;	/* # of color components per pixel */
  if(p.spp==1 || p.spp==2)
	  cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
  else
	  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, JPGCompression, TRUE);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = p.width * p.spp;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & p.data[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
#endif
}


static void ReadULShort(FILE *fptr,unsigned short *p)
	{
	char tmp;
	*p=0;
	fread(&tmp,1,1,fptr);
	*p|=((unsigned short)tmp)<<0;
	fread(&tmp,1,1,fptr);
	*p|=((unsigned short)tmp)<<8;
	return;

#ifdef BIG_ENDIAN
	fread(((char *)p+1),1,1,fptr);
	fread(((char *)p+0),1,1,fptr);
#else
	fread((char *)p,2,1,fptr);
#endif
	}

static void ReadULInt(FILE *fptr,unsigned int *p)
	{
        char tmp;
        *p=0;
        fread(&tmp,1,1,fptr);
        *p|=((unsigned int)tmp)<<0;
        fread(&tmp,1,1,fptr);
        *p|=((unsigned int)tmp)<<8;
        fread(&tmp,1,1,fptr);
        *p|=((unsigned int)tmp)<<16;
        fread(&tmp,1,1,fptr);
        *p|=((unsigned int)tmp)<<24;
        return;

#ifdef BIG_ENDIAN
	fread(((char *)p+3),1,1,fptr);
	fread(((char *)p+2),1,1,fptr);
	fread(((char *)p+1),1,1,fptr);
	fread(((char *)p+0),1,1,fptr);
#else
	fread((char *)p,4,1,fptr);
#endif
	}
NCCAPixmap loadBMP(char * filename)
{
NCCAPixmap newPix;
unsigned char *p;

typedef struct {
   unsigned short int type;                 /* Magic identifier            */
   unsigned int size;                       /* File size in bytes          */
   unsigned short int reserved1, reserved2;
   unsigned int offset;                     /* Offset to image data, bytes */
} HEADER;
typedef struct {
   unsigned int size;               /* Header size in bytes      */
   unsigned int width,height;                /* Width and height of image */
   unsigned short int planes;       /* Number of colour planes   */
   unsigned short int bits;         /* Bits per pixel            */
   unsigned int compression;        /* Compression type          */
   unsigned int imagesize;          /* Image size in bytes       */
   unsigned int xresolution,yresolution;     /* Pixels per meter          */
   unsigned int ncolours;           /* Number of colours         */
   unsigned int importantcolours;   /* Important colours         */
} INFOHEADER;
typedef struct {
   unsigned char r,g,b,junk;
} COLOURINDEX;

   int i,j;
   int gotindex = FALSE;
   unsigned char grey,r,g,b;
   HEADER header;
   INFOHEADER infoheader;
   COLOURINDEX colourindex[256];
   FILE *fptr;
   int remainder;

    newPix.data=0;
    newPix.width=0;
    newPix.height=0;
    newPix.bps=0;
    newPix.spp=0;

   /* Open file */
   if ((fptr = fopen(filename,"rb")) == NULL) {
	fprintf(stderr,"Unable to open BMP file \"%s\"\n",filename);
	return newPix;
	}

   /* Read and check the header */
   ReadULShort(fptr,&header.type);
   if(header.type!='M'*256+'B')
	{
	fprintf(stderr,"Unable to load BMP file \"%s\"\n",filename);
	fprintf(stderr,"ID is: %x, should be %x\n",header.type,'M'*256+'B');
	fclose(fptr);
	return newPix;
	}
   ReadULInt(fptr,&header.size);
   //fprintf(stderr,"File size is %d bytes\n",header.size);
   ReadULShort(fptr,&header.reserved1);
   ReadULShort(fptr,&header.reserved2);
   ReadULInt(fptr,&header.offset);
   //fprintf(stderr,"Offset to image data is %d bytes\n",header.offset);

   /* Read and check the information header */
   ReadULInt(fptr,&infoheader.size);
   ReadULInt(fptr,&infoheader.width);
   ReadULInt(fptr,&infoheader.height);
   //fprintf(stderr,"Image size = %d x %d\n",infoheader.width,infoheader.height);
   ReadULShort(fptr,&infoheader.planes);
   //fprintf(stderr,"Number of colour planes is %d\n",infoheader.planes);
   ReadULShort(fptr,&infoheader.bits);
   //fprintf(stderr,"Bits per pixel is %d\n",infoheader.bits);
   ReadULInt(fptr,&infoheader.compression);
   //fprintf(stderr,"Compression type is %d\n",infoheader.compression);
   ReadULInt(fptr,&infoheader.imagesize);
   ReadULInt(fptr,&infoheader.xresolution);
   ReadULInt(fptr,&infoheader.yresolution);
   ReadULInt(fptr,&infoheader.ncolours);
   //fprintf(stderr,"Number of colours is %d\n",infoheader.ncolours);
   ReadULInt(fptr,&infoheader.importantcolours);
   //fprintf(stderr,"Number of required colours is %d\n",infoheader.importantcolours);

   /* Read the lookup table if there is one */
   for (i=0;i<255;i++) {
      colourindex[i].r = rand() % 256;
      colourindex[i].g = rand() % 256;
      colourindex[i].b = rand() % 256;
      colourindex[i].junk = rand() % 256;
   }
   if (infoheader.ncolours > 0) {
      for (i=0;i<infoheader.ncolours;i++) {
         if (fread(&colourindex[i].b,sizeof(unsigned char),1,fptr) != 1) {
            fprintf(stderr,"Image read failed\n");
	    fclose(fptr);
            return newPix;
         }
         if (fread(&colourindex[i].g,sizeof(unsigned char),1,fptr) != 1) {
            fprintf(stderr,"Image read failed\n");
	    fclose(fptr);
            return newPix;
         }
         if (fread(&colourindex[i].r,sizeof(unsigned char),1,fptr) != 1) {
            fprintf(stderr,"Image read failed\n");
	    fclose(fptr);
            return newPix;
         }
         if (fread(&colourindex[i].junk,sizeof(unsigned char),1,fptr) != 1) {
            fprintf(stderr,"Image read failed\n");
	    fclose(fptr);
            return newPix;
         }
         //fprintf(stderr,"%3d\t%3d\t%3d\t%3d\n",i,colourindex[i].r,colourindex[i].g,colourindex[i].b);
      }
      gotindex = TRUE;
   }

   /* Seek to the start of the image data */
   fseek(fptr,header.offset,SEEK_SET);

   newPix=newPixmap(infoheader.width,infoheader.height,
			(infoheader.bits==24||gotindex)?NCCA_PIXMAP_RGB:NCCA_PIXMAP_GRAY,8);
   assert(newPix.data);

   //Pad lines to 4 byte boundary
   remainder=4-(infoheader.width*(infoheader.bits/8))%4;
   if(remainder==4)
	remainder=0;

   /* Read the image */
   for (j=0;j<infoheader.height;j++) {
      p=newPix.data+(infoheader.height-1-j)*infoheader.width*newPix.spp;
      for (i=0;i<infoheader.width;i++) {

         switch (infoheader.bits) {
         case 1:
            break;
         case 4:
            break;
         case 8:
            if (fread(&grey,sizeof(unsigned char),1,fptr) != 1) {
               fprintf(stderr,"Image read failed\n");
	       fclose(fptr);
               return newPix;
            }
            if (gotindex) {
               *p++=(colourindex[grey].r);
               *p++=(colourindex[grey].g);
               *p++=(colourindex[grey].b);
            } else {
               *p++=(grey);
            }
            break;
         case 24:
            if (fread(&b,sizeof(unsigned char),1,fptr) != 1) {
               fprintf(stderr,"Image read failed\n");
	       fclose(fptr);
               return newPix;
            }
            if (fread(&g,sizeof(unsigned char),1,fptr) != 1) {
               fprintf(stderr,"Image read failed\n");
	       fclose(fptr);
               return newPix;
            }
            if (fread(&r,sizeof(unsigned char),1,fptr) != 1) {
               fprintf(stderr,"Image read failed\n");
	       fclose(fptr);
               return newPix;
            }
            *p++=(r);
	    *p++=(g);
            *p++=(b);
            break;
         }

      } /* i */
      fseek(fptr,remainder,SEEK_CUR);
	
   } /* j */

   fclose(fptr);
   return newPix;
}
