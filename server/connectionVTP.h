#ifndef VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
#define VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H

#include "server/connection.h"

class cDevice;
class cTBSocket;
class cStreamdevLiveStreamer;

class cConnectionVTP: public cServerConnection {
private:
	cTBSocket              *m_DataSockets[si_Count];
	cStreamdevLiveStreamer *m_LiveStreamer;

	// Members adopted from SVDRP
	cRecordings Recordings;

public:
	cConnectionVTP(void);
	virtual ~cConnectionVTP();

	virtual void Welcome(void);
	virtual void Reject(void);

	virtual void Detach(void);
	virtual void Attach(void);

	bool Command(char *Cmd);
	bool CmdCAPS(char *Opts);
	bool CmdPROV(char *Opts);
	bool CmdPORT(char *Opts);
	bool CmdTUNE(char *Opts);
	bool CmdADDP(char *Opts);
	bool CmdDELP(char *Opts);
	bool CmdADDF(char *Opts);
	bool CmdDELF(char *Opts);
	bool CmdABRT(char *Opts);
	bool CmdQUIT(char *Opts);
	bool CmdSUSP(char *Opts);

	// Commands adopted from SVDRP
	bool ReplyWrapper(int Code, const char *fmt, ...);
	bool CmdLSTE(char *Opts);
	bool CmdLSTR(char *Opts);
	bool CmdDELR(char *Opts);
	bool CmdLSTT(char *Opts);
	bool CmdMODT(char *Opts);
	bool CmdNEWT(char *Opts);
	bool CmdDELT(char *Opts);

	bool Respond(int Code, const std::string &Message);
};

#endif // VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
