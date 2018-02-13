/*
 * $Log: PixFileIO.h,v $
 * Revision 1.3  2009/09/10 12:26:16  ian
 * Various updates for 09 release
 *
 * Revision 1.2  2007/06/28 14:12:00  ian
 * Use improved function names
 *
 * Revision 1.1  2000/09/10 16:11:05  ian
 * Initial revision
 *
 */
#ifdef __cplusplus
extern "C" {
#endif


NCCAPixmap loadPixmap(char *filename);
void savePixmap( NCCAPixmap p, char *filename);
void setJPGCompression(int percent);

#ifdef __cplusplus
} /* extern "C" */
#endif


