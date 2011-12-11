/*
 *  $Id: device.h,v 1.10 2010/08/18 10:26:55 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_DEVICE_H
#define VDR_STREAMDEV_DEVICE_H

#include <vdr/device.h>

#include "client/socket.h"
#include "client/filter.h"

class cTBString;

#define CMD_LOCK_OBJ(x) cMutexLock CmdLock((cMutex*)&(x)->m_Mutex)

class cStreamdevDevice: public cDevice {

private:
	const cChannel      *m_Channel;
	cTSBuffer           *m_TSBuffer;
	cStreamdevFilters   *m_Filters;
	int                  m_Pids;
	int                  m_Priority;
	bool                 m_UpdatePriority;
	bool                 m_DvrClosed;

	static cStreamdevDevice *m_Device;
	static const cChannel   *m_DenyChannel;

	bool OpenDvrInt(void);
	void CloseDvrInt(void);

protected:
	virtual bool SetChannelDevice(const cChannel *Channel, bool LiveView);
	virtual bool HasLock(int TimeoutMs) 
	{
		//printf("HasLock is %d\n", (ClientSocket.DataSocket(siLive) != NULL));
		//return ClientSocket.DataSocket(siLive) != NULL;
		return true;
	}

	virtual bool SetPid(cPidHandle *Handle, int Type, bool On);
	virtual bool OpenDvr(void);
	virtual void CloseDvr(void);
	virtual bool GetTSPacket(uchar *&Data);

	virtual int OpenFilter(u_short Pid, u_char Tid, u_char Mask);

public:
	cStreamdevDevice(void);
	virtual ~cStreamdevDevice();

	virtual bool HasInternalCam(void) { return true; }
	virtual bool ProvidesSource(int Source) const;
	virtual bool ProvidesTransponder(const cChannel *Channel) const;
	virtual bool ProvidesChannel(const cChannel *Channel, int Priority = -1,
			bool *NeedsDetachReceivers = NULL) const;
#if APIVERSNUM >= 10700
	virtual int NumProvidedSystems(void) const;
#endif
#if APIVERSNUM >= 10719
	virtual bool AvoidRecording(void) const { return true; }
#endif
#if APIVERSNUM >= 10722
	virtual bool IsTunedToTransponder(const cChannel *Channel) const;
#else
	virtual bool IsTunedToTransponder(const cChannel *Channel);
#endif
	virtual int SignalStrength(void) const;
	virtual int SignalQuality(void) const;

	static void UpdatePriority(void);
	static void DenyChannel(const cChannel *Channel) { m_DenyChannel = Channel; }
	static bool Init(void);
	static bool ReInit(void);

	static cStreamdevDevice *GetDevice(void) { return m_Device; }
};

#endif // VDR_STREAMDEV_DEVICE_H
