/*
 *  $Id: connectionHTTP.c,v 1.17 2009/06/19 06:32:45 schmirl Exp $
 */

#include <ctype.h>
 
#include "server/connectionHTTP.h"
#include "server/menuHTTP.h"
#include "server/server.h"
#include "server/setup.h"

cConnectionHTTP::cConnectionHTTP(void): 
		cServerConnection("HTTP"),
		m_Status(hsRequest),
		m_LiveStreamer(NULL),
		m_StreamerParameter(""),
		m_Channel(NULL),
		m_Apid(0),
		m_StreamType((eStreamType)StreamdevServerSetup.HTTPStreamType),
		m_ChannelList(NULL)
{
	Dprintf("constructor hsRequest\n");
}

cConnectionHTTP::~cConnectionHTTP() 
{
	delete m_LiveStreamer;
}

bool cConnectionHTTP::CanAuthenticate(void)
{
	return opt_auth != NULL;
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
		if (strncasecmp(Cmd, "Host:", 5) == 0) {
			Dprintf("Host-Header\n");
			m_Host = (std::string) skipspace(Cmd + 5);
			return true;
		}
		else if (strncasecmp(Cmd, "Authorization:", 14) == 0) {
			Cmd = skipspace(Cmd + 14);
			if (strncasecmp(Cmd, "Basic", 5) == 0) {
				Dprintf("'Authorization Basic'-Header\n");
				m_Authorization = (std::string) skipspace(Cmd + 5);
				return true;
			}
		}
		Dprintf("header\n");
		return true;
	default:
		// skip additional blank lines
		if (*Cmd == '\0')
			return true;
		break;
	}
	return false; // ??? shouldn't happen
}

bool cConnectionHTTP::ProcessRequest(void) 
{
	Dprintf("process\n");
	if (!StreamdevHosts.Acceptable(RemoteIpAddr()))
	{
		if (!opt_auth || m_Authorization.empty() || m_Authorization.compare(opt_auth) != 0) {
			isyslog("streamdev-server: HTTP authorization required");
			DeferClose();
			return Respond("HTTP/1.0 401 Authorization Required")
				&& Respond("WWW-authenticate: basic Realm=\"Streamdev-Server\")")
				&& Respond("");
		}
	}
	if (m_Request.substr(0, 4) == "GET " && CmdGET(m_Request.substr(4))) {
		switch (m_Job) {
		case hjListing:
			if (m_ChannelList)
				return Respond("%s", true, m_ChannelList->HttpHeader().c_str());
			break;

		case hjTransfer:
			if (m_Channel == NULL) {
				DeferClose();
				return Respond("HTTP/1.0 404 not found");
			}
			
			m_LiveStreamer = new cStreamdevLiveStreamer(0, m_StreamerParameter);
			cDevice *device = GetDevice(m_Channel, 0);
			if (device != NULL) {
				device->SwitchChannel(m_Channel, false);
				if (m_LiveStreamer->SetChannel(m_Channel, m_StreamType, m_Apid)) {
					m_LiveStreamer->SetDevice(device);
					if (!SetDSCP())
						LOG_ERROR_STR("unable to set DSCP sockopt");
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
			return Respond("HTTP/1.0 409 Channel not available")
				&& Respond("");
		}
	}

	DeferClose();
	return Respond("HTTP/1.0 400 Bad Request")
		&& Respond("");
}

void cConnectionHTTP::Flushed(void) 
{
	std::string line;

	if (m_Status != hsBody)
		return;

	switch (m_Job) {
	case hjListing:
		if (m_ChannelList) {
			if (m_ChannelList->HasNext()) {
				if (!Respond("%s", true, m_ChannelList->Next().c_str()))
					DeferClose();
			}
			else {
				DELETENULL(m_ChannelList);
				m_Status = hsFinished;
				DeferClose();
			}
			return;
		}
		// should never be reached
		esyslog("streamdev-server cConnectionHTTP::Flushed(): no channel list");
		m_Status = hsFinished;
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
	const char *ptr, *sp, *pp, *fp, *xp, *qp, *ep;
	const cChannel *chan;
	int apid = 0;

	ptr = Opts.c_str();

	// find begin of URL
	sp = skipspace(ptr);
	// find end of URL (\0 or first space character)
	for (ep = sp; *ep && !isspace(*ep); ep++) 
		;
	// find begin of query string (first ?)
	for (qp = sp; qp < ep && *qp != '?'; qp++)
		;
	// find begin of filename (last /)
	for (fp = qp; fp > sp && *fp != '/'; --fp)
		;
	// find begin of section params (first ;)
	for (pp = sp; pp < fp && *pp != ';'; pp++)
		;
	// find filename extension (first .)
	for (xp = fp; xp < qp && *xp != '.'; xp++)
		;
	if (qp - xp > 5) // too long for a filename extension
		xp = qp;

	std::string type, filespec, fileext, query;
	// Streamtype with leading / stripped off
	if (pp > sp)
		type = Opts.substr(sp - ptr + 1, pp - sp - 1);
	// Section parameters with leading ; stripped off
	if (fp > pp)
		m_StreamerParameter = Opts.substr(pp - ptr + 1, fp - pp - 1);
	// file basename with leading / stripped off
	if (xp > fp)
		filespec = Opts.substr(fp - ptr + 1, xp - fp - 1);
	// file extension including leading .
	fileext = Opts.substr(xp - ptr, qp - xp);
	// query string including leading ?
	query = Opts.substr(qp - ptr, ep - qp);

	Dprintf("before channelfromstring: type(%s) param(%s) filespec(%s) fileext(%s) query(%s)\n", type.c_str(), m_StreamerParameter.c_str(), filespec.c_str(), fileext.c_str(), query.c_str());

	const char* pType = type.c_str();
	if (strcasecmp(pType, "PS") == 0) {
		m_StreamType = stPS;
	} else if (strcasecmp(pType, "PES") == 0) {
		m_StreamType = stPES;
	} else if (strcasecmp(pType, "TS") == 0) {
		m_StreamType = stTS;
	} else if (strcasecmp(pType, "ES") == 0) {
		m_StreamType = stES;
	} else if (strcasecmp(pType, "Extern") == 0) {
		m_StreamType = stExtern;
	}

	std::string groupTarget;
	cChannelIterator *iterator = NULL;

	if (filespec.compare("tree") == 0) {
		const cChannel* c = NULL;
		size_t groupIndex = query.find("group=");
		if (groupIndex != std::string::npos)
			c = cChannelList::GetGroup(atoi(query.c_str() + groupIndex + 6));
		iterator = new cListTree(c);
		groupTarget = filespec + fileext;
	} else if (filespec.compare("groups") == 0) {
		iterator = new cListGroups();
		groupTarget = (std::string) "group" + fileext;
	} else if (filespec.compare("group") == 0) {
		const cChannel* c = NULL;
		size_t groupIndex = query.find("group=");
		if (groupIndex != std::string::npos)
			c = cChannelList::GetGroup(atoi(query.c_str() + groupIndex + 6));
		iterator = new cListGroup(c);
	} else if (filespec.compare("channels") == 0) {
		iterator = new cListChannels();
	} else if (filespec.compare("all") == 0 ||
			(filespec.empty() && fileext.empty())) {
		iterator = new cListAll();
	}

	if (iterator) {
		if (filespec.empty() || fileext.compare(".htm") == 0 || fileext.compare(".html") == 0) {
			m_ChannelList = new cHtmlChannelList(iterator, m_StreamType, (filespec + fileext + query).c_str(), groupTarget.c_str());
			m_Job = hjListing;
		} else if (fileext.compare(".m3u") == 0) {
			std::string base;
			if (*(m_Host.c_str()))
				base = "http://" + m_Host + "/";
			else
				base = (std::string) "http://" + LocalIp() + ":" +
					(const char*) itoa(StreamdevServerSetup.HTTPServerPort) + "/";
			if (type.empty())
			{
				switch (m_StreamType)
				{
					case stTS:	base += "TS/"; break;
					case stPS:	base += "PS/"; break;
					case stPES:	base += "PES/"; break;
					case stES:	base += "ES/"; break;
					case stExtern:	base += "Extern/"; break;
					default:	break;
			
				}
			} else {
				base += type;
				if (!m_StreamerParameter.empty())
					base += ";" + m_StreamerParameter;
				base += "/";
			}
			m_ChannelList = new cM3uChannelList(iterator, base.c_str());
			m_Job = hjListing;
		} else {
			delete iterator;
			return false;
		}
	} else if ((chan = ChannelFromString(filespec.c_str(), &apid)) != NULL) {
		m_Channel = chan;
		m_Apid = apid;
		Dprintf("Apid is %d\n", apid);
		m_Job = hjTransfer;
	} else
		return false;
	
	Dprintf("after channelfromstring\n");
	return true;
}

