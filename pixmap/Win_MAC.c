/*
$Log: Win_MAC.c,v $
Revision 1.1  2007/06/28 14:12:00  ian
Initial revision

*/

#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Events.h>
#include <Windows.h>


QDGlobals qd;

/* Initialise Display */
static int screenInit(void)
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
return 1;

}

/* Create a window (but don't draw into it yet) */
Window windowInit(int dw, int dh, char* window_title, int flags)
{
	Rect windRect;
	Window w;

	windRect.left=50; 
	windRect.top=50; 
	windRect.right=50+dw; 
	windRect.bottom=50+dh; 

	w = NewCWindow(nil,
			&windRect,
			"\pMy Window",
			true,
			documentProc,
			(WindowPtr) -1,
			false,
			0);
	return w;
}

void setWindowTitle(Window w,char *text)
{
	SetPort(w);
}

/* Copy pixmap into a window */
void screenDraw(Window the_window, NCCAPixmap src)
{
	SetPort(the_window);
}


struct NCCAEvent_t catchEvents(int wait)
{
	struct NCCAEvent_t event;
	event.type=EVENTTYPE_NULL;
	if(Button())
		event.type=EVENTTYPE_QUIT;
	return event;		
}
