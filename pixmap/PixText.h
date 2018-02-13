#ifdef __cplusplus
extern "C" {
#endif

void blitChar(NCCAPixmap p,unsigned char c, int x, int y,NCCAPixel color);
void blitString(NCCAPixmap p, char *s, int x, int y, NCCAPixel col);

struct  font_t
	{
	char *name;
	int width;
	int height;
	unsigned char *data;
	};

#define FONT_COUNT 5

extern struct font_t DCTFonts[FONT_COUNT];

int stringWidth(char *s);
void selectFont(struct font_t DCTFonts);

#ifdef __cplusplus
} /* extern "C" */
#endif

