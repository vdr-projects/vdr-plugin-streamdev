#include <vdr/ringbuffer.h>

#include "server/livestreamer.h"
#include "remux/ts2ps.h"
#include "remux/ts2es.h"
#include "common.h"

// --- cStreamdevLiveReceiver -------------------------------------------------

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
	int p = m_Streamer->Receive(Data, Length);
	if (p != Length)
		m_Streamer->ReportOverflow(Length - p);
}

// --- cStreamdevLiveStreamer -------------------------------------------------

cStreamdevLiveStreamer::cStreamdevLiveStreamer(int Priority):
		cStreamdevStreamer("streamdev-livestreaming"),
		m_Priority(Priority),
		m_NumPids(0),
		m_StreamType(stTSPIDS),
		m_Channel(NULL),
		m_Device(NULL),
		m_Receiver(NULL),
		m_PESRemux(NULL),
		m_Remux(NULL)
{
}

cStreamdevLiveStreamer::~cStreamdevLiveStreamer() 
{
	Dprintf("Desctructing Live streamer\n");
	delete m_Receiver;
	delete m_Remux;
#if VDRVERSNUM >= 10300
	//delete m_Filter; TODO
#endif
}

bool cStreamdevLiveStreamer::SetPid(int Pid, bool On) 
{
	int idx;

	if (Pid == 0)
		return true;
	
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
			Attach();
		}
	}
	return true;
}

bool cStreamdevLiveStreamer::SetChannel(const cChannel *Channel, eStreamType StreamType, int Apid) 
{
	Dprintf("Initializing Remuxer for full channel transfer\n");
	printf("ca pid: %d\n", Channel->Ca());
	m_Channel = Channel;
	m_StreamType = StreamType;
	switch (m_StreamType) {
	case stES: 
		{
			int pid = ISRADIO(m_Channel) ? m_Channel->Apid(0) : m_Channel->Vpid();
			if (Apid != 0)
				pid = Apid;
			m_Remux = new cTS2ESRemux(pid);
			return SetPid(pid, true);
		}

	case stPES: 
		Dprintf("PES\n");
		m_PESRemux = new cRemux(m_Channel->Vpid(), m_Channel->Apids(), m_Channel->Dpids(), 
								m_Channel->Spids(), false);
		if (Apid != 0)
			return SetPid(m_Channel->Vpid(),  true)
			    && SetPid(Apid, true);
		else
			return SetPid(m_Channel->Vpid(),  true)
			    && SetPid(m_Channel->Apid(0), true)
			    && SetPid(m_Channel->Dpid(0), true);

	case stPS:  
		m_Remux = new cTS2PSRemux(m_Channel->Vpid(), m_Channel->Apid(0), 0, 0, 0, true);
		return SetPid(m_Channel->Vpid(),  true)
		    && SetPid(m_Channel->Apid(0), true);

	case stTS:
		if (Apid != 0)
			return SetPid(m_Channel->Vpid(),  true)
			    && SetPid(Apid, true);
		else
			return SetPid(m_Channel->Vpid(),  true)
			    && SetPid(m_Channel->Apid(0), true)
			    && SetPid(m_Channel->Dpid(0), true);

	case stTSPIDS:
		Dprintf("pid streaming mode\n");
		return true;
	}
	return false;
}

bool cStreamdevLiveStreamer::SetFilter(u_short Pid, u_char Tid, u_char Mask, bool On) 
{
#if 0
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

int cStreamdevLiveStreamer::Put(const uchar *Data, int Count) 
{
	switch (m_StreamType) {
	case stTS:
	case stTSPIDS:
		return cStreamdevStreamer::Put(Data, Count);

	case stPES:
		return m_PESRemux->Put(Data, Count);

	default:
		abort();
	}
}

uchar *cStreamdevLiveStreamer::Get(int &Count)
{
	switch (m_StreamType) {
	case stTS:
	case stTSPIDS:
		return cStreamdevStreamer::Get(Count);

	case stPES:
		return m_PESRemux->Get(Count);

	default:
		abort();
	}
}

void cStreamdevLiveStreamer::Del(int Count)
{
	switch (m_StreamType) {
	case stTS:
	case stTSPIDS:
		cStreamdevStreamer::Del(Count);
		break;

	case stPES:
		m_PESRemux->Del(Count);
		break;

	default:
		abort();
	}
}

std::string cStreamdevLiveStreamer::Report(void) 
{
	std::string result;

	if (m_Device != NULL)
		result += (std::string)"+- Device is " + (const char*)itoa(m_Device->CardIndex()) + "\n";
	if (m_Receiver != NULL)
		result += "+- Receiver is allocated\n";
		
	result += "+- Pids are ";
	for (int i = 0; i < MAXRECEIVEPIDS; ++i) 
		if (m_Pids[i] != 0)
			result += (std::string)(const char*)itoa(m_Pids[i]) + ", ";
	result += "\n";
	return result;
}
