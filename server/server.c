/*
 *  $Id: server.c,v 1.3 2005/05/09 20:22:29 lordjaxom Exp $
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

cStreamdevServer         *cStreamdevServer::m_Instance = NULL;
cList<cServerComponent>   cStreamdevServer::m_Servers;
cList<cServerConnection>  cStreamdevServer::m_Clients;

cStreamdevServer::cStreamdevServer(void):
		cThread("streamdev server"),
		m_Active(false)
{
	Start();
}

cStreamdevServer::~cStreamdevServer() 
{
	Stop();
}

void cStreamdevServer::Initialize(void) 
{
	if (m_Instance == NULL) {
		if (StreamdevServerSetup.StartVTPServer)  Register(new cComponentVTP);
		if (StreamdevServerSetup.StartHTTPServer) Register(new cComponentHTTP);

		m_Instance = new cStreamdevServer;
	}
}

void cStreamdevServer::Destruct(void) 
{
	DELETENULL(m_Instance);
}

void cStreamdevServer::Stop(void) 
{
	if (m_Active) {
		m_Active = false;
		Cancel(3);
	}
}

void cStreamdevServer::Register(cServerComponent *Server) 
{
	m_Servers.Add(Server);
}

void cStreamdevServer::Action(void) 
{
	m_Active = true;

	/* Initialize Server components, deleting those that failed */
	for (cServerComponent *c = m_Servers.First(); c;) {
		cServerComponent *next = m_Servers.Next(c);
		if (!c->Initialize())
			m_Servers.Del(c);
		c = next;
	}
			
	if (m_Servers.Count() == 0) {
		esyslog("ERROR: no streamdev server activated, exiting");
		m_Active = false;
	}

	cTBSelect select;
	while (m_Active) {
		select.Clear();

		/* Ask all Server components to register to the selector */
		for (cServerComponent *c = m_Servers.First(); c; c = m_Servers.Next(c))
			select.Add(c->Socket(), false);
		
		/* Ask all Client connections to register to the selector */
		for (cServerConnection *s = m_Clients.First(); s; s = m_Clients.Next(s))
		{
			select.Add(s->Socket(), false);
			if (s->HasData())
				select.Add(s->Socket(), true);
		}

		if (select.Select() < 0) {
			if (m_Active) // no exit was requested while polling
				esyslog("fatal error, server exiting: %m");
			break;
		}
	
		/* Ask all Server components to act on signalled sockets */
		for (cServerComponent *c = m_Servers.First(); c; c = m_Servers.Next(c)){
			if (select.CanRead(c->Socket())) {
				cServerConnection *client = c->Accept();
				m_Clients.Add(client);

				if (m_Clients.Count() > StreamdevServerSetup.MaxClients) {
					esyslog("streamdev: too many clients, rejecting %s:%d",
					        client->RemoteIp().c_str(), client->RemotePort());
					client->Reject();
				} else if (!StreamdevHosts.Acceptable(client->RemoteIpAddr())) {
					esyslog("streamdev: client %s:%d not allowed to connect",
					        client->RemoteIp().c_str(), client->RemotePort());
					client->Reject();
				} else 
					client->Welcome();
			}
		}

		/* Ask all Client connections to act on signalled sockets */
		for (cServerConnection *s = m_Clients.First(); s;) {
			bool result = true;

			if (select.CanWrite(s->Socket()))
				result = s->Write();

			if (result && select.CanRead(s->Socket()))
				result = s->Read();
			
			cServerConnection *next = m_Clients.Next(s);
			if (!result) {
				isyslog("streamdev: closing streamdev connection to %s:%d", 
				        s->RemoteIp().c_str(), s->RemotePort());
				s->Close();
				m_Clients.Del(s);
			}
			s = next;
		}
	}
	
	while (m_Clients.Count() > 0) {
		cServerConnection *s = m_Clients.First();
		s->Close();
		m_Clients.Del(s);
	}

	while (m_Servers.Count() > 0) {
		cServerComponent *c = m_Servers.First();
		c->Destruct();
		m_Servers.Del(c);
	}

	m_Active = false;
}
