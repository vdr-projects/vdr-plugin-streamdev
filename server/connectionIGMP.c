/*
 *  $Id: connectionIGMP.c,v 1.1.4.1 2010/06/11 06:06:02 schmirl Exp $
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
		m_StreamType(StreamType)
{
}

cConnectionIGMP::~cConnectionIGMP() 
{
	delete m_LiveStreamer;
}

bool cConnectionIGMP::Start(cChannel *Channel, in_addr_t Dst)
{
	if (Channel != NULL) {
		cDevice *device = GetDevice(Channel, 0);
		if (device != NULL) {
			device->SwitchChannel(Channel, false);
			struct in_addr ip;
			ip.s_addr = Dst;
			if (Connect(inet_ntoa(ip), m_ClientPort)) {
				m_LiveStreamer = new cStreamdevLiveStreamer(0, this);
				if (m_LiveStreamer->SetChannel(Channel, m_StreamType)) {
					m_LiveStreamer->SetDevice(device);
					if (!SetDSCP())
						LOG_ERROR_STR("unable to set DSCP sockopt");
					Dprintf("streamer start\n");
					m_LiveStreamer->Start(this);
					return true;
				}
				else
					esyslog("streamdev-server IGMP: SetDevice failed");
				DELETENULL(m_LiveStreamer);
			}
			else
				esyslog("streamdev-server IGMP: Connect failed: %m");
		}
		else
			esyslog("streamdev-server IGMP: GetDevice failed");
	}
	else
		esyslog("streamdev-server IGMP: Channel not found");
	return false;
}

void cConnectionIGMP::Stop()
{
	if (m_LiveStreamer) {
		m_LiveStreamer->Stop();
		DELETENULL(m_LiveStreamer);
	}
}
