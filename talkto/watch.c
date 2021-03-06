#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <error.h>
#include <errno.h>

#include <sys/select.h>




#ifdef __linux__
#include <sys/inotify.h>

static int wd;
static int fd;

void watchInit()
{

fd = inotify_init ();
if (fd < 0)
        perror ("inotify_init");


wd = inotify_add_watch (fd,
                "/tmp/LIVE",
                IN_MOVED_TO|IN_DELETE);

if (wd < 0)
        perror ("inotify_add_watch");
}


void watchWait()
{
#define EVENT_SIZE  (sizeof (struct inotify_event))
#define BUF_LEN        (1024 * (EVENT_SIZE + 16))

char buf[BUF_LEN];

fd_set ReadSet;
FD_ZERO(&ReadSet);
FD_SET(fd, &ReadSet);

struct timeval timeout={2,0};
select(FD_SETSIZE, &ReadSet, NULL, NULL, &timeout);
if(FD_ISSET(fd, &ReadSet))
	{
	//int len =
	read (fd, buf, BUF_LEN);

//#define DIAGNOSTICS
	#ifdef DIAGNOSTICS
	puts("NOTIFY");
	if (len <= 0)
		{
		if (errno == EINTR)
			{
			/* need to reissue system call */
			}
		else
			perror ("read");
		}
	else
		{
		int i=0;
		while (i < len)
			{
			struct inotify_event *event;
			event = (struct inotify_event *) &buf[i];
			if(event->len)// && event->name[0]!='X')
				{
				printf ("%d:wd=%d mask=%u cookie=%u len=%u\n",
					i,
					event->wd, event->mask,
					event->cookie, event->len);

				if (event->len)
					printf ("name=%s\n", event->name);
				}
			i += EVENT_SIZE + event->len;
			}
		}
	#endif
	}
}
#else
void watchInit()
	{
	}
void watchWait()
	{
	usleep(100000);
	}
#endif

