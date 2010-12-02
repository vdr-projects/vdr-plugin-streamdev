#ifndef _OSD_H_
#define _OSD_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
int OSDClose(int dev);
int OSDOpen(int dev, int x0, int y0, int x1, int y1, int BitPerPixel, int mix);
int OSDShow(int dev);
int OSDHide(int dev); 
int OSDClear(int dev); 
int OSDFill(int dev, int color); 
int OSDSetColor(int dev, int color, int r, int g, int b, int op);
int OSDText(int dev, int x, int y, int size, int color, const char *text);
int OSDSetPalette(int dev, int first, int last, unsigned char *data);
int OSDSetTrans(int dev, int trans);
int OSDSetPixel(int dev, int x, int y, unsigned int color);
int OSDGetPixel(int dev, int x, int y);
int OSDSetRow(int dev, int x, int y, int x1, unsigned char *data);
int OSDSetBlock(int dev, int x, int y, int x1, int y1, int inc, unsigned char *data);
int OSDFillRow(int dev, int x, int y, int x1, int color);
int OSDFillBlock(int dev, int x, int y, int x1, int y1, int color);
int OSDLine(int dev, int x, int y, int x1, int y1, int color);
int OSDQuery(int dev);
int OSDSetWindow(int dev, int win);
int OSDMoveWindow(int dev, int x, int y);
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif
