/*
 * $Log: Win_X.c,v $
 * Revision 1.8  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
 * Revision 1.7  2007/06/28 14:12:00  ian
 * Improve namespace usage
 *
 * Revision 1.6  2004/03/16 13:47:19  ian
 * do gamma correction
 *
 * Revision 1.5  2002/07/30 09:03:34  ian
 * RCS seems to think EVERYTHING has changed, so check it
 * in again to make it happy...
 *
 * Revision 1.4  2001/04/29 16:16:33  ian
 * Add support for 8 bit visual
 * Add popup menu functions
 *
 * Revision 1.3  2000/12/28 19:17:35  ian
 * add wait parameter to catch events call
 *
 * Revision 1.2  2000/09/17 16:35:01  ian
 * add SetWindowTitle, so title can be changed after the window
 * has been created
 *
 * Revision 1.1  2000/09/10 16:09:23  ian
 * Initial revision
 *
 */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

static Display 		*display;
static GC 		blackgc=0;
static Window 		rootwin;
static Colormap		mycmap;
static XColor 		black,white;
static Visual 		*visual;
static int		screenDepth;

static int falseColor=0;
static int  colorLookup[4096];
static int red_bits;
static int green_bits;
static int blue_bits;
static int red_shift;
static int green_shift;
static int blue_shift;

static unsigned char gammaLut[256];
static float OOgamma;
static float displayGain;
	
static void fillLookupTable(void)
{
int r,g,b;
while(1)
	{
	int doAgain=0;
	for(r=0;r<16;r++)
		for(g=0;g<16;g++)
			for(b=0;b<16;b++)
				{
				int i;
				i=r<<8;
				i|=g<<4;
				i|=b<<0;
				if(colorLookup[i]!=-1)
					continue;
				if(b<15)
					{
					colorLookup[i]=colorLookup[i+1];
					if(colorLookup[i]!=-1)
						continue;
					}
				if(r<15)
					{
					colorLookup[i]=colorLookup[i+256];
					if(colorLookup[i]!=-1)
						continue;
					}
				if(g<15)
					{
					colorLookup[i]=colorLookup[i+16];
					if(colorLookup[i]!=-1)
						continue;
					}
				if(b<15 && r<15)
					{
					colorLookup[i]=colorLookup[i+1+256];
					if(colorLookup[i]!=-1)
						continue;
					}
				if(b<15 && g<15)
					{
					colorLookup[i]=colorLookup[i+1+16];
					if(colorLookup[i]!=-1)
						continue;
					}
				if(r<15 && g<15)
					{
					colorLookup[i]=colorLookup[i+16+256];
					if(colorLookup[i]!=-1)
						continue;
					}
				if(r<15 && g<15 && b<15)
					{
					colorLookup[i]=colorLookup[i+1+16+256];
					if(colorLookup[i]!=-1)
						continue;
					}
				doAgain=1;
				}
	if(!doAgain)
		return;
	}
}



static int bitsInMask(unsigned long mask)
{
    /* count bits in mask */
    int n = 0;

   while(mask) {
	n += mask&1;
	mask >>= 1;
    }

    return n;
}

/* --------------------------------------------------------------------	*/

static int maskShift(unsigned long mask)
{
    /* determine how far mask is shifted */
    int n = 0;
    while(!(mask&1)) {
	n++;
	mask >>= 1;
    }
    return n;
}

/* --------------------------------------------------------------------	*/
static void screenInit(void)
{
	int i;
	int 	screen;
	XVisualInfo 	visualInfo;
	  char *gammaString;
	  float gamma;

	char 	*display_name = NULL;

	static int screenInited=0;
	if(screenInited)
		return;
	screenInited=1;
	
	/* Get connection to X Server */
	if ( (display=XOpenDisplay(display_name)) == NULL )
	{
		printf("Cannot connect to X server %s\n",
		XDisplayName(display_name));
		exit(-1);
	}
	
	screen = DefaultScreen(display);
    	rootwin = XRootWindow(display,screen);

	if(XMatchVisualInfo(display, screen, 32, TrueColor, &visualInfo)
	|| XMatchVisualInfo(display, screen, 24, TrueColor, &visualInfo)
	|| XMatchVisualInfo(display, screen, 16, TrueColor, &visualInfo))
		{
		visual=visualInfo.visual;
		screenDepth=visualInfo.depth;
		mycmap   = XCreateColormap(display, rootwin, visual, AllocNone);

		red_bits = bitsInMask(visualInfo.red_mask);
		green_bits = bitsInMask(visualInfo.green_mask);
		blue_bits = bitsInMask(visualInfo.blue_mask);
		red_shift = maskShift(visualInfo.red_mask);
		green_shift = maskShift(visualInfo.green_mask);
		blue_shift = maskShift(visualInfo.blue_mask);
		}
	else
		{
		int r,g,b;
		int i=0;
		red_bits = 4;
		green_bits = 4;
		blue_bits = 4;
		red_shift = 8;
		green_shift = 4;
		blue_shift = 0;

		visual=XDefaultVisual(display,screen);
		screenDepth=XDefaultDepth(display,screen);
		mycmap=XDefaultColormap(display,screen);

		for(r=0;r<16;r++)
			for(g=0;g<16;g++)
				for(b=0;b<16;b++)
					{
					XColor thisColor;

					thisColor.red=(r<<12)+(r<<8)+(r<<4)+r;
					thisColor.green=(g<<12)+(g<<8)+(g<<4)+g;
					thisColor.blue=(b<<12)+(b<<8)+(b<<4)+b;

					
					i=r<<red_shift;
					i|=g<<green_shift;
					i|=b<<blue_shift;
					if(XAllocColor(display,mycmap,&thisColor))
						{
						colorLookup[i]=thisColor.pixel;
						}
					else
						colorLookup[i]=-1;
					}

		fillLookupTable();
		falseColor=1;
		}

	XParseColor(display, mycmap, "#000000", &black);
	if (!XAllocColor(display, mycmap, &black))
		fprintf(stderr, "Alloc colour failed??\n");
	XParseColor(display, mycmap, "#ffffff", &white);
	if (!XAllocColor(display, mycmap, &white))
		fprintf(stderr, "Alloc colour failed??\n");
	

	//We need to gamma correct for SOME x displays...
        gammaString=getenv("GAMMA");
        if(gammaString==NULL)
                gamma=1;
        else
                {
                gamma=atof(gammaString);
                if(gamma<=0 || gamma>10)
                        {
                        fprintf(stderr,"Check Gamma environment variable for sanity!\n");
                        gamma=1;
                        }
                }

        for(i=0;i<256;i++)
                gammaLut[i]=pow(((float)i)/255.0, 1.0/gamma)*255;
	OOgamma=1.0/gamma;
	displayGain=1;

	return;
}


Window windowInit(int dw, int dh, char* window_title,int winitFlags)
{
	Window  		mywin;
    	XSetWindowAttributes 	wattr;

	screenInit();


    wattr.event_mask = ButtonPressMask
			|ButtonReleaseMask
			|KeyPressMask
			|ExposureMask
			|PointerMotionMask;
    wattr.background_pixel = black.pixel;
    wattr.backing_store = Always;
    wattr.backing_planes = screenDepth;
    wattr.border_pixmap = None;
    wattr.border_pixel = black.pixel;
    wattr.colormap = mycmap;

    mywin = XCreateWindow(display, rootwin,
				110,10,
				dw, dh, 0,
			  	screenDepth, InputOutput, visual,
			  	CWEventMask|CWBackPixel|CWBorderPixel|CWBackingStore
			  |CWBackingPlanes|CWColormap,
			  &wattr);

	if(blackgc==0)
		{
		XGCValues values;
		values.foreground=black.pixel;
		values.background=white.pixel;
		blackgc = XCreateGC(display, mywin,
				(GCForeground|GCBackground), &values);
		}

	XMapWindow(display, mywin);

	setWindowTitle(mywin,window_title);

	return mywin;
}

void setWindowTitle(Window w,char *title)
{
	XTextProperty		xtextproperty;
	xtextproperty.value = (unsigned char *)title;
	xtextproperty.encoding = XA_STRING;
	xtextproperty.format = 8;		/* 8 bit non-DBCS */
	xtextproperty.nitems = strlen(title);
	XSetWMName(display, w, &xtextproperty);
	XSync(display, 0);
}

void screenDraw(Window the_window,  NCCAPixmap srcP)
{
	XWindowAttributes winInfo;
	XImage		*img;
	int	i,j;
    	unsigned char * src;
	char *image_Mem;


	assert(srcP.data);
	assert(srcP.spp >0 && srcP.spp<=4);

	XGetWindowAttributes(display,the_window,&winInfo);

	image_Mem = (char *) malloc(winInfo.height * winInfo.width
						* (screenDepth+8)/ 8);
	img = XCreateImage(display, visual, 
			screenDepth,
			ZPixmap,
			0, image_Mem,
			winInfo.width, winInfo.height,
			32, 0);

	if(srcP.bps == 8 && winInfo.height==srcP.height && winInfo.width==srcP.width)
		{
		int actualHeight,actualWidth;

		actualHeight = srcP.height;
		actualWidth = srcP.width;
		if(srcP.spp == 1 || srcP.spp == 2)
			{
			for (i = 0; i < actualHeight; i++)
				{
				src = srcP.data+ (i*srcP.width*srcP.spp*srcP.bps)/8;
				for (j = 0; j < actualWidth; j++)
					{
					long pixel;
					unsigned char val=gammaLut[*src];
					pixel  = 0;
					pixel |= ((long)(val >> (8 - red_bits)))
							<< red_shift;
					pixel |= ((long)(val >> (8 - green_bits)))
							<< green_shift;
					pixel |= ((long)(val >> (8 - blue_bits)))
							<< blue_shift;

					if(falseColor)
						XPutPixel(img, j, i, colorLookup[pixel]);
					else
						XPutPixel(img, j, i, pixel);

					src+=srcP.spp;
					}
				}
			}
		else if(srcP.spp == 3 || srcP.spp == 4)
			{
			for (i = 0; i < actualHeight; i++)
				{
				src = srcP.data+ (i*srcP.width*srcP.spp*srcP.bps)/8;
				for (j = 0; j < actualWidth; j++)
					{
					long pixel;
					pixel  = 0;
					pixel |= ((long)(gammaLut[src[0]] >> (8 - red_bits)))
									<< red_shift;
					pixel |= ((long)(gammaLut[src[1]] >> (8 - green_bits)))
									<< green_shift;
					pixel |= ((long)(gammaLut[src[2]] >> (8 - blue_bits)))
									<< blue_shift;
					if(falseColor)
						XPutPixel(img, j, i, colorLookup[pixel]);
					else
						XPutPixel(img, j, i, pixel);
					src+=srcP.spp;
					}
				}
			}
		}
	else
		{
		int red_mul=(1<<  red_bits)-1;
		int green_mul=(1<<  green_bits)-1;
		int blue_mul=(1<<  blue_bits)-1;

            for (i = 0; i < winInfo.height; i++)
            {
                for (j = 0; j < winInfo.width; j++)
				{
				NCCAPixel ip;
				long op;
				op  = 0;
				//Shoud this have -1's in?
				ip=getAAPixel(srcP, (((float)j)/winInfo.width)*srcP.width,
					(((float)i)/winInfo.height)*srcP.height);

				//Scale brightness for HDR images...
				ip.r*=displayGain;
				ip.g*=displayGain;
				ip.b*=displayGain;

				if(ip.r<0)ip.r=0;
				if(ip.g<0)ip.g=0;
				if(ip.b<0)ip.b=0;
				if(ip.r>1)ip.r=1;
				if(ip.g>1)ip.g=1;
				if(ip.b>1)ip.b=1;

				ip.r=powf(ip.r, OOgamma);
				ip.g=powf(ip.g, OOgamma);
				ip.b=powf(ip.b, OOgamma);

				op |= ((int)(ip.r*red_mul)) <<   red_shift;
				op |= ((int)(ip.g*green_mul)) << green_shift;
				op |= ((int)(ip.b*blue_mul)) <<  blue_shift;

				if(falseColor)
					XPutPixel(img, j, i, colorLookup[op]);
				else
					XPutPixel(img, j, i, op);
				src+=srcP.spp;
				}
			}
		}
	XPutImage(display, the_window, blackgc, img,
			0, 0, 0, 0,
			winInfo.width, winInfo.height);
	free(img -> data);
	img -> data = NULL;

	XDestroyImage(img);
}

static int nextEventIsExposure(Window w)
	{
	//If the we get two exposure events in a row, then
	//We want to avoid two redraws!
	//So this checks if the next event would generate a redraw...
	XEvent event;  
	if(!XPending(display))
		return 0;
	XPeekEvent(display,&event);

	switch(event.type)
		{
		case ConfigureNotify:
			if(w==event.xconfigurerequest.window)
				return 1;
			break;
		case Expose:
			if(w==event.xexpose.window)
				return 1;
			break;
		}
	return 0;
	}

static char **menuData=NULL;

void setMenuData(char **newMenu)
{
int i;
int menuCount;
if(menuData!=NULL)
	{
	//Delte Old Menu
	for(i=0;menuData[i]!=NULL;i++)
		free(menuData[i]);
	free(menuData);
	menuData=NULL;
	}

if(newMenu==NULL)
	return;

for(menuCount=0;newMenu[menuCount]!=NULL;menuCount++)
	;

menuData=malloc((menuCount+1)*sizeof(char *));
for(i=0;i<menuCount;i++)
	{
	menuData[i]=malloc(strlen(newMenu[i])+1);
	strcpy(menuData[i],newMenu[i]);
	}
menuData[i]=NULL;
}

static int popupMenu(XEvent ev)
{
XEvent event;
Window menu;
XSetWindowAttributes 	wattr;
static XFontStruct *font=NULL;
int mwidth=0;
int mheight=0;
int i;
int elementHeight=0;
int elementDescent=0;
int elementCount;
int selectedElement=-1;
int screen=DefaultScreen(display);
Visual *popupVisual=XDefaultVisual(display,screen);
int popupScreenDepth=XDefaultDepth(display,screen);
Colormap popupColormap=XDefaultColormap(display,screen);
static GC blackgc=0;
static GC whitegc=0;

if(font==NULL)
	font=XLoadQueryFont(display,"8x13");
if(font==NULL)
	font=XLoadQueryFont(display,"9x15");

for(i=0;menuData[i]!=NULL;i++)
	{
	int ascent,descent,direction;
	XCharStruct overall;
	XTextExtents(font,menuData[i],strlen(menuData[i]),
			&direction,&ascent,&descent,&overall);
	if(overall.width>mwidth)
		mwidth=overall.width;
	if(overall.ascent+overall.descent>elementHeight)
		elementHeight=overall.ascent+overall.descent;
	if(overall.descent>elementDescent)
		elementDescent=overall.descent;
	}
elementCount=i;
elementHeight+=3;
elementDescent+=1;
mwidth+=4;
mheight=elementCount*elementHeight;

wattr.override_redirect=1;
wattr.event_mask = ButtonPressMask
			|ButtonReleaseMask
			|KeyPressMask
			|ExposureMask
			|PointerMotionMask;

menu = XCreateWindow(display, rootwin,
				ev.xbutton.x_root-mwidth/2,ev.xbutton.y_root,
				mwidth, mheight, 0,
			  	popupScreenDepth, InputOutput, popupVisual,
				CWEventMask|CWOverrideRedirect, &wattr);
if(blackgc==0)
	{
	XGCValues values;
	XColor black,white;
	XParseColor(display, popupColormap, "#000000", &black);
	if (!XAllocColor(display, popupColormap, &black))
		fprintf(stderr, "Alloc colour failed??\n");
	XParseColor(display, popupColormap, "#ffffff", &white);
	if (!XAllocColor(display, popupColormap, &white))
		fprintf(stderr, "Alloc colour failed??\n");
	values.foreground=black.pixel;
	values.background=white.pixel;
	values.font=font->fid;

	blackgc = XCreateGC(display, menu,
			(GCForeground|GCBackground|GCFont), &values);
	values.foreground=white.pixel;
	values.background=black.pixel;
	whitegc = XCreateGC(display, menu,
			(GCForeground|GCBackground|GCFont), &values);
	}

XMapWindow(display,menu);

while(1)
	{
	int needsRedraw=0;;
	XNextEvent(display, &event);
	if(event.type==ButtonRelease && event.xbutton.button==ev.xbutton.button)
		{
		XDestroyWindow(display,menu);
		return selectedElement;
		}
	if(event.type==MotionNotify)
		{
		int newSelection;
		newSelection=(event.xbutton.y-ev.xbutton.y)/elementHeight;
		if(newSelection<0) newSelection=-1;
		if(newSelection>=elementCount) newSelection=-1;
		if(newSelection!=selectedElement)
			{
			selectedElement=newSelection;
			needsRedraw=1;
			}
		}
	if(needsRedraw || (event.type==Expose && event.xexpose.count==0))
		{
		XFillRectangle(display,menu,whitegc,0,0,mwidth,mheight);
		XDrawRectangle(display,menu,blackgc,0,0,mwidth-1,mheight-1);

		for(i=0;menuData[i]!=NULL;i++)
			{
			if(i==selectedElement)
				{
				XFillRectangle(display,menu,blackgc,
					0,i*elementHeight,
					mwidth,elementHeight);
				XDrawString(display,menu,whitegc,
					2,(i+1)*elementHeight-elementDescent,
					menuData[i],strlen(menuData[i]));
				}
			else
				{
				XDrawLine(display,menu,blackgc,
					0,(i+1)*elementHeight,
					mwidth,(i+1)*elementHeight);
				XDrawString(display,menu,blackgc,
					2,(i+1)*elementHeight-elementDescent,
					menuData[i],strlen(menuData[i]));
				}
			}
		}
	}
}


struct NCCAEvent_t catchEvents(int wait)
{
struct NCCAEvent_t NCCAEvent;
	NCCAEvent.type = EVENTTYPE_NULL;
	NCCAEvent.val = 0; //Silence Warnings
	NCCAEvent.w = 0; //Silence Warnings
	NCCAEvent.x = 0; //Silence Warnings
	NCCAEvent.y = 0; //Silence Warnings
	if(wait || XPending(display) > 0)
	    do
		{
		XEvent event;  
		static int mouseDown=0;
	
		XNextEvent(display, &event);
						
		switch(event.type)
			{
			case ButtonPress:
				if(menuData &&(event.xbutton.button==Button2
					|| event.xbutton.button==Button3))
					{
					int v=popupMenu(event);
					if(v==-1)
						break;
					NCCAEvent.type = EVENTTYPE_MENU;
					NCCAEvent.val = v;
					NCCAEvent.w=event.xbutton.window;
					return NCCAEvent;
					}
				NCCAEvent.type = EVENTTYPE_MOUSE;
				NCCAEvent.val = MOUSEDOWN;
				NCCAEvent.x=event.xbutton.x;
				NCCAEvent.y=event.xbutton.y;
				NCCAEvent.w=event.xbutton.window;
				mouseDown=1;
				return NCCAEvent;
			case ButtonRelease:
				NCCAEvent.type = EVENTTYPE_MOUSE;
				NCCAEvent.val = MOUSEUP;
				NCCAEvent.x=event.xbutton.x;
				NCCAEvent.y=event.xbutton.y;
				NCCAEvent.w=event.xbutton.window;
				mouseDown=0;
				return NCCAEvent;
			case MotionNotify:
				if(mouseDown)
					{
					NCCAEvent.type = EVENTTYPE_MOUSE;
					NCCAEvent.val = MOUSEDRAG;
					NCCAEvent.x=event.xbutton.x;
					NCCAEvent.y=event.xbutton.y;
					NCCAEvent.w=event.xbutton.window;
					break;
					}
				break;
                       case KeyPress:
                                NCCAEvent.type = EVENTTYPE_KEYBOARD;
				NCCAEvent.w=event.xkey.window;
                                NCCAEvent.val = XKeycodeToKeysym(
						event.xkey.display,
						event.xkey.keycode,
						0);
				return NCCAEvent;

			case Expose:
				{
				NCCAEvent.type = EVENTTYPE_EXPOSE;
				NCCAEvent.w=event.xexpose.window;
				if(nextEventIsExposure(NCCAEvent.w))
					break;
				XWindowAttributes winInfo;
				XGetWindowAttributes(display,NCCAEvent.w,&winInfo);
				NCCAEvent.x=winInfo.width;
				NCCAEvent.y=winInfo.height;

				return NCCAEvent;
				}
			case ConfigureNotify:
				{
				break;
				NCCAEvent.type = EVENTTYPE_EXPOSE;
				NCCAEvent.w=event.xconfigurerequest.window;
				if(nextEventIsExposure(NCCAEvent.w))
					break;
				XWindowAttributes winInfo;
				XGetWindowAttributes(display,NCCAEvent.w,&winInfo);
				NCCAEvent.x=winInfo.width;
				NCCAEvent.y=winInfo.height;
				return NCCAEvent;
				}

			default:
				printf("Unknown event\n");
				break;
			}
		}
	    while(XPending(display)>0);
	return NCCAEvent;
}

void setDisplayBrightness(float scale)
{
displayGain=scale;
}
