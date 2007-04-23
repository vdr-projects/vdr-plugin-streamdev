/*
 *  $Id: filter.h,v 1.3 2007/04/23 12:52:28 schmirl Exp $
 */

#ifndef VDR_STREAMDEV_FILTER_H
#define VDR_STREAMDEV_FILTER_H

#include <vdr/config.h>

#	if VDRVERSNUM >= 10300

#include <vdr/tools.h>
#include <vdr/thread.h>

class cRingBufferLinear;
class cTSBuffer;
class cStreamdevFilter;

class cStreamdevFilters: public cList<cStreamdevFilter>, public cThread {
private:
	bool               m_Active;
	cRingBufferLinear *m_RingBuffer;

protected:
	virtual void Action(void);
	void CarbageCollect(void);

public:
	cStreamdevFilters(void);
	virtual ~cStreamdevFilters();

	int OpenFilter(u_short Pid, u_char Tid, u_char Mask);
	cStreamdevFilter *Matches(u_short Pid, u_char Tid);
	void Put(const uchar *Data);
};

#	endif // VDRVERSNUM >= 10300
#endif // VDR_STREAMDEV_FILTER_H
