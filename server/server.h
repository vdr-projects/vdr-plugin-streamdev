/*
 *  $Id: server.h,v 1.3 2008/04/07 14:50:33 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVER_H
#define VDR_STREAMDEV_SERVER_H

#include <vdr/thread.h>

#include "server/component.h"
#include "server/connection.h"

#define EXTERNREMUXPATH (*AddDirectory(cPlugin::ConfigDirectory(PLUGIN_NAME_I18N), "externremux.sh"))
#define STREAMDEVHOSTSPATH (*AddDirectory(cPlugin::ConfigDirectory(PLUGIN_NAME_I18N), "streamdevhosts.conf"))

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
