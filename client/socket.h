/*
 *  $Id: socket.h,v 1.3 2005/02/08 17:22:35 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_CLIENT_CONNECTION_H
#define VDR_STREAMDEV_CLIENT_CONNECTION_H

#include <tools/socket.h>

#include "common.h"

#include <string>

#define CMD_LOCK cMutexLock CmdLock((cMutex*)&m_Mutex)

class cRemoteRecordings;
class cRemoteRecording;
class cRemoteTimers;
class cRemoteTimer;
class cPES2TSRemux;

class cClientSocket: public cTBSocket {
private:
	cTBSocket    *m_DataSockets[si_Count];
	cMutex        m_Mutex;
	char          m_Buffer[BUFSIZ + 1]; // various uses

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
	bool SetChannelDevice(const cChannel *Channel);
	bool SetPid(int Pid, bool On);
#if VDRVERSNUM >= 10300
	bool SetFilter(ushort Pid, uchar Tid, uchar Mask, bool On);
#endif
	bool CloseDvr(void);
	bool SynchronizeEPG(void);
	bool LoadRecordings(cRemoteRecordings &Recordings);
	bool StartReplay(const char *Filename);
	bool AbortReplay(void);
	bool DeleteRecording(cRemoteRecording *Recording);
	bool LoadTimers(cRemoteTimers &Timers);
	bool SaveTimer(cRemoteTimer *Old, cRemoteTimer &New);
	bool DeleteTimer(cRemoteTimer *Timer);
	bool SuspendServer(void);
	bool Quit(void);

	cTBSocket *DataSocket(eSocketId Id) const;
};

extern class cClientSocket ClientSocket;

#endif // VDR_STREAMDEV_CLIENT_CONNECTION_H
