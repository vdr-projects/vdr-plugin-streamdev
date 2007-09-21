#ifndef VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
#define VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H

#include "server/connection.h"

class cTBSocket;
class cStreamdevLiveStreamer;
class cStreamdevFilterStreamer;
class cLSTEHandler;
class cLSTCHandler;
class cLSTTHandler;

class cConnectionVTP: public cServerConnection {
	friend class cLSTEHandler;
	// if your compiler doesn't understand the following statement
	// (e.g. gcc 2.x), simply remove it and try again ;-)
	using cServerConnection::Respond;

private:
	cTBSocket                *m_LiveSocket;
	cStreamdevLiveStreamer   *m_LiveStreamer;
	cTBSocket                *m_FilterSocket;
	cStreamdevFilterStreamer *m_FilterStreamer;

	char                   *m_LastCommand;
	eStreamType             m_StreamType;

	// Members adopted for SVDRP
	cRecordings Recordings;
	cLSTEHandler *m_LSTEHandler;
	cLSTCHandler *m_LSTCHandler;
	cLSTTHandler *m_LSTTHandler;

protected:
	template<class cHandler>
	bool CmdLSTX(cHandler *&Handler, char *Option);

public:
	cConnectionVTP(void);
	virtual ~cConnectionVTP();

	virtual void Welcome(void);
	virtual void Reject(void);

	virtual bool Abort(void) const;
	virtual void Detach(void);
	virtual void Attach(void);

	virtual bool Command(char *Cmd);
	bool CmdCAPS(char *Opts);
	bool CmdPROV(char *Opts);
	bool CmdPORT(char *Opts);
	bool CmdTUNE(char *Opts);
	bool CmdADDP(char *Opts);
	bool CmdDELP(char *Opts);
	bool CmdADDF(char *Opts);
	bool CmdDELF(char *Opts);
	bool CmdABRT(char *Opts);
	bool CmdQUIT(void);
	bool CmdSUSP(void);

	// Thread-safe implementations of SVDRP commands
	bool CmdLSTE(char *Opts);
	bool CmdLSTC(char *Opts);
	bool CmdLSTT(char *Opts);

	// Commands adopted from SVDRP
	bool CmdMODT(const char *Option);
	bool CmdNEWT(const char *Option);
	bool CmdDELT(const char *Option);

	//bool CmdLSTR(char *Opts);
	//bool CmdDELR(char *Opts);

	bool Respond(int Code, const char *Message, ...)
			__attribute__ ((format (printf, 3, 4)));
};

#endif // VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
