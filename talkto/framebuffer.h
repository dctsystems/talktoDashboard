#define NCCA_RESIZE 1
typedef void *Window;

Window FBwindowInit(int w, int h, char*title, int flags);
void FBscreenDraw(Window w, NCCAPixmap p);
