/*
 *  $Id: connectionHTTP.h,v 1.6 2008/10/14 11:05:48 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H
#define VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H

#include "connection.h"
#include "server/livestreamer.h"

#include <tools/select.h>

class cChannel;
class cStreamdevLiveStreamer;
class cChannelList;

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
	std::string                       m_Host;
	std::string                       m_Authorization;
	//std::map<std::string,std::string> m_Headers; TODO: later?
	eHTTPStatus                       m_Status;
	eHTTPJob                          m_Job;
	// job: transfer
	cStreamdevLiveStreamer           *m_LiveStreamer;
	std::string                       m_StreamerParameter;
	const cChannel                   *m_Channel;
	int                               m_Apid;
	eStreamType                       m_StreamType;
	// job: listing
	cChannelList                     *m_ChannelList;

protected:
	bool ProcessRequest(void);

public:
	cConnectionHTTP(void);
	virtual ~cConnectionHTTP();

	virtual void Attach(void) { if (m_LiveStreamer != NULL) m_LiveStreamer->Attach(); }
	virtual void Detach(void) { if (m_LiveStreamer != NULL) m_LiveStreamer->Detach(); }

	virtual bool CanAuthenticate(void);

	virtual bool Command(char *Cmd);
	bool CmdGET(const std::string &Opts);

	virtual bool Abort(void) const;
	virtual void Flushed(void);
};

inline bool cConnectionHTTP::Abort(void) const
{
	return m_LiveStreamer && m_LiveStreamer->Abort();
}

#endif // VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
