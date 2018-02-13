/*
 * $Log: Win_NeXT.c,v $
 * Revision 1.3  2007/06/28 14:12:00  ian
 * Improve namespace usage
 *
 * Revision 1.2  2001/01/08 20:13:40  ian
 * add wait parameter to Catch_X_Events (currently ignored)
 *
 * Revision 1.1  2000/09/10 16:09:23  ian
 * Initial revision
 *
 */
#undef DARWIN
#import <Cocoa/Cocoa.h>



@interface SniffView : NSView<NSWindowDelegate> {
    
    NSBitmapImageRep         *   nsBitmapImageRepObj;
}
- (void) setPixelAtX:(int) x y:(int)y color:(NSUInteger *)c;
- (void)windowWillClose:(NSNotification *)notification;
@end


@implementation SniffView
-(id)initX:(int)width Y:(int)height
{
    [super init];
    nsBitmapImageRepObj = [[NSBitmapImageRep alloc]
                           initWithBitmapDataPlanes:NULL
                           pixelsWide:width
                           pixelsHigh:height
                           bitsPerSample:8
                           samplesPerPixel:3
                           hasAlpha:NO
                           isPlanar:NO
                           colorSpaceName:@"NSCalibratedRGBColorSpace"
                           bytesPerRow:0
                           bitsPerPixel:0];
    
    return self;
}

- (void) setPixelAtX:(int) x y:(int)y color:(NSUInteger *)c
{
    y=480-1-y;
    [nsBitmapImageRepObj setPixel:c atX:x y:y];
}

-(uint8_t *)bitmapData
{
    return [nsBitmapImageRepObj bitmapData];
}
-(NSUInteger)bytesPerRow
{
    return [nsBitmapImageRepObj bytesPerRow];
}

- (void)windowWillClose:(NSNotification *)notification
{
    exit(0);
}

- (void)drawRect:(NSRect)pRect {


    [NSGraphicsContext saveGraphicsState];
    
    [nsBitmapImageRepObj drawInRect:pRect];
    
    [NSGraphicsContext restoreGraphicsState];
} // end drawRect

@end


static struct NCCAEvent_t lastEvent = {
	EVENTTYPE_NULL,
	0,0,
	0,
	0
	};


    NSAutoreleasePool *pool;
static void screenInit(void)
{
    static int screenInited=0;
    if(screenInited)
        return;
    
    pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];
    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
                                                  action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
    
    
    [NSApp finishLaunching];
}

Window windowInit(int dw, int dh, char* window_title, int flags)
{
    screenInit();

    NSWindow *window = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, dw, dh)
                                                    styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable backing:NSBackingStoreBuffered defer:NO]
                        autorelease];
    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    id appName = [[NSProcessInfo processInfo] processName];
    [window setTitle:appName];
    SniffView*view=[[SniffView alloc]initX:dw Y:dh];
    [window setContentView:view];
    [window setDelegate:view];
    [window makeKeyAndOrderFront:nil];
    return (Window) window;
}
static float OOgamma=1;
static float displayGain=1;

void screenDraw(Window the_window,  NCCAPixmap srcP)
{
    SniffView *view=[(NSWindow *)the_window contentView];
    int vw=[view frame].size.width;
    int vh=[view frame].size.height;
    uint8_t *backBuffer=[view bitmapData];
    int paddedWidth=[view bytesPerRow];
    int i,j;
    for (i = 0; i < vh; i++)
    {
        for (j = 0; j < vw; j++)
        {
            NCCAPixel ip;
            unsigned int pix_offset = j * 3 + i * paddedWidth;
            //Shoud this have -1's in?
            ip=getAAPixel(srcP, (((float)j)/vw)*srcP.width,
                          (((float)i)/vh)*srcP.height);
            
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

            *((char*)(backBuffer + pix_offset)) = ip.r*255;
            *((char*)(backBuffer + pix_offset + 1)) = ip.g*255;
            *((char*)(backBuffer + pix_offset + 2)) = ip.b*255;
        }
    }

    [view setNeedsDisplay:YES];
}


struct NCCAEvent_t catchEvents(int wait)
{
    struct NCCAEvent_t NCCAEvent;
    NCCAEvent.type = EVENTTYPE_NULL;
    NCCAEvent.val = 0; //Silence Warnings
    NCCAEvent.w = 0; //Silence Warnings
    NCCAEvent.x = 0; //Silence Warnings
    NCCAEvent.y = 0; //Silence Warnings
    int SB_waitForEvents=0;
    static int mouseDown=0;

    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:SB_waitForEvents?[NSDate distantFuture]:nil
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];

    switch ([event type])
    {
        case NSEventTypeLeftMouseDown:
        {
            NSPoint where=[[[event window] contentView] convertPoint:[event locationInWindow] fromView:nil];
            if(where.x<0 || where.x>=640 || where.y<0 || where.y>=480)
            {
                [NSApp sendEvent:event];
                break;
            }
            NCCAEvent.type = EVENTTYPE_MOUSE;
            NCCAEvent.val = MOUSEDOWN;
            NCCAEvent.x=where.x;
            NCCAEvent.y=where.y;
            NCCAEvent.w=[event window];
            mouseDown=1;
            break;

            
        }
        case NSEventTypeLeftMouseUp:
        {
            NSPoint where=[[[event window] contentView] convertPoint:[event locationInWindow] fromView:nil];
            if(where.x<0 || where.x>=640 || where.y<0 || where.y>=480)
            {
                [NSApp sendEvent:event];
                break;
            }
            NCCAEvent.type = EVENTTYPE_MOUSE;
            NCCAEvent.val = MOUSEUP;
            NCCAEvent.x=where.x;
            NCCAEvent.y=where.y;
            NCCAEvent.w=[event window];
            mouseDown=0;
            break;
        }
        case NSEventTypeLeftMouseDragged:
        {
            NSPoint where=[[[event window] contentView] convertPoint:[event locationInWindow] fromView:nil];
            if(where.x<0 || where.x>=640 || where.y<0 || where.y>=480)
            {
                [NSApp sendEvent:event];
                break;
            }
            if(mouseDown)
            {
                NCCAEvent.type = EVENTTYPE_MOUSE;
                NCCAEvent.val = MOUSEDRAG;
                NCCAEvent.x=where.x;
                NCCAEvent.y=where.y;
                NCCAEvent.w=[event window];
                break;
            }
            break;
        }
        case NSEventTypeKeyDown:
        {
            unichar const c=[[event characters] characterAtIndex:0];
            //printf("KeyDown:%d\n",c);
            if(c>32 && c <128)
                {
                NCCAEvent.type = EVENTTYPE_KEYBOARD;
                NCCAEvent.val=c;
                NCCAEvent.w=[event window];
                }
            else
                {
               [NSApp sendEvent:event];
                }
            break;
        }
        default:
            [NSApp sendEvent:event];
    }
    [NSApp updateWindows];
    [pool release];
    pool = [[NSAutoreleasePool alloc] init];

    return NCCAEvent;
}
void setMenuData(char **newMenu)
{
}
void setWindowTitle(Window w,char *title)
{
    [(NSWindow *)w setTitle:[NSString stringWithCString:title encoding:NSASCIIStringEncoding]];
}
