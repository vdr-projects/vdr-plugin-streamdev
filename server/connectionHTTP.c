/*
 *  $Id: connectionHTTP.c,v 1.6 2005/02/10 22:24:26 lordjaxom Exp $
 */
 
#include "server/connectionHTTP.h"
#include "server/livestreamer.h"
#include "server/setup.h"

cConnectionHTTP::cConnectionHTTP(void): cServerConnection("HTTP") {
	m_Channel      = NULL;
	m_Apid         = 0;
	m_ListChannel  = NULL;
	m_LiveStreamer = NULL;
	m_Status       = hsRequest;
	m_StreamType   = (eStreamType)StreamdevServerSetup.HTTPStreamType;
	m_Startup      = false;
}

cConnectionHTTP::~cConnectionHTTP() {
	if (m_LiveStreamer != NULL) delete m_LiveStreamer;
}

void cConnectionHTTP::Detach(void) {
	if (m_LiveStreamer != NULL) m_LiveStreamer->Detach();
}

void cConnectionHTTP::Attach(void) {
	if (m_LiveStreamer != NULL) m_LiveStreamer->Attach();
}

bool cConnectionHTTP::Command(char *Cmd) {
	switch (m_Status) {
	case hsRequest:
		if (strncmp(Cmd, "GET ", 4) == 0) return CmdGET(Cmd + 4);
		else {
			DeferClose();
			m_Status = hsTransfer; // Ignore following lines
			return Respond("HTTP/1.0 400 Bad Request");
		}
		break;

	case hsHeaders:
		if (*Cmd == '\0') {
			if (m_ListChannel != NULL) {
				m_Status = hsListing;
				return Respond("HTTP/1.0 200 OK")
						&& Respond("Content-Type: text/html")
						&& Respond("")
						&& Respond("<html><head><title>VDR Channel Listing</title></head>")
						&& Respond("<body><ul>");
			} else if (m_Channel == NULL) {
				DeferClose();
				return Respond("HTTP/1.0 404 not found");
			}
			m_Status = hsTransfer;
			m_LiveStreamer = new cStreamdevLiveStreamer(0);
			cDevice *device = GetDevice(m_Channel, 0);
			if (device != NULL) {
				device->SwitchChannel(m_Channel, false);
				if (m_LiveStreamer->SetChannel(m_Channel, m_StreamType, m_Apid)) {
					m_LiveStreamer->SetDevice(device);
					m_Startup = true;
					if (m_StreamType == stES && (m_Channel->Vpid() == 0 
							|| m_Channel->Vpid() == 1 || m_Channel->Vpid() == 0x1FFF)) {
						return Respond("HTTP/1.0 200 OK")
								&& Respond("Content-Type: audio/mpeg")
								&& Respond((std::string)"icy-name: " + m_Channel->Name())
								&& Respond("");
					} else {
						return Respond("HTTP/1.0 200 OK")
								&& Respond("Content-Type: video/mpeg")
								&& Respond("");
					}
				}
			}
			DELETENULL(m_LiveStreamer);
			DeferClose();
			return Respond("HTTP/1.0 409 Channel not available");
		}
		break;

	default:
		break;
	}
	return true;
}

void cConnectionHTTP::Flushed(void) {
	if (m_Status == hsListing) {
		if (m_ListChannel == NULL) {
			Respond("</ul></body></html>");
			DeferClose();
			return;
		}

		std::string line;
		if (m_ListChannel->GroupSep())
			line = (std::string)"<li>--- " + m_ListChannel->Name() + "---</li>";
		else
			line = (std::string)"<li><a href=\"http://" + LocalIp() + ":" 
			     + (const char*)itoa(StreamdevServerSetup.HTTPServerPort) + "/"
			     + (const char*)m_ListChannel->GetChannelID().ToString() + "\">"
			     + m_ListChannel->Name() + "</a></li>";
		if (!Respond(line))
			DeferClose();
		m_ListChannel = Channels.Next(m_ListChannel);
	} else if (m_Startup) {
		Dprintf("streamer start\n");
		m_LiveStreamer->Start(this);
		m_Startup = false;
	}
}

bool cConnectionHTTP::CmdGET(char *Opts) {
	const cChannel *chan;
	char *ep;
	int apid = 0;

	Opts = skipspace(Opts);
	while (*Opts == '/')
		++Opts;

	if (strncasecmp(Opts, "PS/", 3) == 0) {
		m_StreamType = stPS;
		Opts+=3;
	} else if (strncasecmp(Opts, "PES/", 4) == 0) {
		m_StreamType = stPES;
		Opts+=4;
	} else if (strncasecmp(Opts, "TS/", 3) == 0) {
		m_StreamType = stTS;
		Opts+=3;
	} else if (strncasecmp(Opts, "ES/", 3) == 0) {
		m_StreamType = stES;
		Opts+=3;
	}

	while (*Opts == '/')
		++Opts;
	for (ep = Opts + strlen(Opts); ep >= Opts && !isspace(*ep); --ep) 
		;
	*ep = '\0';

	Dprintf("before channelfromstring\n");
	if (strncmp(Opts, "channels.htm", 12) == 0) {
		m_ListChannel = Channels.First();
		m_Status = hsHeaders;
	} else if ((chan = ChannelFromString(Opts, &apid)) != NULL) {
		m_Channel = chan;
		m_Apid = apid;
		Dprintf("Apid is %d\n", apid);
		m_Status = hsHeaders;
	}
	Dprintf("after channelfromstring\n");
	return true;
}

#if 0
bool cHTTPConnection::Listing(void) {
	cChannel *chan;
	cTBString line;

	Respond(200, "OK");
	Respond("Content-Type: text/html");
	Respond("");
	Respond("<html><head><title>VDR Channel Listing</title></head>");
	Respond("<body><ul>");

	for (chan = Channels.First(); chan != NULL; chan = Channels.Next(chan)) {
		if (chan->GroupSep() && !*chan->Name())
			continue;

		if (chan->GroupSep())
			line.Format("<li>--- %s ---</li>", chan->Name());
		else
			line.Format("<li><a href=\"http://%s:%d/%s\">%s</a></li>", 
				(const char*)m_Socket.LocalIp(), StreamdevServerSetup.HTTPServerPort, 
				chan->GetChannelID().ToString(), chan->Name());
		Respond(line);
	}
	
	Respond("</ul></body></html>");

	m_DeferClose = true;
	return true;
}
#endif
