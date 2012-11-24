/*
 *  $Id: livefilter.c,v 1.7 2009/02/13 13:02:40 schmirl Exp $
 */

#include <vdr/filter.h>

#include "server/livefilter.h"
#include "common.h"

#ifndef TS_SYNC_BYTE
#    define TS_SYNC_BYTE     0x47
#endif


// --- cStreamdevLiveFilter -------------------------------------------------

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

cStreamdevLiveFilter::cStreamdevLiveFilter(cStreamdevStreamer *Streamer) {
	m_Streamer = Streamer;
}

void cStreamdevLiveFilter::Process(u_short Pid, u_char Tid, const u_char *Data, int Length) 
{
	uchar buffer[TS_SIZE];
	int length = Length;
	int pos = 0;

	while (length > 0) {
		int chunk = min(length, TS_SIZE - 5);
		buffer[0] = TS_SYNC_BYTE;
		buffer[1] = ((Pid >> 8) & 0x3f) | (pos==0 ? 0x40 : 0); /* bit 6: payload unit start indicator (PUSI) */
		buffer[2] = Pid & 0xff;
		buffer[3] = Tid;
		// this makes it a proprietary stream
		buffer[4] = (uchar)chunk;
		memcpy(buffer + 5, Data + pos, chunk);
		length -= chunk;
		pos += chunk;

		int p = m_Streamer->Put(buffer, TS_SIZE);
		if (p != TS_SIZE)
			m_Streamer->ReportOverflow(TS_SIZE - p);
	}
}

// --- cStreamdevFilterStreamer -------------------------------------------------

cStreamdevFilterStreamer::cStreamdevFilterStreamer():
		cStreamdevStreamer("streamdev-filterstreaming"),
		m_Device(NULL),
		m_Filter(NULL)/*,
		m_Channel(NULL)*/
{
}

cStreamdevFilterStreamer::~cStreamdevFilterStreamer() 
{
	Dprintf("Desctructing Filter streamer\n");
	Detach();
	m_Device = NULL;
	DELETENULL(m_Filter);
	Stop();
}

void cStreamdevFilterStreamer::Attach(void) 
{ 
	Dprintf("cStreamdevFilterStreamer::Attach()\n");
	LOCK_THREAD;
	if(m_Device && m_Filter)
		m_Device->AttachFilter(m_Filter);
}

void cStreamdevFilterStreamer::Detach(void) 
{ 
	Dprintf("cStreamdevFilterStreamer::Detach()\n");
	LOCK_THREAD;
	if(m_Device && m_Filter)
		m_Device->Detach(m_Filter); 
}

void cStreamdevFilterStreamer::SetDevice(cDevice *Device)
{
	Dprintf("cStreamdevFilterStreamer::SetDevice()\n");
	LOCK_THREAD;
	Detach();
	m_Device = Device;
	Attach();
}

bool cStreamdevFilterStreamer::SetFilter(u_short Pid, u_char Tid, u_char Mask, bool On) 
{	
	Dprintf("cStreamdevFilterStreamer::SetFilter(%u,0x%x,0x%x,%s)\n", Pid, Tid, Mask, On?"On":"Off");

	if(!m_Device)
		return false;

	if (On) {
		if (m_Filter == NULL) {
			m_Filter = new cStreamdevLiveFilter(this);
			Dprintf("attaching filter to device\n");
			Attach();
		}
		m_Filter->Set(Pid, Tid, Mask);
	} else if (m_Filter != NULL) 
		m_Filter->Del(Pid, Tid, Mask);

	return true;
}

