#include <vdr/ringbuffer.h>

#include "server/livestreamer.h"
#include "remux/ts2ps.h"
#include "remux/ts2es.h"
#include "common.h"

cStreamdevLiveReceiver::cStreamdevLiveReceiver(cStreamdevLiveStreamer *Streamer,
		int Ca, int Priority, 
		int Pid1,  int Pid2,  int Pid3,  int Pid4,
		int Pid5,  int Pid6,  int Pid7,  int Pid8,
		int Pid9,  int Pid10, int Pid11, int Pid12,
		int Pid13, int Pid14, int Pid15, int Pid16):
		cReceiver(Ca, Priority, 16, 
			Pid1, Pid2,  Pid3,  Pid4,  Pid5,  Pid6,  Pid7,  Pid8,
			Pid9, Pid10, Pid11, Pid12, Pid13, Pid14, Pid15, Pid16) {
	m_Streamer = Streamer;
}

cStreamdevLiveReceiver::~cStreamdevLiveReceiver() {
	Dprintf("Killing live receiver\n");
	Detach();
}

void cStreamdevLiveReceiver::Receive(uchar *Data, int Length) {
	static time_t firsterr = 0;
	static int errcnt = 0;
	static bool showerr = true;

	int p = m_Streamer->Put(Data, Length);
	if (p != Length) {
		++errcnt;
		if (showerr) {
			if (firsterr == 0)
				firsterr = time_ms();
			else if (firsterr + BUFOVERTIME > time_ms() && errcnt > BUFOVERCOUNT) {
				esyslog("ERROR: too many buffer overflows, logging stopped");
				showerr = false;
				firsterr = time_ms();
			}
		} else if (firsterr + BUFOVERTIME < time_ms()) {
			showerr = true;
			firsterr = 0;
			errcnt = 0;
		}

		if (showerr)
			esyslog("ERROR: ring buffer overflow (%d bytes dropped)", Length - p);
		else
			firsterr = time_ms();
	}
}

cStreamdevLiveStreamer::cStreamdevLiveStreamer(int Priority):
		cStreamdevStreamer("Live streamer") {
	m_Priority   = Priority;
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
	bool haspids = false;
	
	if (On) {
		for (idx = 0; idx < MAXRECEIVEPIDS; ++idx) {
			if (m_Pids[idx] == Pid)
				return true; // No change needed
			else if (m_Pids[idx] == 0) {
				m_Pids[idx] = Pid;
				haspids = true;
				break;
			}
		}

		if (idx == MAXRECEIVEPIDS) {
			esyslog("ERROR: Streamdev: No free slot to receive pid %d\n", Pid);
			return false;
		}
	} else {
		for (idx = 0; idx < MAXRECEIVEPIDS; ++idx) {
			if (m_Pids[idx] == Pid)
				m_Pids[idx] = 0;
			else if (m_Pids[idx] != 0)
				haspids = true;
		}
	}

	DELETENULL(m_Receiver);
	if (haspids) {
		Dprintf("Creating Receiver to respect changed pids\n");
		m_Receiver = new cStreamdevLiveReceiver(this, m_Channel->Ca(), m_Priority, 
				m_Pids[0],  m_Pids[1],  m_Pids[2],  m_Pids[3], 
				m_Pids[4],  m_Pids[5],  m_Pids[6],  m_Pids[7], 
				m_Pids[8],  m_Pids[9],  m_Pids[10], m_Pids[11], 
				m_Pids[12], m_Pids[13], m_Pids[14], m_Pids[15]);
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
			int pid = ISRADIO(Channel) ? Channel->Apid1() : Channel->Vpid();
			m_Remux = new cTS2ESRemux(pid);
			return SetPid(pid, true);
		}

	case stPES: 
		m_Remux = new cTS2PSRemux(Channel->Vpid(), Channel->Apid1(), 
				Channel->Apid2(), Channel->Dpid1(), 0, false);
		return SetPid(Channel->Vpid(),  true)
				&& SetPid(Channel->Apid1(), true)
				&& SetPid(Channel->Apid2(), true)
				&& SetPid(Channel->Dpid1(), true);
		break;

	case stPS:  
		m_Remux = new cTS2PSRemux(Channel->Vpid(), Channel->Apid1(), 0, 0, 0, true);
		return SetPid(Channel->Vpid(),  true)
				&& SetPid(Channel->Apid1(), true);
		break;

	case stTS:
		if (!StreamPIDS) {
			return SetPid(Channel->Vpid(),  true)
					&& SetPid(Channel->Apid1(), true)
					&& SetPid(Channel->Apid2(), true)
					&& SetPid(Channel->Dpid1(), true);
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
