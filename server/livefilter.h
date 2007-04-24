/*
 *  $Id: livefilter.h,v 1.4 2007/04/24 11:29:29 schmirl Exp $
 */

#ifndef VDR_STREAMEV_LIVEFILTER_H
#define VDR_STREAMEV_LIVEFILTER_H

#include <vdr/config.h>

#	if VDRVERSNUM >= 10300

#include <vdr/filter.h>

class cStreamdevStreamer;

class cStreamdevLiveFilter: public cFilter {
private:
	cStreamdevStreamer *m_Streamer;

protected:
	virtual void Process(u_short Pid, u_char Tid, const u_char *Data, int Length);

public:
	cStreamdevLiveFilter(cStreamdevStreamer *Streamer);

	void Set(u_short Pid, u_char Tid, u_char Mask) {
		cFilter::Set(Pid, Tid, Mask);
	}
	void Del(u_short Pid, u_char Tid, u_char Mask) {
		cFilter::Del(Pid, Tid, Mask);
	}
};

#	endif // VDRVERSNUM >= 10300
#endif // VDR_STREAMEV_LIVEFILTER_H
