#ifndef VDR_STREAMDEV_LIVESTREAMER_H
#define VDR_STREAMDEV_LIVESTREAMER_H

#include <vdr/config.h>
#include <vdr/receiver.h>

#include "server/streamer.h"
#include "server/livefilter.h"
#include "common.h"

class cTS2PSRemux;
class cTS2ESRemux;
class cExternRemux;
class cRemux;

// --- cStreamdevLiveReceiver -------------------------------------------------

class cStreamdevLiveReceiver: public cReceiver {
	friend class cStreamdevLiveStreamer;

private:
	cStreamdevLiveStreamer *m_Streamer;

protected:
	virtual void Activate(bool On);
	virtual void Receive(uchar *Data, int Length);

public:
	cStreamdevLiveReceiver(cStreamdevLiveStreamer *Streamer, int Ca, int Priority, const int *Pids);
	virtual ~cStreamdevLiveReceiver();
};

// --- cStreamdevLiveStreamer -------------------------------------------------

class cStreamdevLiveStreamer: public cStreamdevStreamer {
private:
	int                     m_Priority;
	int                     m_Pids[MAXRECEIVEPIDS + 1];
	int                     m_NumPids;
	eStreamType             m_StreamType;
	const cChannel         *m_Channel;
	cDevice                *m_Device;
	cStreamdevLiveReceiver *m_Receiver;
	cRemux                 *m_PESRemux;
	cTS2ESRemux            *m_ESRemux;
	cTS2PSRemux            *m_PSRemux;
	cExternRemux           *m_ExtRemux;

public:
	cStreamdevLiveStreamer(int Priority);
	virtual ~cStreamdevLiveStreamer();

	void SetDevice(cDevice *Device) { m_Device = Device; }
	bool SetPid(int Pid, bool On);
	bool SetChannel(const cChannel *Channel, eStreamType StreamType, int Apid = 0);
	bool SetFilter(u_short Pid, u_char Tid, u_char Mask, bool On);
	
	virtual int Put(const uchar *Data, int Count);
	virtual uchar *Get(int &Count);
	virtual void Del(int Count);

	virtual void Attach(void);
	virtual void Detach(void);

	// Statistical purposes:
	virtual std::string Report(void);
};

// --- cStreamdevLiveReceiver reverse inlines ---------------------------------

inline void cStreamdevLiveReceiver::Activate(bool On) 
{ 
	Dprintf("LiveReceiver->Activate(%d)\n", On);
	m_Streamer->Activate(On); 
}

#endif // VDR_STREAMDEV_LIVESTREAMER_H
