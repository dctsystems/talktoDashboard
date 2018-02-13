/*
 * $Log: Win_NT.c,v $
 * Revision 1.8  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
 * Revision 1.7  2007/06/28 14:12:00  ian
 * Improve namespace usage
 *
 * Revision 1.6  2001/06/29 16:23:01  ian
 * add support for displaying mono pixmaps
 *
 * Revision 1.5  2001/04/29 16:17:46  ian
 * add popup menu support
 *
 * Revision 1.4  2000/11/26 19:44:01  ian
 * add wait parameter to Catch_X_Events
 *
 * Revision 1.3  2000/09/17 16:35:01  ian
 * add SetWindowTitle, so title can be changed after the window
 * has been created
 *
 * Revision 1.2  2000/09/10 18:49:37  ian
 * add gamma correction for all screen draws
 *
 * Revision 1.1  2000/09/10 16:09:23  ian
 * Initial revision
 *
 */

#include <math.h>

static struct NCCAEvent_t lastEvent={
	EVENTTYPE_NULL,
	0,0,
	0,
	0
	};
	
unsigned char gammaLut[256];
static float displayGain;
HMENU hMenu=NULL;

LONG WINAPI __nccaWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
	{
	case WM_CLOSE:
		{
		//PostQuitMessage(0);
		lastEvent.type=EVENTTYPE_QUIT;
		return 0;
		}
	case WM_PAINT:
		{
	        RECT r;
		lastEvent.type=EVENTTYPE_EXPOSE;
		lastEvent.w=hwnd;
	        GetClientRect(hwnd,&r);
		lastEvent.x=r.right-r.left;
		lastEvent.y=r.bottom-r.top;
		return 0;
		}
	case WM_CHAR:
		{
		lastEvent.type=EVENTTYPE_KEYBOARD;
		lastEvent.w=hwnd;
		lastEvent.val=wParam;
		return 0;
		}
	case WM_LBUTTONDOWN:
		{
		lastEvent.type=EVENTTYPE_MOUSE;
		lastEvent.w=hwnd;
		lastEvent.val=MOUSEDOWN;
		lastEvent.x=lParam & 0xffff;
		lastEvent.y=lParam >>16;
		return 0;
		}
	case WM_LBUTTONUP:
		{
		lastEvent.type=EVENTTYPE_MOUSE;
		lastEvent.w=hwnd;
		lastEvent.val=MOUSEUP;
		lastEvent.x=lParam & 0xffff;
		lastEvent.y=lParam >>16;
		return 0;
		}
	case WM_MOUSEMOVE:
		{
		if(wParam && MK_LBUTTON)
			{
			lastEvent.type=EVENTTYPE_MOUSE;
			lastEvent.w=hwnd;
			lastEvent.val=MOUSEDRAG;
			lastEvent.x=lParam & 0xffff;
			lastEvent.y=lParam >>16;
			}
		return 0;
		}
	case WM_RBUTTONDOWN :
		{
		if(hMenu)
			{
			POINT point;
			point.x=lParam & 0xffff;
			point.y=lParam >>16;
			ClientToScreen(hwnd,&point);
			TrackPopupMenu(hMenu,0,point.x,point.y,0,hwnd,NULL);
			}
		return 0;
		}
	case WM_COMMAND :
		{
		lastEvent.type=EVENTTYPE_MENU;
		lastEvent.w=hwnd;
		lastEvent.val=wParam;
		return 0;
		}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam); 
	}
}


void setDisplayBrightness(float scale)
{
  displayGain=scale;
}


static int screenInit(void)
{
  static char *classname=0;
  WNDCLASS  wc;
  HINSTANCE hInstance = GetModuleHandle(NULL);
  int i;
  char *gammaString;
  float gamma;
  displayGain=1;
  
  /* Make sure we register the window only once. */
  if(classname)
    return 0;

  classname = "NCCA";

  /* Clear (important!) and then fill in the window class structure. */
  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style         = CS_OWNDC;
  wc.lpfnWndProc   = (WNDPROC)__nccaWindowProc;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = classname;
  
  if(!RegisterClass(&wc))
  	{
    fprintf(stderr,"Failed to register window class\n");
	exit(-1);
	}
	
	//One WIN32 we _REALLY_ need to do our own gamma correction...
	gammaString=getenv("GAMMA");
	if(gammaString==NULL)
		gamma=2;
	else
		{
		gamma=atof(gammaString);
		if(gamma<=0 || gamma>10)
			{
			fprintf(stderr,"Check Gamma environment variable for sanity!\n");
			gamma=2;
			}
		}
	for(i=0;i<256;i++)
		gammaLut[i]=pow(((float)i)/255.0, 1.0/gamma)*255;

	return 1;
}


Window windowInit(int dw, int dh, char* window_title,int winitFlags)
{
	HWND win;
	int winStyle=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
	int resizeFlags=(winitFlags&NCCA_RESIZE)
				?(WS_THICKFRAME|WS_MAXIMIZEBOX):0;
	screenInit();

	RECT winR;
#define DUMMY 100
	winR.left=DUMMY;
	winR.right=DUMMY+dw;
	winR.top=DUMMY;
	winR.bottom=DUMMY+dh;
	AdjustWindowRect(&winR,winStyle,FALSE);

	win= CreateWindow("NCCA", "NCCA", winStyle|resizeFlags,
					CW_USEDEFAULT, CW_USEDEFAULT,
					winR.right-winR.left, winR.bottom-winR.top,
					0, NULL,
					GetModuleHandle(NULL), 0);
	SetWindowText(win, window_title);
	ShowWindow(win, SW_SHOWNORMAL);
	return (Window *)win;
}

void setWindowTitle(Window w,char *text)
{
SetWindowText(w, text);
}

void screenDraw(Window the_window,  NCCAPixmap srcP)
{
    LPBITMAPINFOHEADER dib;
    HDC hdc;
    unsigned char *data;
    int i,j;
    RECT r;
    int winWidth=((srcP.width*3)+3)& ~3;

	assert(srcP.data);

    // Allocate a chunk of memory, cast
    // as a BITMAPINFOHEADER struct for
    // easy initializing.
    dib = (LPBITMAPINFOHEADER) malloc (sizeof(BITMAPINFOHEADER));
    if (!dib)
		{
        return;
		}
    data = (unsigned char *) malloc(winWidth*srcP.height);
    if (!data)
		{
		free(dib);
        return;
		}
	
    dib->biSize = sizeof(BITMAPINFOHEADER);
    dib->biWidth = srcP.width;
    dib->biHeight = srcP.height;
    dib->biPlanes = 1;      // Always 1
    dib->biBitCount = 24;
    dib->biCompression = BI_RGB;
    dib->biSizeImage = 0;   // Not needed for BI_RGB
    dib->biXPelsPerMeter = 0;
    dib->biYPelsPerMeter = 0;   // These are arbitrary
    dib->biClrUsed = 0;     // Use biBitCount
    dib->biClrImportant = 0;    // Not important

    if(srcP.bps==8)
	    {
	    if(srcP.spp>2)
		    {
		    for(i=0;i<srcP.width;i++)
			for(j=0;j<srcP.height;j++)
				{
				unsigned char*src=srcP.data+(srcP.width*j+i)*srcP.spp;
				unsigned char*dest=data+winWidth*(srcP.height-1-j)+i*3;
				dest[0]=gammaLut[src[2]];
				dest[1]=gammaLut[src[1]];
				dest[2]=gammaLut[src[0]];
				}
		    }
	    else
		    {
		    for(i=0;i<srcP.width;i++)
			for(j=0;j<srcP.height;j++)
				{
				unsigned char*src=srcP.data+(srcP.width*j+i)*srcP.spp;
				unsigned char*dest=data+winWidth*(srcP.height-1-j)+i*3;
				dest[0]=dest[1]=dest[2]=gammaLut[src[0]];
				}
	            }
	    }
    else
            {
            if(srcP.spp>2)
                    {
                    for(i=0;i<srcP.width;i++)
                        for(j=0;j<srcP.height;j++)
                                {
                                unsigned char*dest=data+winWidth*(srcP.height-1-j)+i*3;
				NCCAPixel src=getPixelColor(srcP,i,j);
				int v=src.b*255*displayGain;
				if(v<0)v=0; if(v>255)v=255;
                                dest[0]=gammaLut[v];
				v=src.g*255*displayGain;
				if(v<0)v=0; if(v>255)v=255;
                                dest[1]=gammaLut[v];
				v=src.r*255*displayGain;
				if(v<0)v=0; if(v>255)v=255;
                                dest[2]=gammaLut[v];
                                }
                    }
            else
                    {
                    for(i=0;i<srcP.width;i++)
                        for(j=0;j<srcP.height;j++)
                                {
                                unsigned char*dest=data+winWidth*(srcP.height-1-j)+i*3;
				float src=getPixelGrey(srcP,i,j);
				int v=src*255*displayGain;
				if(v<0)v=0; if(v>255)v=255;
                                dest[0]=dest[1]=dest[2]=gammaLut[v];
                                }
                    }
            }

    hdc=GetDC(the_window);
    GetClientRect(the_window,&r);

    StretchDIBits(hdc, 0, 0,
                       r.right, r.bottom,
                       0, 0,
                       srcP.width, srcP.height,
                       data,
                       (LPBITMAPINFO)dib,
                       DIB_RGB_COLORS,
                       SRCCOPY);
    ValidateRect(the_window,&r);
    free(dib);
    free(data);
}

/* --------------------------------------------------------------------	*/

void setMenuData(char **newMenu)
{
screenInit();

if(hMenu)
	DestroyMenu(hMenu);
hMenu=NULL;

if(newMenu)
	{
	int i;
	hMenu=CreatePopupMenu();
	for(i=0;newMenu[i]!=NULL;i++)
		AppendMenu(hMenu,MF_STRING,i,newMenu[i]);
	}
}
/* --------------------------------------------------------------------	*/

struct NCCAEvent_t catchEvents(int wait)
{
    MSG event;
    lastEvent.type=EVENTTYPE_NULL;
    if(wait)
	{
	do
		{
		if(GetMessage(&event, NULL, 0, 0))
			{
			TranslateMessage(&event);
			DispatchMessage(&event);
			}
		else
			{
			lastEvent.type = EVENTTYPE_QUIT;
			}
		}
	    while(lastEvent.type==EVENTTYPE_NULL);
	}
    else
	{
	if(PeekMessage(&event, NULL, 0, 0,PM_REMOVE))
		{
		if(event.message==WM_QUIT)
			lastEvent.type = EVENTTYPE_QUIT;
		else
			{
			TranslateMessage(&event);
			DispatchMessage(&event);
			}
		}
	}
    return lastEvent;
}
