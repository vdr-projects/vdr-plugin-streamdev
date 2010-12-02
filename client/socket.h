/*
 *  $Id: socket.h,v 1.8 2010/08/18 10:26:55 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_CLIENT_CONNECTION_H
#define VDR_STREAMDEV_CLIENT_CONNECTION_H

#include <tools/socket.h>

#include "common.h"

#include <string>

#define CMD_LOCK cMutexLock CmdLock((cMutex*)&m_Mutex)

class cPES2TSRemux;

class cClientSocket: public cTBSocket {
private:
	cTBSocket    *m_DataSockets[si_Count];
	cMutex        m_Mutex;
	char          m_Buffer[BUFSIZ + 1]; // various uses
	bool          m_Prio; // server supports command PRIO

protected:
	/* Send Command, and return true if the command results in Expected. 
	   Returns false on failure, setting errno appropriately if it has been
	   a system failure. If Expected is zero, returns immediately after
		 sending the command. */
	bool Command(const std::string &Command, uint Expected = 0, uint TimeoutMs = 1500);

	/* Fetch results from an ongoing Command called with Expected == 0. Returns
	   true if the response has the code Expected, returning an internal buffer
		 in the array pointer pointed to by Result. Returns false on failure, 
		 setting errno appropriately if it has been a system failure. */
	bool Expect(uint Expected, std::string *Result = NULL, uint TimeoutMs = 1500);

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
	bool CloseDvr(void);
	bool SuspendServer(void);
	bool Quit(void);

	cTBSocket *DataSocket(eSocketId Id) const;
};

extern class cClientSocket ClientSocket;

#endif // VDR_STREAMDEV_CLIENT_CONNECTION_H
