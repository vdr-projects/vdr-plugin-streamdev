/*
 *  $Id: livefilter.h,v 1.1 2004/12/30 22:44:27 lordjaxom Exp $
 */

#ifndef VDR_STREAMEV_LIVEFILTER_H
#define VDR_STREAMEV_LIVEFILTER_H

#include <vdr/config.h>

#	if VDRVERSNUM >= 10300

#include <vdr/filter.h>

class cStreamdevLiveFilter: public cFilter {
	friend class cStreamdevLiveStreamer;

private:
	cStreamdevLiveStreamer *m_Streamer;

protected:
	virtual void Process(u_short Pid, u_char Tid, const u_char *Data, int Length);

public:
	cStreamdevLiveFilter(cStreamdevLiveStreamer *Streamer);
	virtual ~cStreamdevLiveFilter();
};

#	endif // VDRVERSNUM >= 10300
#endif // VDR_STREAMEV_LIVEFILTER_H
