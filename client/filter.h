/*
 *  $Id: filter.h,v 1.4 2007/04/24 11:23:16 schmirl Exp $
 */

#ifndef VDR_STREAMDEV_FILTER_H
#define VDR_STREAMDEV_FILTER_H

#include <vdr/config.h>

#	if VDRVERSNUM >= 10300

#include <vdr/tools.h>
#include <vdr/thread.h>

class cTSBuffer;
class cStreamdevFilter;

class cStreamdevFilters: public cList<cStreamdevFilter>, public cThread {
private:
	cTSBuffer         *m_TSBuffer;
	
protected:
	virtual void Action(void);
	void CarbageCollect(void);

	bool ReActivateFilters(void);

public:
	cStreamdevFilters(void);
	virtual ~cStreamdevFilters();

	void SetConnection(int Handle);
	int OpenFilter(u_short Pid, u_char Tid, u_char Mask);
};

#	endif // VDRVERSNUM >= 10300
#endif // VDR_STREAMDEV_FILTER_H
