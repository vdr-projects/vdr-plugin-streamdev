/*
 *  $Id: component.c,v 1.2 2005/02/08 17:22:35 lordjaxom Exp $
 */
 
#include "server/component.h"
#include "server/connection.h"

#include <vdr/tools.h>
#include <string.h>
#include <errno.h>

cServerComponent::cServerComponent(const char *Protocol, const char *ListenIp,
		uint ListenPort) {
	m_Protocol = Protocol;
	m_ListenIp = ListenIp;
	m_ListenPort = ListenPort;
}

cServerComponent::~cServerComponent() {
}

bool cServerComponent::Init(void) {
	if (!m_Listen.Listen(m_ListenIp, m_ListenPort, 5)) {
		esyslog("Streamdev: Couldn't listen (%s) %s:%d: %s", m_Protocol, m_ListenIp,
				m_ListenPort, strerror(errno));
		return false;
	}
	isyslog("Streamdev: Listening (%s) on port %d", m_Protocol, m_ListenPort);
	return true;
}

void cServerComponent::Exit(void) {
	m_Listen.Close();
}

cServerConnection *cServerComponent::CanAct(const cTBSelect &Select) {
	if (Select.CanRead(m_Listen)) {
		cServerConnection *client = NewConnection();
		if (client->Accept(m_Listen)) {
			isyslog("Streamdev: Accepted new client (%s) %s:%d", m_Protocol,
					client->RemoteIp().c_str(), client->RemotePort());
			return client;
		} else {
			esyslog("Streamdev: Couldn't accept (%s): %s", m_Protocol, 
					strerror(errno));
			delete client;
		}
	}
	return NULL;
}
