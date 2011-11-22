/*
 *  $Id: connectionHTTP.h,v 1.7 2010/07/19 13:49:31 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H
#define VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H

#include "connection.h"
#include "server/livestreamer.h"

#include <map>
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

	std::string                       m_Authorization;
	eHTTPStatus                       m_Status;
	// job: transfer
	cStreamdevLiveStreamer           *m_LiveStreamer;
	const cChannel                   *m_Channel;
	int                               m_Apid[2];
	int                               m_Dpid[2];
	eStreamType                       m_StreamType;
	// job: listing
	cChannelList                     *m_ChannelList;

	cChannelList* ChannelListFromString(const std::string &PathInfo, const std::string &Filebase, const std::string &Fileext) const;
	bool ProcessURI(const std::string &PathInfo);
protected:
	bool ProcessRequest(void);

public:
	cConnectionHTTP(void);
	virtual ~cConnectionHTTP();

	virtual void Attach(void) { if (m_LiveStreamer != NULL) m_LiveStreamer->Attach(); }
	virtual void Detach(void) { if (m_LiveStreamer != NULL) m_LiveStreamer->Detach(); }

	virtual cString ToText() const;

	virtual bool CanAuthenticate(void);

	virtual bool Command(char *Cmd);

	virtual bool Abort(void) const;
	virtual void Flushed(void);
};

inline bool cConnectionHTTP::Abort(void) const
{
	return !IsOpen() || (m_LiveStreamer && m_LiveStreamer->Abort());
}

#endif // VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
