#ifndef VDR_STREAMDEV_LIVESTREAMER_H
#define VDR_STREAMDEV_LIVESTREAMER_H

#include <vdr/config.h>
#include <vdr/receiver.h>

#include "server/streamer.h"
#include "common.h"

#define LIVEBUFSIZE (20000 * TS_SIZE)

namespace Streamdev {
	class cTSRemux;
}
class cStreamdevPatFilter;
class cStreamdevLiveReceiver;

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
	cStreamdevBuffer       *m_ReceiveBuffer;
	cStreamdevPatFilter    *m_PatFilter;
	Streamdev::cTSRemux    *m_Remux;

	void StartReceiver(void);
	bool HasPid(int Pid);

protected:
	virtual uchar* GetFromReceiver(int &Count) { return m_ReceiveBuffer->Get(Count); }
	virtual void DelFromReceiver(int Count) { m_ReceiveBuffer->Del(Count); }

	virtual int Put(const uchar *Data, int Count);

public:
	cStreamdevLiveStreamer(int Priority, const cServerConnection *Connection);
	virtual ~cStreamdevLiveStreamer();

	void SetDevice(cDevice *Device) { m_Device = Device; }
	bool SetPid(int Pid, bool On);
	bool SetPids(int Pid, const int *Pids1 = NULL, const int *Pids2 = NULL, const int *Pids3 = NULL);
	bool SetChannel(const cChannel *Channel, eStreamType StreamType, const int* Apid = NULL, const int* Dpid = NULL);
	void SetPriority(int Priority);
	void GetSignal(int *DevNum, int *Strength, int *Quality) const;
	virtual cString ToText() const;
	
	void Receive(uchar *Data, int Length);
	virtual bool IsReceiving(void) const;

	virtual uchar *Get(int &Count);
	virtual void Del(int Count);

	virtual void Attach(void);
	virtual void Detach(void);

	// Statistical purposes:
	virtual std::string Report(void);
};

#endif // VDR_STREAMDEV_LIVESTREAMER_H
