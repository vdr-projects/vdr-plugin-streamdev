/*
 *  $Id: connectionHTTP.c,v 1.9 2005/05/09 20:22:29 lordjaxom Exp $
 */

#include <ctype.h>
 
#include "server/connectionHTTP.h"
#include "server/setup.h"

cConnectionHTTP::cConnectionHTTP(void): 
		cServerConnection("HTTP"),
		m_Status(hsRequest),
		m_LiveStreamer(NULL),
		m_Channel(NULL),
		m_Apid(0),
		m_StreamType((eStreamType)StreamdevServerSetup.HTTPStreamType),
		m_ListChannel(NULL)
{
	Dprintf("constructor hsRequest\n");
}

cConnectionHTTP::~cConnectionHTTP() 
{
	delete m_LiveStreamer;
}

bool cConnectionHTTP::Command(char *Cmd) 
{
	Dprintf("command %s\n", Cmd);
	switch (m_Status) {
	case hsRequest:
		Dprintf("Request\n");
		m_Request = Cmd;
		m_Status = hsHeaders;
		return true;

	case hsHeaders:
		if (*Cmd == '\0') {
			m_Status = hsBody;
			return ProcessRequest();
		}
		Dprintf("header\n");
		return true;
	}
	return false; // ??? shouldn't happen
}

bool cConnectionHTTP::ProcessRequest(void) 
{
	Dprintf("process\n");
	if (m_Request.substr(0, 4) == "GET " && CmdGET(m_Request.substr(4))) {
		switch (m_Job) {
		case hjListing:
			return Respond("HTTP/1.0 200 OK")
			    && Respond("Content-Type: text/html")
			    && Respond("")
			    && Respond("<html><head><title>VDR Channel Listing</title></head>")
			    && Respond("<body><ul>");

		case hjTransfer:
			if (m_Channel == NULL) {
				DeferClose();
				return Respond("HTTP/1.0 404 not found");
			}
			
			m_LiveStreamer = new cStreamdevLiveStreamer(0);
			cDevice *device = GetDevice(m_Channel, 0);
			if (device != NULL) {
				device->SwitchChannel(m_Channel, false);
				if (m_LiveStreamer->SetChannel(m_Channel, m_StreamType, m_Apid)) {
					m_LiveStreamer->SetDevice(device);
					if (m_StreamType == stES && (m_Apid != 0 || ISRADIO(m_Channel))) {
						return Respond("HTTP/1.0 200 OK")
						    && Respond("Content-Type: audio/mpeg")
						    && Respond("icy-name: %s", true, m_Channel->Name())
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
	}

	DeferClose();
	return Respond("HTTP/1.0 400 Bad Request");
}

void cConnectionHTTP::Flushed(void) 
{
	std::string line;

	if (m_Status != hsBody)
		return;

	switch (m_Job) {
	case hjListing:
		if (m_ListChannel == NULL) {
			Respond("</ul></body></html>");
			DeferClose();
			m_Status = hsFinished;
			return;
		}

		if (m_ListChannel->GroupSep())
			line = (std::string)"<li>--- " + m_ListChannel->Name() + "---</li>";
		else {
			int index = 1;
			line = (std::string)"<li><a href=\"http://" + LocalIp() + ":" 
			     + (const char*)itoa(StreamdevServerSetup.HTTPServerPort) + "/"
			     + StreamTypes[m_StreamType] + "/"
			     + (const char*)m_ListChannel->GetChannelID().ToString() + "\">"
			     + m_ListChannel->Name() + "</a> ";
			for (int i = 0; m_ListChannel->Apid(i) != 0; ++i, ++index) {
				line += "<a href=\"http://" + LocalIp() + ":"
				     + (const char*)itoa(StreamdevServerSetup.HTTPServerPort) + "/"
				     + StreamTypes[m_StreamType] + "/"
				     + (const char*)m_ListChannel->GetChannelID().ToString() + "+"
				     + (const char*)itoa(index) + "\">("
				     + m_ListChannel->Alang(i) + ")</a> ";
			}
			for (int i = 0; m_ListChannel->Dpid(i) != 0; ++i, ++index) {
				line += "<a href=\"http://" + LocalIp() + ":"
				     + (const char*)itoa(StreamdevServerSetup.HTTPServerPort) + "/"
				     + StreamTypes[m_StreamType] + "/"
				     + (const char*)m_ListChannel->GetChannelID().ToString() + "+"
				     + (const char*)itoa(index) + "\">("
				     + m_ListChannel->Dlang(i) + ")</a> ";
			}
			line += "</li>";
		}
		if (!Respond(line.c_str()))
			DeferClose();
		m_ListChannel = Channels.Next(m_ListChannel);
		break;

	case hjTransfer:
		Dprintf("streamer start\n");
		m_LiveStreamer->Start(this);
		m_Status = hsFinished;
		break;
	}
}

bool cConnectionHTTP::CmdGET(const std::string &Opts) 
{
	const char *sp = Opts.c_str(), *ptr = sp, *ep;
	const cChannel *chan;
	int apid = 0, pos;

	ptr = skipspace(ptr);
	while (*ptr == '/')
		++ptr;

	if (strncasecmp(ptr, "PS/", 3) == 0) {
		m_StreamType = stPS;
		ptr += 3;
	} else if (strncasecmp(ptr, "PES/", 4) == 0) {
		m_StreamType = stPES;
		ptr += 4;
	} else if (strncasecmp(ptr, "TS/", 3) == 0) {
		m_StreamType = stTS;
		ptr += 3;
	} else if (strncasecmp(ptr, "ES/", 3) == 0) {
		m_StreamType = stES;
		ptr += 3;
	} else if (strncasecmp(ptr, "Extern/", 3) == 0) {
		m_StreamType = stExtern;
		ptr += 7;
	}

	while (*ptr == '/')
		++ptr;
	for (ep = ptr + strlen(ptr); ep >= ptr && !isspace(*ep); --ep) 
		;

	std::string filespec = Opts.substr(ptr - sp, ep - ptr);
	Dprintf("substr: %s\n", filespec.c_str());

	Dprintf("before channelfromstring\n");
	if (filespec == "" || filespec.substr(0, 12) == "channels.htm") {
		m_ListChannel = Channels.First();
		m_Job = hjListing;
	} else if ((chan = ChannelFromString(filespec.c_str(), &apid)) != NULL) {
		m_Channel = chan;
		m_Apid = apid;
		Dprintf("Apid is %d\n", apid);
		m_Job = hjTransfer;
	}
	Dprintf("after channelfromstring\n");
	return true;
}

