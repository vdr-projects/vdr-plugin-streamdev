/*
 *  $Id: connectionHTTP.h,v 1.2 2005/02/10 22:24:26 lordjaxom Exp $
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

	const cChannel         *m_Channel;
	int                     m_Apid;
	const cChannel         *m_ListChannel;
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
