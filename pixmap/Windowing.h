/*
$Log: Windowing.h,v $
Revision 1.12  2009/09/10 12:26:16  ian
Various updates for 09 release

Revision 1.11  2007/06/28 14:12:00  ian
Improve namespace usage

Revision 1.10  2001/04/29 16:18:11  ian
add popup menu support

Revision 1.9  2000/11/26 18:01:55  ian
Rework ifdefs for MAC port
add wait parameter to Catch_X_Events

Revision 1.8  2000/09/17 16:35:01  ian
add SetWindowTitle, so title can be changed after the window
has been created

Revision 1.7  2000/08/31 19:36:08  ian
resize flag to create window

Revision 1.6  2000/08/28 16:15:44  ian
use #defines for MOUSEUP/DOWN and DRAG rather than magic numbers

Revision 1.5  2000/08/26 17:41:28  ian
Major overhaul to catchEvents to handle Kendra UI
bug fixes to WIN32 drawing code

 * Revision 1.4  20/0./5.  8.:4.:3.  ian
 * add WIN32 support
 * 
 * Revision 1.3  10/.0/.2  .2:.4:.3  ian
 * add support for OpenStep
 * 
Revision 1.2  1998/04/03 13:30:38  mquek
Created Window_Init() and modified Screen_Init() and Screen_Draw() to
allow for multiple windows.

Revision 1.1  1998/02/23 14:58:15  istephen
Initial revision

*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
	#include <windows.h>
	typedef HWND Window;
#else
	#ifdef NeXT
		typedef void * Window;
	#else
            #ifdef __APPLE__
                    typedef void *Window;
            #else
                    #include <X11/X.h>
            #endif
        #endif
#endif


/* Create a window (but don't draw into it yet) */
Window windowInit(int dw, int dh, char* window_title, int flags);
#define NCCA_RESIZE 1

void setWindowTitle(Window w,char *text);

/* Copy pixmap into a window */
void screenDraw(Window the_window, NCCAPixmap src);


#define EVENTTYPE_NULL 0
#define EVENTTYPE_MOUSE 1
#define EVENTTYPE_KEYBOARD 2
#define EVENTTYPE_EXPOSE 3
#define EVENTTYPE_MENU 4
#define EVENTTYPE_QUIT 99

#define MOUSEUP 0
#define MOUSEDOWN 1
#define MOUSEDRAG 3

struct NCCAEvent_t
	{
	int type;
	int x,y;
	Window w;
	int val;
	};

struct NCCAEvent_t catchEvents(int wait);
void setMenuData(char **newMenu);

void setDisplayBrightness(float scale);
#ifdef __cplusplus
} /* extern "C" */
#endif

