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
#define HEIGHT 360
#define WIDTH 640
#else
#define HEIGHT 180
#define WIDTH 320
#endif


struct Xwebcam_struct
{
    int width;
    int height;
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

    instance->width=WIDTH;
    instance->height=HEIGHT;
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
		memcpy(img.data,camData,instance->buffers[instance->index].length);
		savePixmap(img,tmpFilename);
		rename(tmpFilename,realFilename);
		usleep(100000);
		}
    }
	return 0;
}
