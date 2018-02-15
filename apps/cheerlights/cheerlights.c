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
#include <signal.h>
#include <math.h>
#include <string.h>

#include <unistd.h>

#include <assert.h>
#include <NCCAPixmap.h>
#include <PixDrawing.h>
#include <PixBlit.h>
#include <PixFileIO.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


#ifdef LARGE
#define HEIGHT 360
#define WIDTH 640
#else
#define HEIGHT 180
#define WIDTH 320
#endif

#define MAX_BUF 4096
static int XXcheerlight_openConnection()
{
    int sd;
    
    int portno=80;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    /* Create a socket point */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }
    server = gethostbyname("api.thingspeak.com");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    memset((char *) &serv_addr,0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(
           (char *)&serv_addr.sin_addr.s_addr,
           (char *)server->h_addr,
           server->h_length);
    serv_addr.sin_port = htons(portno);
    /* Now connect to the server */
    if (connect(sd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(1);
    }
    
    return sd;
}

static void XXcheerlight_doGet(char *url,char *data)
{
    int sd=XXcheerlight_openConnection();
    
    char send_buf[MAX_BUF];
    memset(send_buf, 0, MAX_BUF);
    
    sprintf(send_buf, "GET %s HTTP/1.1\r\n\r\n", url);
    send(sd, send_buf, strlen(send_buf),0);
    
    char *ptr=data;
    int count;
    while((count=recv(sd, ptr, MAX_BUF,0))>0)
        ptr+=count;
    *ptr=0;
#ifdef WIN32
    closesocket(sd);
#else
    close(sd);
#endif
}


static const char *isStringLiteral(const char*data, const char *key)
{
    const char*kptr = key;
    const char*dptr = data;
    if(*dptr=='"')
        dptr++;
    else
    {
        return 0;
    }
    
    while (*dptr&&*kptr)
    {
        if (*dptr++!=*kptr++)
            return 0;
    }
    
    if(*dptr=='"')
    {
        return dptr;
    }
    return 0;
}

static const char *getArrayEntry(const char *array, int index)
{
    const char *ptr=array;
    int depth=0;
    int found=0;
    do
    {
        if(*ptr==']' || *ptr=='}')
        {
            depth--;
        }
        if(depth==1)
        {
            if(*ptr==',')
            {
                found++;
                ptr++;
            }
            if(found==index)
            {
                return ptr;
            }
        }
        if(*ptr=='[' || *ptr=='{')
        {
            depth++;
        }
        ptr++;
    }
    while(depth>0);
    return NULL;
}

static const char *getDictionaryEntry(const char *dictionary,const char *key)
{
    const char *ptr=dictionary;
    int depth=0;
    do
    {
        if(*ptr==0)
            return NULL;
        
        if(*ptr=='{')
            depth++;
        if(*ptr=='}')
            depth--;
        if(depth==1)
        {
            if(*ptr=='"')
            {
                if(isStringLiteral(ptr,key))
                {
                    ptr+=strlen(key)+2;
                    while(*ptr!=':')
                        ptr++;
                    ptr++;
                    return ptr;
                }
                else
                {
                    do
                    {
                        ptr++;
                    }while(*ptr!='"');
                }
            }
        }
        ptr++;
    }
    while(depth>0);
    return NULL;
}



int main(int argc, char*argv[])
{
	NCCAPixmap img;
    char tmpFilename[1024];
    char realFilename[1024];
    int pid=getpid();
    sprintf(tmpFilename,"/tmp/LIVE/X%d.jpg",pid);
    sprintf(realFilename,"/tmp/LIVE/%d.jpg",pid);

	img=newPixmap(WIDTH,HEIGHT,3,8);
	if(img.data==NULL)
		{
		exit(-1);
		}


    while(1)
    {
        NCCAPixel c;

	    char data[MAX_BUF];
    XXcheerlight_doGet("/channels/1417/feeds.json?results=1\n",data);
    char *startPtr;
    startPtr=data;
    while(*startPtr && *startPtr!='{')
        startPtr++;
    if(*startPtr==0)
        continue;
    

    const char *feedPtr=getDictionaryEntry(startPtr,"feeds");

    if(feedPtr)
    {
        const char *recordPtr=getArrayEntry(feedPtr,0);
        if(recordPtr)
        {
        const char *colPtr=getDictionaryEntry(recordPtr,"field2");
        if(colPtr)
            {
                if(*colPtr!='\"')
                    continue;
                colPtr++;
               if(*colPtr!='#')
                    continue;
                colPtr++;
                
                uint32_t cVal;
                sscanf(colPtr,"%x",&cVal);

                float SN_blue=cVal&0xff;
                cVal>>=8;
                float SN_green=cVal&0xff;
                cVal>>=8;
                float SN_red=cVal&0xff;

		c.r=SN_red/255.0;
		c.g=SN_green/255.0;
		c.b=SN_blue/255.0;
                   
            }
        }
    }

        c.a=1;

	int x,y;
	for(x=0;x<WIDTH;x++)
		for(y=0;y<HEIGHT;y++)
			{
			setPixelColor(img,x,y,c);
			}

        c.r=0;
        c.g=0;
        c.b=0;
	drawLine(img,0,0,WIDTH-1,0,c);
	drawLine(img,WIDTH-1,0,WIDTH-1,HEIGHT-1,c);
	drawLine(img,WIDTH-1,HEIGHT-1,0,HEIGHT-1,c);
	drawLine(img,0,HEIGHT-1,0,0,c);

        savePixmap(img,tmpFilename);

        //Needed to so that the write is atomic!
        rename(tmpFilename,realFilename);
        sleep(5);
    }
	return 0;
}
