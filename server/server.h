/*
 *  $Id: server.h,v 1.1 2004/12/30 22:44:21 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVER_H
#define VDR_STREAMDEV_SERVER_H

#include <vdr/thread.h>

#include "server/component.h"
#include "server/connection.h"

class cStreamdevServer: public cThread {
private:
	bool m_Active;

	cServerComponents m_Servers;
	cServerConnections m_Clients;

	static cStreamdevServer *m_Instance;

protected:
	virtual void Action(void);

	void Stop(void);

public:
	cStreamdevServer(void);
	virtual ~cStreamdevServer();

	void Register(cServerComponent *Server);
	
	static void Init(void);
	static void Exit(void);
	static bool Active(void);
};

inline bool cStreamdevServer::Active(void) {
	return m_Instance && m_Instance->m_Clients.Count() > 0;
}

extern cSVDRPhosts StreamdevHosts;

#endif // VDR_STREAMDEV_SERVER_H
