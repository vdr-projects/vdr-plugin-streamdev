#ifndef _OSD_HH_
#define _OSD_HH_

extern "C" {
#include "OSD.h"
}
struct OSD {
        int dev;
        
        void init(int d) {
	        dev=d;
	}
        int Open(int x0, int y0, int x1, int y1, int BitPerPixel, int mix, int win) {
	        if (OSDSetWindow(dev, win))
		        return -1;
	        return OSDOpen(dev, x0, y0, x1, y1, BitPerPixel, mix);
	}
        int Open(int x0, int y0, int x1, int y1, int BitPerPixel, int mix) {
	        return OSDOpen(dev, x0, y0, x1, y1, BitPerPixel, mix);
	}
        int Close(int win) {
	        if (OSDSetWindow(dev, win))
		        return -1;
	        return OSDClose(dev);
	}
        int Close(void) {
	        return OSDClose(dev);
	}
        int Show(void) {
	        return OSDShow(dev);
	}
        int Hide(void) {
	        return OSDHide(dev);
	}
        int Clear(void) {
	        return OSDClear(dev);
	}
        int Fill(int color) {
	        return OSDFill(dev, color);
	}
        int SetColor(int color, int r, int g, int b, int op) {
	        return OSDSetColor(dev, color, r, g, b, op);
	}
        int Text(int x, int y, int size, int color, const char *text) {
	        return OSDText(dev, x, y, size, color, text); 
	}
        int SetPalette(int first, int last, unsigned char *data) {
	        return OSDSetPalette(dev, first, last, data);

	}
        int SetTrans(int trans) {
	        return OSDSetTrans(dev, trans);

	}
        int SetPixel(int dev, int x, int y, unsigned int color) {
	        return OSDSetPixel(dev, x, y, color); 
	}
        int GetPixel(int dev, int x, int y) {
	        return OSDGetPixel(dev, x, y); 
	}
        int SetRow(int x, int y, int x1, unsigned char *data) {
	        return OSDSetRow(dev, x, y, x1, data); 
	}
        int SetBlock(int x, int y, int x1, int y1, int inc, unsigned char *data) {
	        return OSDSetBlock(dev, x, y, x1, y1, inc, data); 
	}
        int FillRow(int x, int y, int x1, int color) {
	        return OSDFillRow(dev, x, y, x1, color); 
	}
        int FillBlock(int x, int y, int x1, int y1, int color) {
	        return OSDFillBlock(dev, x, y, x1, y1, color); 
	}
        int Line(int x, int y, int x1, int y1, int color) {
	        return OSDLine(dev, x, y, x1, y1, color); 
	}
        int Query() {
	        return OSDQuery(dev); 
	}
        int SetWindow(int win) {
	        return OSDSetWindow(dev, win); 
	}
};

#endif
