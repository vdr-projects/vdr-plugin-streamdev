#ifndef VDR_STREAMDEV_TS2PESREMUX_H
#define VDR_STREAMDEV_TS2PESREMUX_H

#include "remux/tsremux.h"

class cTS2PS;

class cTS2PSRemux: public cTSRemux {
private:
	int m_VPid, m_APid1, m_APid2, m_DPid1, m_DPid2;
	cTS2PS *m_VRemux, *m_ARemux1, *m_ARemux2, *m_DRemux1, *m_DRemux2;

protected:	
	virtual void PutTSPacket(int Pid, const uint8_t *Data);

public:
	cTS2PSRemux(int VPid, int APid1, int APid2, int DPid1, int DPid2, 
			bool PS = false);
	virtual ~cTS2PSRemux();
};

#endif // VDR_STREAMDEV_TS2PESREMUX_H
