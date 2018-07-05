/***************************************\
 * $Log: viewer.c,v $
 * Revision 1.2  2000/11/26 18:48:50  ian
 * add wait parameter to Catch_X_Events
 *
 * Revision 1.1  2000/09/12 18:04:30  ian
 * Initial revision
 *
 * 
\***************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
#endif


#define CLEAR(x) memset(&(x), 0, sizeof(x))

static int xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}
static void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}



#include <assert.h>
#include <NCCAPixmap.h>
#include <PixDrawing.h>
#include <PixBlit.h>
#include <PixFileIO.h>

#ifdef LARGE
#define HEIGHT 180*2
#define WIDTH 320*2
#else
#define HEIGHT 180
#define WIDTH 320
#endif

#define RGB24 0
#define YUV422 1
#define CPIA 2
struct Xwebcam_struct
{
    int width;
    int height;
    int format; 
#ifdef __linux
    int fd;
    int index;
    struct buffer {
		void   *start;
		size_t  length;
	}buffers[2];
#else
#error Video input only supported on Linux
#endif

};


void Xwebcam_init(struct Xwebcam_struct *instance)
{
     char *dev_name="/dev/video0";
     instance->fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

     if (-1 == instance->fd) {
              fprintf(stderr, "Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
	}


	struct v4l2_capability cap;
    if (-1 == xioctl(instance->fd, VIDIOC_QUERYCAP, &cap))
	{
	if (EINVAL == errno) {
		fprintf(stderr, "%s is no V4L2 device\n",
			 dev_name);
		exit(EXIT_FAILURE);
		}
		else
		{
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf(stderr, "%s is no video capture device\n",
                         dev_name);
                exit(EXIT_FAILURE);
        }


       struct v4l2_cropcap cropcap;
        CLEAR(cropcap);
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (0 == xioctl(instance->fd, VIDIOC_CROPCAP, &cropcap))
	{
		struct v4l2_crop crop;
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                xioctl(instance->fd, VIDIOC_S_CROP, &crop);
        }

	struct v4l2_format fmt;
        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = WIDTH;  
        fmt.fmt.pix.height      = HEIGHT;  
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        fmt.fmt.pix.field       = 0;//V4L2_FIELD_INTERLACED;

        if (-1 == xioctl(instance->fd, VIDIOC_S_FMT, &fmt))
                errno_exit("VIDIOC_S_FMT");


#if 0
	printf("%d x %d:%c%c%c%c\n", fmt.fmt.pix.width,fmt.fmt.pix.height,
		(fmt.fmt.pix.pixelformat>>0)&0xff,
		(fmt.fmt.pix.pixelformat>>8)&0xff,
		(fmt.fmt.pix.pixelformat>>16)&0xff,
		(fmt.fmt.pix.pixelformat>>24)&0xff
		);
#endif

	switch(fmt.fmt.pix.pixelformat)
	{
	case V4L2_PIX_FMT_RGB24:
		instance->format=RGB24;
		break;
	case V4L2_PIX_FMT_YUYV:
		instance->format=YUV422;
		break;
	case V4L2_PIX_FMT_CPIA1:
		instance->format=CPIA;
		break;
	default:
		fprintf(stderr,"Unknown Video Format:%c%c%c%c\n",
		                (fmt.fmt.pix.pixelformat>>0)&0xff,
				(fmt.fmt.pix.pixelformat>>8)&0xff,
				(fmt.fmt.pix.pixelformat>>16)&0xff,
				(fmt.fmt.pix.pixelformat>>24)&0xff
			);

		exit(-1);
	}

   
    //Allocate a buffer
        struct v4l2_requestbuffers req;

        CLEAR(req);
        req.count = 2;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(instance->fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "memory mapping\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf(stderr, "Insufficient buffer memory on %s\n",
                         dev_name);
                exit(EXIT_FAILURE);
        }

	int count;
	for(count=0;count<2;count++)
		{
                struct v4l2_buffer buf;
                CLEAR(buf);
                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = count;

                if (-1 == xioctl(instance->fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                instance->buffers[count].length = buf.length;
                instance->buffers[count].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              instance->fd, buf.m.offset);

                if (MAP_FAILED == instance->buffers[0].start)
                        errno_exit("mmap");

		if (-1 == xioctl(instance->fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
		}

        enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(instance->fd, VIDIOC_STREAMON, &type))
		errno_exit("VIDIOC_STREAMON");

    instance->width=fmt.fmt.pix.width;
    instance->height=fmt.fmt.pix.height;
    instance->index=-1;
}

void Xwebcam_update(struct Xwebcam_struct *instance)
{

        struct v4l2_buffer buf;
	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	//Put old buffer (if any) back in queue
	if(instance->index>=0)
	{
	buf.index=instance->index;
        if (-1 == xioctl(instance->fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
	instance->index=-1;
	}



	if (-1 == xioctl(instance->fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("VIDIOC_DQBUF");
                        }
                }
	instance->index=buf.index;
}

void YUV2RGB(uint8_t y,uint8_t u,uint8_t v,uint8_t *dest)
{
   int r,g,b;
   
   // u and v are +-0.5
   int uu =((int)u)-128;
   int vv =((int)v)-128;

   // Conversion
   r = y + 1.370705 * vv;
   g = y - 0.698001 * vv - 0.337633 * uu;
   b = y + 1.732446 * uu;

/*
   r = y + 1.402 * vv;
   g = y - 0.344 * u - 0.714 * vv;
   b = y + 1.772 * u;
*/
/*
   y -= 16;
   r = 1.164 * y + 1.596 * vv;
   g = 1.164 * y - 0.392 * u - 0.813 * vv;
   b = 1.164 * y + 2.017 * u;
*/

   // Clamp to 0..1
   if (r < 0) r = 0;
   if (g < 0) g = 0;
   if (b < 0) b = 0;
   if (r > 255) r = 255;
   if (g > 255) g = 255;
   if (b > 255) b = 255;

   dest[0]= r;
   dest[1]= g;
   dest[2]= b;
}
void rgbPixmapFromYuv422(NCCAPixmap destP,uint8_t *yuvSrc)
	{
	uint8_t *src=yuvSrc;
	uint8_t *dest=destP.data;
	int i;
	int imgSize=destP.width*destP.height;
	for(i=0;i<imgSize;i+=2)
		{
		YUV2RGB(src[0],src[1],src[3],dest);
		dest+=3;
		YUV2RGB(src[2],src[1],src[3],dest);
		dest+=3;
		src+=4;
		}
	}

uint8_t *yuv422Buf=NULL;
void rgbPixmapFromYuv420(NCCAPixmap img,uint8_t *camData)
{
#define FRAME_HEADER_SIZE 64
#define MAGIC_0         0x19    /**< First header byte */
#define MAGIC_1         0x68    /**< Second header byte */
#define SUBSAMPLE_420      0
#define SUBSAMPLE_422      1
#define YUVORDER_YUYV      0
#define YUVORDER_UYVY      1
#define NOT_COMPRESSED     0
#define COMPRESSED         1
#define NO_DECIMATION      0
#define DECIMATION_ENAB    1
#define EOL             0xfd    /**< End Of Line marker */
#define EOI             0xff    /**< End Of Image marker */

	uint8_t *header=camData;
	assert(header[0]==MAGIC_0);
	assert(header[1]==MAGIC_1);
	assert(header[17]==SUBSAMPLE_420);
	assert(header[18]==YUVORDER_YUYV);
	assert(header[28]==NOT_COMPRESSED ||header[28]==COMPRESSED);
	assert(header[29]==NO_DECIMATION);
	int compressed;
	if(header[28]==COMPRESSED)
		compressed=1;
	else
		compressed=0;
	int maxW=img.width;
	int maxH=img.height;

	//printf("%dx%d\n",maxW,maxH);

	int x,y;
	uint8_t *srcLine=camData+FRAME_HEADER_SIZE;
	for(y=0;y<maxH;y+=1)
		{
		int lineLength =srcLine[0]+(((int)srcLine[1])<<8);
		//printf("Y:%d\n",y);
		srcLine+=2;
		int srcIndex=0;

		int destIndex=0;
		uint8_t *thisLineDest=yuv422Buf+2*img.width*y;
		uint8_t *nextLineDest=yuv422Buf+2*img.width*(y+1);

		if((y&1)==0)
			{
			//EVEN LINES WE HAVE FULL YUV
			for(x=0;x<maxW;)
				{
				if(compressed && (srcLine[srcIndex]&1))
					{
					//Compressed
				        int skip=srcLine[srcIndex]&0xfe;
					skip>>=1;
					//printf("Even Skip:%d\n",skip);
					srcIndex++;
					destIndex+=skip<<1;
					x+=skip;
					if(x>maxW)
						{
						srcIndex=lineLength-1;
						break;
						}
					}
				else
					{
					thisLineDest[destIndex+0]=srcLine[srcIndex+0];
					thisLineDest[destIndex+1]=srcLine[srcIndex+1];
					thisLineDest[destIndex+2]=srcLine[srcIndex+2];
					thisLineDest[destIndex+3]=srcLine[srcIndex+3];

					nextLineDest[destIndex+1]=srcLine[srcIndex+1];
					nextLineDest[destIndex+3]=srcLine[srcIndex+3];
					srcIndex+=4;
					destIndex+=4;
					x+=2;
					}
				}
			}
		else
			{
			//ODD LINES WE HAVE ONLY Y's
			for(x=0;x<maxW;)
				{       
				if(compressed && (srcLine[srcIndex]&1))
                                        {
					//Compressed
                                        int skip=srcLine[srcIndex]&0xfe;
					skip>>=1;
					//printf("Odd Skip:%d\n",skip);
                                        srcIndex++;
                                        destIndex+=skip<<1;
                                        x+=skip;
					if(x>maxW)
						{
						srcIndex=lineLength-1;
						break;
						}
					}
				else
					{
					thisLineDest[destIndex+0]=srcLine[srcIndex];
					thisLineDest[destIndex+2]=srcLine[srcIndex+1];
					srcIndex+=2;
					destIndex+=4;
					x+=2;
					}
				}
			}
		if(srcIndex!=lineLength-1)
			break;

		if(srcLine[srcIndex]==EOI)
			break;

		if(srcLine[srcIndex]!=EOL)
			break;

		srcLine+=lineLength;
		}
	rgbPixmapFromYuv422(img,yuv422Buf);
}

int main(int argc, char*argv[])
{
	NCCAPixmap img;
    char tmpFilename[1024];
    char realFilename[1024];
    int pid=getpid();
    sprintf(tmpFilename,"/tmp/LIVE/X%d.jpg",pid);
    sprintf(realFilename,"/tmp/LIVE/%d.jpg",pid);

    struct Xwebcam_struct instance[1];
    Xwebcam_init(instance);
    img=newPixmap(instance->width,instance->height,3,8);

    if(instance->format==CPIA)
	yuv422Buf=malloc(instance->width*instance->height*2);//4 bytes=2 pixels!

    if(img.data==NULL)
		{
		exit(-1);
		}


    while(1)
    {

        //Needed to so that the write is atomic!
	Xwebcam_update(instance);
	if(instance->index>=0)
		{
		uint8_t *camData=instance->buffers[instance->index].start;
		switch(instance->format)
		{
		case RGB24:
			memcpy(img.data,camData,instance->buffers[instance->index].length);
			break;
		case YUV422:
			{
			rgbPixmapFromYuv422(img,camData);
			break;
			}
		case CPIA:
			{
			rgbPixmapFromYuv420(img,camData);
                        break;
                        }

		default:
			exit(1);	
		}
		savePixmap(img,tmpFilename);
		rename(tmpFilename,realFilename);
		usleep(100000);
		}
    }
	return 0;
}
