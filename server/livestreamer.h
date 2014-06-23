#ifndef VDR_STREAMDEV_LIVESTREAMER_H
#define VDR_STREAMDEV_LIVESTREAMER_H

#include <vdr/config.h>
#include <vdr/receiver.h>

#include "server/streamer.h"
#include "server/streamdev-server.h"
#include "common.h"

#define LIVEBUFSIZE (20000 * TS_SIZE)

namespace Streamdev {
	class cTSRemux;
}
class cStreamdevPatFilter;
class cStreamdevLiveReceiver;

// --- cStreamdevLiveStreamer -------------------------------------------------

class cStreamdevLiveStreamer: public cStreamdevStreamer, public cMainThreadHookSubscriber {
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
	bool                    m_SwitchLive;

	void StartReceiver(bool Force = false);
	bool HasPid(int Pid);

	/* Test if device is in use as the transfer mode receiver device
	   or a FF card, displaying live TV from internal tuner */
	static bool UsedByLiveTV(cDevice *device);

	/* Find a suitable device and tune it to the requested channel. */
	cDevice *SwitchDevice(const cChannel *Channel, int Priority);

	bool SetChannel(const int* Apid = NULL, const int* Dpid = NULL);

protected:
	virtual uchar* GetFromReceiver(int &Count) { return m_ReceiveBuffer->Get(Count); }
	virtual void DelFromReceiver(int Count) { m_ReceiveBuffer->Del(Count); }

	virtual int Put(const uchar *Data, int Count);
	virtual void Action(void);

public:
	cStreamdevLiveStreamer(const cServerConnection *Connection, const cChannel *Channel, int Priority, eStreamType StreamType, const int* Apid = NULL, const int* Dpid = NULL);
	virtual ~cStreamdevLiveStreamer();

	bool SetPid(int Pid, bool On);
	bool SetPids(int Pid, const int *Pids1 = NULL, const int *Pids2 = NULL, const int *Pids3 = NULL);
	void SetPriority(int Priority);
	void GetSignal(int *DevNum, int *Strength, int *Quality) const;
	virtual cString ToText() const;
	
	void Receive(uchar *Data, int Length);
	virtual bool IsReceiving(void) const;

	virtual uchar *Get(int &Count);
	virtual void Del(int Count);

	virtual void Attach(void);
	virtual void Detach(void);

	cDevice *GetDevice() const { return m_Device; }

	/* Test if a call to GetDevice would return a usable device. */
	static bool ProvidesChannel(const cChannel *Channel, int Priority);

	/* Do things which must be done in VDR's main loop */
	void MainThreadHook();

	// Statistical purposes:
	virtual std::string Report(void);
};

#endif // VDR_STREAMDEV_LIVESTREAMER_H
