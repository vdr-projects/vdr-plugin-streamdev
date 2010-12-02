#ifndef VDR_STREAMDEV_TS2PESREMUX_H
#define VDR_STREAMDEV_TS2PESREMUX_H

#include "remux/tsremux.h"
#include <vdr/remux.h>
#include <vdr/ringbuffer.h>

class cTS2PS;

class cTS2PSRemux: public cTSRemux {
private:
	int                m_NumTracks;
	cTS2PS            *m_Remux[MAXTRACKS];
	cRingBufferLinear *m_ResultBuffer;
	int                m_ResultSkipped;
	int                m_Skipped;
	bool               m_Synced;
	bool               m_IsRadio;
	
public:
	cTS2PSRemux(int VPid, const int *Apids, const int *Dpids, const int *Spids);
	virtual ~cTS2PSRemux();

	int Put(const uchar *Data, int Count);
	uchar *Get(int &Count);
	void Del(int Count) { m_ResultBuffer->Del(Count); }
};

#endif // VDR_STREAMDEV_TS2PESREMUX_H
