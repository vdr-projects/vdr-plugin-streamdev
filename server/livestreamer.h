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

class cStreamdevLiveReceiver: public cReceiver {
	friend class cStreamdevLiveStreamer;

private:
	cStreamdevLiveStreamer *m_Streamer;

protected:
	virtual void Receive(uchar *Data, int Length);

public:
	cStreamdevLiveReceiver(cStreamdevLiveStreamer *Streamer, int Priority, int Ca,
			int Pid1  = 0, int Pid2  = 0, int Pid3  = 0, int Pid4  = 0,
			int Pid5  = 0, int Pid6  = 0, int Pid7  = 0, int Pid8  = 0,
			int Pid9  = 0, int Pid10 = 0, int Pid11 = 0, int Pid12 = 0,
			int Pid13 = 0, int Pid14 = 0, int Pid15 = 0, int Pid16 = 0);
	virtual ~cStreamdevLiveReceiver();
};

class cStreamdevLiveStreamer: public cStreamdevStreamer {
private:
	int                     m_Priority;
	int                     m_Pids[MAXRECEIVEPIDS];
	const cChannel         *m_Channel;
	cDevice                *m_Device;
	cStreamdevLiveReceiver *m_Receiver;
	cTSRemux               *m_Remux;
	uchar                  *m_Buffer;
	int                     m_Sequence;
#if VDRVERSNUM >= 10300
	cStreamdevLiveFilter   *m_Filter;
#endif

protected:
	virtual uchar *Process(const uchar *Data, int &Count, int &Result);

public:
	cStreamdevLiveStreamer(int Priority);
	virtual ~cStreamdevLiveStreamer();

	void SetDevice(cDevice *Device) { m_Device = Device; }
	bool SetPid(int Pid, bool On);
	bool SetChannel(const cChannel *Channel, int StreamType, bool StreamPIDS);
	bool SetFilter(u_short Pid, u_char Tid, u_char Mask, bool On);
	
	virtual void Detach(void);
	virtual void Attach(void);

	virtual void Start(cTBSocket *Socket);
	
	// Statistical purposes:
	virtual cTBString Report(void);
};

#endif // VDR_STREAMDEV_LIVESTREAMER_H
