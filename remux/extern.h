#ifndef VDR_STREAMDEV_EXTERNREMUX_H
#define VDR_STREAMDEV_EXTERNREMUX_H

#include "remux/tsremux.h"
#include <vdr/ringbuffer.h>

class cTSExt;

class cExternRemux: public cTSRemux {
private:
	cRingBufferLinear *m_ResultBuffer;
	cTSExt            *m_Remux;

public:
	cExternRemux(int VPid, const int *APids, const int *Dpids, const int *SPids);
	virtual ~cExternRemux();
	
	int Put(const uchar *Data, int Count);
	uchar *Get(int &Count) { return m_ResultBuffer->Get(Count); }
	void Del(int Count) { m_ResultBuffer->Del(Count); }
};

#endif // VDR_STREAMDEV_EXTERNREMUX_H
