/*
 *  $Id: server.c,v 1.1 2004/12/30 22:44:20 lordjaxom Exp $
 */

#include "server/server.h"
#include "server/componentVTP.h"
#include "server/componentHTTP.h"
#include "server/setup.h"

#include <vdr/tools.h>
#include <tools/select.h>
#include <string.h>
#include <errno.h>

cSVDRPhosts StreamdevHosts;

cStreamdevServer *cStreamdevServer::m_Instance = NULL;

cStreamdevServer::cStreamdevServer(void)
#if VDRVERSNUM >= 10300
		: cThread("Streamdev: server")
#endif
{
	m_Active = false;

	StreamdevHosts.Load(AddDirectory(cPlugin::ConfigDirectory(), 
			"streamdevhosts.conf"), true);
}

cStreamdevServer::~cStreamdevServer() {
	if (m_Active) Stop();
}

void cStreamdevServer::Init(void) {
	if (m_Instance == NULL) {
		m_Instance = new cStreamdevServer;
		if (StreamdevServerSetup.StartVTPServer)
			m_Instance->Register(new cComponentVTP);
		if (StreamdevServerSetup.StartHTTPServer)
			m_Instance->Register(new cComponentHTTP);
		m_Instance->Start();
	}
}

void cStreamdevServer::Exit(void) {
	if (m_Instance != NULL) {
		m_Instance->Stop();
		DELETENULL(m_Instance);
	}
}

void cStreamdevServer::Stop(void) {
	m_Active = false;
	Cancel(3);
}

void cStreamdevServer::Register(cServerComponent *Server) {
	m_Servers.Add(Server);
}

void cStreamdevServer::Action(void) {
	cTBSelect select;

#if VDRVERSNUM < 10300
	isyslog("Streamdev: Server thread started (pid=%d)", getpid());
#endif

	m_Active = true;

	/* Initialize Server components, deleting those that failed */
	for (cServerComponent *c = m_Servers.First(); c;) {
		cServerComponent *next = m_Servers.Next(c);
		if (!c->Init())
			m_Servers.Del(c);
		c = next;
	}
			
	if (!m_Servers.Count()) {
		esyslog("Streamdev: No server components registered, exiting");
		m_Active = false;
	}

	while (m_Active) {
		select.Clear();

		/* Ask all Server components to register to the selector */
		for (cServerComponent *c = m_Servers.First(); c; c = m_Servers.Next(c))
			c->AddSelect(select);
		
		/* Ask all Client connections to register to the selector */
		for (cServerConnection *s = m_Clients.First(); s; s = m_Clients.Next(s))
			s->AddSelect(select);

		if (select.Select(1000) < 0) {
			if (!m_Active) // Exit was requested while polling
				continue;
			esyslog("Streamdev: Fatal error, server exiting: %s", strerror(errno));
			m_Active = false;
		}
	
		/* Ask all Server components to act on signalled sockets */
		for (cServerComponent *c = m_Servers.First(); c; c = m_Servers.Next(c)) {
			cServerConnection *client;
			if ((client = c->CanAct(select)) != NULL) {
				m_Clients.Add(client);

				if (m_Clients.Count() > StreamdevServerSetup.MaxClients) {
					esyslog("Streamdev: Too many clients, rejecting %s:%d",
							(const char*)client->RemoteIp(), client->RemotePort());
					client->Reject();
				} else if (!StreamdevHosts.Acceptable(client->RemoteIpAddr())) {
					esyslog("Streamdev: Client from %s:%d not allowed to connect",
							(const char*)client->RemoteIp(), client->RemotePort());
					client->Reject();
				} else 
					client->Welcome();
			}
		}

		/* Ask all Client connections to act on signalled sockets */
		for (cServerConnection *s = m_Clients.First(); s;) {
			cServerConnection *next = m_Clients.Next(s);
			if (!s->CanAct(select)) {
				isyslog("Streamdev: Closing connection to %s:%d", 
						(const char*)s->RemoteIp(), s->RemotePort());
				s->Close();
				m_Clients.Del(s);
			}
			s = next;
		}
	}
	
	while (m_Clients.Count()) {
		cServerConnection *client = m_Clients.First();
		client->Close();
		m_Clients.Del(client);
	}

	while (m_Servers.Count()) {
		cServerComponent *server = m_Servers.First();
		server->Exit();
		m_Servers.Del(server);
	}

#if VDRVERSNUM < 10300
	isyslog("Streamdev: Server thread stopped");
#endif
}
