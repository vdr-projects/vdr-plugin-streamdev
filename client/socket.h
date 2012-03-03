/*
 *  $Id: socket.h,v 1.8 2010/08/18 10:26:55 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_CLIENT_CONNECTION_H
#define VDR_STREAMDEV_CLIENT_CONNECTION_H

#include <tools/socket.h>

#include "common.h"
#include "client/setup.h"

#include <string>

#define CMD_LOCK cMutexLock CmdLock((cMutex*)&m_Mutex)

class cPES2TSRemux;

class cClientSocket: public cTBSocket {
private:
	cTBSocket    *m_DataSockets[si_Count];
	cMutex        m_Mutex;
	char          m_Buffer[BUFSIZ + 1]; // various uses
	bool          m_Prio; // server supports command PRIO
	bool          m_Abort; // quit command pending

	time_t        m_LastSignalUpdate;
	int           m_LastSignalStrength;
	int           m_LastSignalQuality;
protected:
	/* Send Command, and return true if the command results in Expected. 
	   Returns false on failure. */
	bool Command(const std::string &Command, uint Expected);

	/* Send the given command. Returns false on failure. */
	bool Send(const std::string &Command);

	/* Fetch results from an ongoing Command. The status code and the
	   buffer holding the server's response are stored in Code and Result
	   if non-NULL. Returns false on failure. */
	bool Receive(const std::string &Command, uint *Code = NULL, std::string *Result = NULL, uint TimeoutMs = StreamdevClientSetup.Timeout * 1000);

public:
	cClientSocket(void);
	virtual ~cClientSocket();

	void Reset(void);

	bool CheckConnection(void);
	bool ProvidesChannel(const cChannel *Channel, int Priority);
	bool CreateDataConnection(eSocketId Id);
	bool CloseDataConnection(eSocketId Id);
	bool SetChannelDevice(const cChannel *Channel);
	bool SupportsPrio() { return m_Prio; }
	bool SetPriority(int Priority);
	bool SetPid(int Pid, bool On);
	bool SetFilter(ushort Pid, uchar Tid, uchar Mask, bool On);
	bool GetSignal(int *SignalStrength, int *SignalQuality);
	bool CloseDvr(void);
	bool SuspendServer(void);
	bool Quit(void);

	cTBSocket *DataSocket(eSocketId Id) const;
};

extern class cClientSocket ClientSocket;

#endif // VDR_STREAMDEV_CLIENT_CONNECTION_H
