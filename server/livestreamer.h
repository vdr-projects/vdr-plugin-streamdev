#ifndef VDR_STREAMDEV_LIVESTREAMER_H
#define VDR_STREAMDEV_LIVESTREAMER_H

#include <vdr/config.h>
#include <vdr/receiver.h>

#include "server/streamer.h"
#include "server/livefilter.h"
#include "common.h"

#if MAXRECEIVEPIDS < 16
#	error Too few receiver pids allowed! Please contact sascha@akv-soft.de!
#endif

class cTSRemux;
class cRemux;

class cStreamdevLiveReceiver: public cReceiver {
	friend class cStreamdevLiveStreamer;

private:
	cStreamdevLiveStreamer *m_Streamer;

protected:
	virtual void Activate(bool On);
	virtual void Receive(uchar *Data, int Length);

public:
	cStreamdevLiveReceiver(cStreamdevLiveStreamer *Streamer, int Ca, int Priority,
	                       const int *Pids);
	virtual ~cStreamdevLiveReceiver();
};

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
	cTSRemux               *m_Remux;
	uchar                  *m_Buffer;

public:
	cStreamdevLiveStreamer(int Priority);
	virtual ~cStreamdevLiveStreamer();

	void SetDevice(cDevice *Device) { m_Device = Device; }
	bool SetPid(int Pid, bool On);
	bool SetChannel(const cChannel *Channel, eStreamType StreamType);
	bool SetFilter(u_short Pid, u_char Tid, u_char Mask, bool On);
	
	virtual int Put(const uchar *Data, int Count);
	virtual uchar *Get(int &Count);
	virtual void Del(int Count);

	virtual void Detach(void);
	virtual void Attach(void);

	virtual void Start(cTBSocket *Socket);
	
	// Statistical purposes:
	virtual std::string Report(void);
};

#endif // VDR_STREAMDEV_LIVESTREAMER_H
