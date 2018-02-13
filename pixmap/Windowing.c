/*
$Log: Windowing.c,v $
Revision 1.17  2009/09/10 12:26:16  ian
Various updates for 09 release

Revision 1.16  2007/06/28 14:12:00  ian
Improve namespace usage

Revision 1.15  2000/11/26 18:49:39  ian
add ifdef for MAC port

Revision 1.14  2000/09/10 16:09:23  ian
Split the individual platforms into their own files
but still held together by Windowing.c

Revision 1.13  2000/08/31 19:36:08  ian
For Win32: fix cursor problem, resize flags, better positioninh

Revision 1.12  2000/08/28 16:15:44  ian
use #defines for MOUSEUP/DOWN and DRAG rather than magic numbers

Revision 1.11  2000/08/27 13:59:39  ian
Validate Rect in WIN32 drawing code
Reshuffle WIN32 event handling to avoid busy waiting

Revision 1.10  2000/08/26 17:41:28  ian
Major overhaul to catchEvents to handle Kendra UI
bug fixes to WIN32 drawing code

Revision 1.9  2000/08/20 19:09:37  ian
get height and width right way round under Windows!

 * Revision 1.8  20/0./5.  9.:9.:9.  ian
 * use correct colour spaces when creating NeXT bitmaps
 * 
Revision 1.7  2000/05/28 14:03:44  ian
add WIN32 support

Revision 1.6  2000/05/24 20:48:20  ian
add OpenStep support

Revision 1.5  2000/04/18 11:50:44  ian
Use ifdefs to protect against compiling on non X platforms

Revision 1.4  1999/05/17 19:40:30  ian
Bail out if the connect fails

Revision 1.3  1999/04/01 15:32:08  istephen
Remove printf about visual depth

Revision 1.2  1998/04/03 13:34:33  mquek
Created Window_Init() and modified Screen_Init() and Screen_Draw() to
allow for multiple windows.

Revision 1.1  1998/02/23 14:58:15  istephen
Initial revision

*/
#include <assert.h>

#include <stdlib.h>
#include <stdio.h>

#include "NCCAPixmap.h"
#include "Windowing.h"


#ifdef WIN32
	#include "Win_NT.c"
#else
	#ifdef NeXT
		#include "Win_NeXT.c"
        #else
            #ifdef __APPLE__
		    #include "Win_NeXT.c"
            #else
		    #include "Win_X.c"
	    #endif
	#endif
#endif


/* End of file */
