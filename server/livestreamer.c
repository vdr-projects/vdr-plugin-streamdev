#include <vdr/ringbuffer.h>

#include "server/livestreamer.h"
#include "remux/ts2ps.h"
#include "remux/ts2es.h"
#include "common.h"

cStreamdevLiveReceiver::cStreamdevLiveReceiver(cStreamdevLiveStreamer *Streamer, int Ca, 
                                               int Priority, const int *Pids):
		cReceiver(Ca, Priority, 0, Pids),
		m_Streamer(Streamer)
{
}

cStreamdevLiveReceiver::~cStreamdevLiveReceiver() 
{
	Dprintf("Killing live receiver\n");
	Detach();
}

void cStreamdevLiveReceiver::Receive(uchar *Data, int Length) {
	int p = m_Streamer->Put(Data, Length);
	if (p != Length)
		m_Streamer->ReportOverflow(Length - p);
}

cStreamdevLiveStreamer::cStreamdevLiveStreamer(int Priority):
		cStreamdevStreamer("Live streamer") {
	m_Priority   = Priority;
	m_NumPids    = 0;
	m_Channel    = NULL;
	m_Device     = NULL;
	m_Receiver   = NULL;
	m_Remux      = NULL;
	m_Buffer     = NULL;
	m_Sequence   = 0;
#if VDRVERSNUM >= 10300
	m_Filter     = NULL;
#endif
	memset(m_Pids, 0, sizeof(m_Pids));
}

cStreamdevLiveStreamer::~cStreamdevLiveStreamer() {
	Dprintf("Desctructing Live streamer\n");
	delete m_Receiver;
	delete m_Remux;
#if VDRVERSNUM >= 10300
	delete m_Filter;
#endif
	free(m_Buffer);
}

void cStreamdevLiveStreamer::Detach(void) {
	m_Device->Detach(m_Receiver);
}

void cStreamdevLiveStreamer::Attach(void) {
	m_Device->AttachReceiver(m_Receiver);
}

void cStreamdevLiveStreamer::Start(cTBSocket *Socket) {
	Dprintf("LIVESTREAMER START\n");
	cStreamdevStreamer::Start(Socket);
}

bool cStreamdevLiveStreamer::SetPid(int Pid, bool On) {
	int idx;
	
	if (On) {
		for (idx = 0; idx < m_NumPids; ++idx) {
			if (m_Pids[idx] == Pid)
				return true; // No change needed
		}

		if (m_NumPids == MAXRECEIVEPIDS) {
			esyslog("ERROR: Streamdev: No free slot to receive pid %d\n", Pid);
			return false;
		}

		m_Pids[m_NumPids++] = Pid;
		m_Pids[m_NumPids] = 0;
	} else {
		for (idx = 0; idx < m_NumPids; ++idx) {
			if (m_Pids[idx] == Pid) {
				--m_NumPids;
				memmove(&m_Pids[idx], &m_Pids[idx + 1], sizeof(int) * (m_NumPids - idx));
			}
		}
	}

	DELETENULL(m_Receiver);
	if (m_NumPids > 0) {
		Dprintf("Creating Receiver to respect changed pids\n");
		m_Receiver = new cStreamdevLiveReceiver(this, m_Channel->Ca(), m_Priority, m_Pids);
		if (m_Device != NULL) {
			Dprintf("Attaching new receiver\n");
			m_Device->AttachReceiver(m_Receiver);
		}
	}
	return true;
}

bool cStreamdevLiveStreamer::SetChannel(const cChannel *Channel, int StreamType,
		bool StreamPIDS) {
	Dprintf("Initializing Remuxer for full channel transfer\n");
	printf("ca pid: %d\n", Channel->Ca());
	m_Channel = Channel;
	switch (StreamType) {
	case stES: 
		{
			int pid = ISRADIO(Channel) ? Channel->Apid(0) : Channel->Vpid();
			m_Remux = new cTS2ESRemux(pid);
			return SetPid(pid, true);
		}

	case stPES: 
		m_Remux = new cTS2PSRemux(Channel->Vpid(), Channel->Apid(0), Channel->Apid(1), 
		                          Channel->Dpid(0), 0, false);
		return SetPid(Channel->Vpid(),  true)
		    && SetPid(Channel->Apid(0), true)
		    && SetPid(Channel->Apid(1), true)
		    && SetPid(Channel->Dpid(0), true);
		break;

	case stPS:  
		m_Remux = new cTS2PSRemux(Channel->Vpid(), Channel->Apid(0), 0, 0, 0, true);
		return SetPid(Channel->Vpid(),  true)
		    && SetPid(Channel->Apid(0), true);
		break;

	case stTS:
		if (!StreamPIDS) {
			return SetPid(Channel->Vpid(),  true)
			    && SetPid(Channel->Apid(0), true)
			    && SetPid(Channel->Apid(1), true)
			    && SetPid(Channel->Dpid(0), true);
		}
		Dprintf("pid streaming mode\n");
		return true;
		break;
	}
	return false;
}

bool cStreamdevLiveStreamer::SetFilter(u_short Pid, u_char Tid, u_char Mask, 
		bool On) {
#if VDRVERSNUM >= 10300
	Dprintf("setting filter\n");
	if (On) {
		if (m_Filter == NULL) {
			m_Filter = new cStreamdevLiveFilter(this);
			Dprintf("attaching filter to device\n");
			m_Device->AttachFilter(m_Filter);
		}
		m_Filter->Set(Pid, Tid, Mask);
	} else if (m_Filter != NULL)
		m_Filter->Del(Pid, Tid, Mask);
	return true;
#else
	return false;
#endif
}

uchar *cStreamdevLiveStreamer::Process(const uchar *Data, int &Count,
		int &Result) {
	uchar *remuxed = m_Remux != NULL ? m_Remux->Process(Data, Count, Result)
			: cStreamdevStreamer::Process(Data, Count, Result);
	if (remuxed) {
		/*if (Socket()->Type() == SOCK_DGRAM) {
			free(m_Buffer);
			Result += 12;
			m_Buffer = MALLOC(uchar, Result);
			m_Buffer[0] = 0x01;
			m_Buffer[1] = 0x02;
			m_Buffer[2] = 0x03;
			m_Buffer[3] = 0x04;
			m_Buffer[4] = (Result & 0xff000000) >> 24;
			m_Buffer[5] = (Result & 0xff0000) >> 16;
			m_Buffer[6] = (Result & 0xff00) >> 8;
			m_Buffer[7] = (Result & 0xff);
			m_Buffer[8] = (m_Sequence & 0xff000000) >> 24;
			m_Buffer[9] = (m_Sequence & 0xff0000) >> 16;
			m_Buffer[10] = (m_Sequence & 0xff00) >> 8;
			m_Buffer[11] = (m_Sequence & 0xff);
			memcpy(m_Buffer + 12, Data, Result - 12);
			if (m_Sequence++ == 0x7fffffff)
				m_Sequence = 0;
			return m_Buffer;
		}*/
		return remuxed;
	}
	return NULL;
}

cTBString cStreamdevLiveStreamer::Report(void) {
	cTBString result;

	if (m_Device != NULL)
		result += "+- Device is " + cTBString::Number(m_Device->CardIndex()) + "\n";
	if (m_Receiver != NULL)
		result += "+- Receiver is allocated\n";
		
	result += "+- Pids are ";
	for (int i = 0; i < MAXRECEIVEPIDS; ++i) 
		if (m_Pids[i] != 0)
			result += cTBString::Number(m_Pids[i]) + ", ";
	result += "\n";
	return result;
}
