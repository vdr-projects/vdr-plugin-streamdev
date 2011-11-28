/*
 *  $Id: connectionIGMP.c,v 1.3 2010/08/03 10:46:41 schmirl Exp $
 */

#include <ctype.h>
 
#include "server/connectionIGMP.h"
#include "server/server.h"
#include "server/setup.h"
#include <vdr/channels.h>

cConnectionIGMP::cConnectionIGMP(const char* Name, int ClientPort, eStreamType StreamType) :
		cServerConnection(Name, SOCK_DGRAM),
		m_LiveStreamer(NULL),
		m_ClientPort(ClientPort),
		m_StreamType(StreamType),
		m_Channel(NULL)
{
}

cConnectionIGMP::~cConnectionIGMP() 
{
	delete m_LiveStreamer;
}

bool cConnectionIGMP::SetChannel(cChannel *Channel, in_addr_t Dst)
{
	if (Channel) {
		m_Channel = Channel;
		struct in_addr ip;
		ip.s_addr = Dst;
		if (Connect(inet_ntoa(ip), m_ClientPort))
			return true;
		else
			esyslog("streamdev-server IGMP: Connect failed: %m");
		return false;
	}
	else
		esyslog("streamdev-server IGMP: Channel not found");
	return false;
}

void cConnectionIGMP::Welcome()
{
	cDevice *device = NULL;
	if (ProvidesChannel(m_Channel, 0))
		device = GetDevice(m_Channel, 0);
	if (device != NULL) {
		device->SwitchChannel(m_Channel, false);
		m_LiveStreamer = new cStreamdevLiveStreamer(0, this);
		if (m_LiveStreamer->SetChannel(m_Channel, m_StreamType)) {
			m_LiveStreamer->SetDevice(device);
			if (!SetDSCP())
				LOG_ERROR_STR("unable to set DSCP sockopt");
			Dprintf("streamer start\n");
			m_LiveStreamer->Start(this);
		}
		else {
			esyslog("streamdev-server IGMP: SetChannel failed");
			DELETENULL(m_LiveStreamer);
		}
	}
	else
		esyslog("streamdev-server IGMP: GetDevice failed");
}

bool cConnectionIGMP::Close()
{
	if (m_LiveStreamer)
		m_LiveStreamer->Stop();
	return cServerConnection::Close();
}

cString cConnectionIGMP::ToText() const
{
	cString str = cServerConnection::ToText();
	return m_LiveStreamer ? cString::sprintf("%s\t%s", *str, *m_LiveStreamer->ToText()) : str;
}
