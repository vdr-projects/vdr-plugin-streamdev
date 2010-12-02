/*
 *  $Id: component.c,v 1.3 2005/05/09 20:22:29 lordjaxom Exp $
 */
 
#include "server/component.h"
#include "server/connection.h"

cServerComponent::cServerComponent(const char *Protocol, const char *ListenIp,
                                   uint ListenPort):
		m_Protocol(Protocol),
		m_ListenIp(ListenIp),
		m_ListenPort(ListenPort)
{
}

cServerComponent::~cServerComponent() 
{
}

bool cServerComponent::Initialize(void) 
{
	if (!m_Listen.Listen(m_ListenIp, m_ListenPort, 5)) {
		esyslog("Streamdev: Couldn't listen (%s) %s:%d: %m", 
		        m_Protocol, m_ListenIp, m_ListenPort);
		return false;
	}
	isyslog("Streamdev: Listening (%s) on port %d", m_Protocol, m_ListenPort);
	return true;
}

void cServerComponent::Destruct(void) 
{
	m_Listen.Close();
}

cServerConnection *cServerComponent::Accept(void) 
{
	cServerConnection *client = NewClient();
	if (client->Accept(m_Listen)) {
		isyslog("Streamdev: Accepted new client (%s) %s:%d", m_Protocol,
				client->RemoteIp().c_str(), client->RemotePort());
		return client;
	} else {
		esyslog("Streamdev: Couldn't accept (%s): %m", m_Protocol);
		delete client;
	}
	return NULL;
}
