/*
 *  $Id: server.h,v 1.2 2005/05/09 20:22:29 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVER_H
#define VDR_STREAMDEV_SERVER_H

#include <vdr/thread.h>

#include "server/component.h"
#include "server/connection.h"

#define STREAMDEVHOSTSPATH (*AddDirectory(cPlugin::ConfigDirectory(), "streamdevhosts.conf"))

class cStreamdevServer: public cThread {
private:
	bool m_Active;

	static cStreamdevServer         *m_Instance;
	static cList<cServerComponent>   m_Servers;
	static cList<cServerConnection>  m_Clients;

protected:
	void Stop(void);

	virtual void Action(void);

	static void Register(cServerComponent *Server);

public:
	cStreamdevServer(void);
	virtual ~cStreamdevServer();

	static void Initialize(void);
	static void Destruct(void);
	static bool Active(void);
};

inline bool cStreamdevServer::Active(void) 
{
	return m_Instance != NULL 
	    && m_Instance->m_Clients.Count() > 0;
}

extern cSVDRPhosts StreamdevHosts;

#endif // VDR_STREAMDEV_SERVER_H
