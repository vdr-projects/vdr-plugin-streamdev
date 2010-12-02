/*
 *  $Id: filter.h,v 1.1.1.1 2004/12/30 22:44:04 lordjaxom Exp $
 */

#ifndef VDR_STREAMDEV_FILTER_H
#define VDR_STREAMDEV_FILTER_H

#include <vdr/config.h>

#	if VDRVERSNUM >= 10300

#include <vdr/tools.h>
#include <vdr/thread.h>

class cRingBufferFrame;
class cRingBufferLinear;

class cStreamdevFilter: public cListObject {
private:
	uchar              m_Buffer[4096];
	int                m_Used;
	int                m_Pipe[2];
	u_short            m_Pid;
	u_char             m_Tid;
	u_char             m_Mask;
	cRingBufferFrame  *m_RingBuffer;

public:
	cStreamdevFilter(u_short Pid, u_char Tid, u_char Mask);
	virtual ~cStreamdevFilter();

	bool Matches(u_short Pid, u_char Tid);
	bool PutSection(const uchar *Data, int Length);
	int ReadPipe(void) const { return m_Pipe[0]; }

	u_short Pid(void) const { return m_Pid; }
	u_char Tid(void) const { return m_Tid; }
	u_char Mask(void) const { return m_Mask; }

};

inline bool cStreamdevFilter::Matches(u_short Pid, u_char Tid) {
	return m_Pid == Pid && m_Tid == (Tid & m_Mask);
}

class cStreamdevFilters: public cList<cStreamdevFilter>, public cThread {
private:
	bool               m_Active;
	cRingBufferLinear *m_RingBuffer;

protected:
	virtual void Action(void);

public:
	cStreamdevFilters(void);
	virtual ~cStreamdevFilters();

	int OpenFilter(u_short Pid, u_char Tid, u_char Mask);
	cStreamdevFilter *Matches(u_short Pid, u_char Tid);
	void Put(const uchar *Data);
};

#	endif // VDRVERSNUM >= 10300
#endif // VDR_STREAMDEV_FILTER_H
