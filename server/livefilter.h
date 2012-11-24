/*
 *  $Id: livefilter.h,v 1.5 2008/04/07 14:27:31 schmirl Exp $
 */

#ifndef VDR_STREAMEV_LIVEFILTER_H
#define VDR_STREAMEV_LIVEFILTER_H

#include "server/streamer.h"

class cDevice;
class cStreamdevLiveFilter;

class cStreamdevFilterStreamer: public cStreamdevStreamer {
private:
	cDevice                *m_Device;
	cStreamdevLiveFilter   *m_Filter;

public:
	cStreamdevFilterStreamer();
	virtual ~cStreamdevFilterStreamer();

	void SetDevice(cDevice *Device);
	bool SetFilter(u_short Pid, u_char Tid, u_char Mask, bool On);
	
	virtual void Attach(void);
	virtual void Detach(void);
};

#endif // VDR_STREAMEV_LIVEFILTER_H
