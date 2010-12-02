/*
 *  $Id: device.h,v 1.3 2005/02/08 15:21:19 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_DEVICE_H
#define VDR_STREAMDEV_DEVICE_H

#include <vdr/device.h>

#include "client/socket.h"
#include "client/assembler.h"
#include "client/filter.h"

class cTBString;

#define CMD_LOCK_OBJ(x) cMutexLock CmdLock((cMutex*)&(x)->m_Mutex)

class cStreamdevDevice: public cDevice {
	friend class cRemoteRecordings;

private:
	const cChannel      *m_Channel;
	cTSBuffer           *m_TSBuffer;
	cStreamdevAssembler *m_Assembler;
#if VDRVERSNUM >= 10307
	cStreamdevFilters   *m_Filters;
#endif

	static cStreamdevDevice *m_Device;

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

#if VDRVERSNUM >= 10300
	virtual int OpenFilter(u_short Pid, u_char Tid, u_char Mask);
#endif

public:
	cStreamdevDevice(void);
	virtual ~cStreamdevDevice();

	virtual bool ProvidesSource(int Source) const;
  virtual bool ProvidesTransponder(const cChannel *Channel) const;
	virtual bool ProvidesChannel(const cChannel *Channel, int Priority = -1,
			bool *NeedsDetachReceivers = NULL) const;

	static bool Init(void);
	static bool ReInit(void);

	static cStreamdevDevice *GetDevice(void) { return m_Device; }
};

#endif // VDR_STREAMDEV_DEVICE_H
