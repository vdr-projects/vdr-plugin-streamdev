/*
 *  $Id: connectionHTTP.h,v 1.3 2005/02/11 16:44:15 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H
#define VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H

#include "connection.h"
#include "server/livestreamer.h"

#include <tools/select.h>

class cChannel;
class cStreamdevLiveStreamer;

class cConnectionHTTP: public cServerConnection {
private:
	enum eHTTPStatus {
		hsRequest,
		hsHeaders,
		hsBody,
		hsFinished,
	};

	enum eHTTPJob {
		hjTransfer,
		hjListing,
	};

	std::string                       m_Request;
	//std::map<std::string,std::string> m_Headers; TODO: later?
	eHTTPStatus                       m_Status;
	eHTTPJob                          m_Job;
	// job: transfer
	cStreamdevLiveStreamer           *m_LiveStreamer;
	const cChannel                   *m_Channel;
	int                               m_Apid;
	eStreamType                       m_StreamType;
	// job: listing
	const cChannel                   *m_ListChannel;

protected:
	bool ProcessRequest(void);

public:
	cConnectionHTTP(void);
	virtual ~cConnectionHTTP();

	virtual void Attach(void) { if (m_LiveStreamer != NULL) m_LiveStreamer->Attach(); }
	virtual void Detach(void) { if (m_LiveStreamer != NULL) m_LiveStreamer->Detach(); }

	virtual bool Command(char *Cmd);
	bool CmdGET(const std::string &Opts);

	virtual void Flushed(void);
};

#endif // VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
