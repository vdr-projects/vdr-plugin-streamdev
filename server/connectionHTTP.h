/*
 *  $Id: connectionHTTP.h,v 1.1 2004/12/30 22:44:18 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H
#define VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H

#include "connection.h"

#include <tools/select.h>

class cChannel;
class cStreamdevLiveStreamer;

class cConnectionHTTP: public cServerConnection {
private:
	enum eHTTPStatus {
		hsRequest,
		hsHeaders,
		hsTransfer,
		hsListing,
	};

	cChannel               *m_Channel;
	cChannel               *m_ListChannel;
	cStreamdevLiveStreamer *m_LiveStreamer;
	eStreamType             m_StreamType;
	eHTTPStatus             m_Status;
	bool                    m_Startup;

public:
	cConnectionHTTP(void);
	virtual ~cConnectionHTTP();

	virtual void Detach(void);
	virtual void Attach(void);

	virtual bool Command(char *Cmd);
	bool CmdGET(char *Opts);

	virtual void Flushed(void);
};

#endif // VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
