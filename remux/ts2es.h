#ifndef VDR_STREAMDEV_TS2ESREMUX_H
#define VDR_STREAMDEV_TS2ESREMUX_H

#include "remux/tsremux.h"

class cTS2ES;

class cTS2ESRemux: public cTSRemux {
private:
	int m_Pid;
	cTS2ES *m_Remux;

protected:
	virtual void PutTSPacket(int Pid, const uint8_t *Data);

public:
	cTS2ESRemux(int Pid);
	virtual ~cTS2ESRemux();
};

#endif // VDR_STREAMDEV_TS2ESREMUX_H
