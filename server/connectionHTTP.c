/*
 *  $Id: connectionHTTP.c,v 1.21 2010/08/03 10:46:41 schmirl Exp $
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
		m_Channel(NULL),
		m_StreamType((eStreamType)StreamdevServerSetup.HTTPStreamType),
		m_ChannelList(NULL)
{
	Dprintf("constructor hsRequest\n");
	m_Apid[0] = m_Apid[1] = 0;
	m_Dpid[0] = m_Dpid[1] = 0;
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
		// parse METHOD PATH[?QUERY] VERSION
		{
			char *p, *q, *v;
			p = strchr(Cmd, ' ');
			if (p) {
				*p = 0;
				v = strchr(++p, ' ');
				if (v) {
					*v = 0;
					SetHeader("REQUEST_METHOD", Cmd);
					q = strchr(p, '?');
					if (q)
						*q = 0;
					SetHeader("QUERY_STRING", q ? ++q : "");
					SetHeader("PATH_INFO", p);
					m_Status = hsHeaders;
					return true;
				}
			}
		}
		return false;

	case hsHeaders:
		if (*Cmd == '\0') {
			m_Status = hsBody;
			return ProcessRequest();
		}
		else if (isspace(*Cmd)) {
			; //TODO: multi-line header
		}
		else {
			// convert header name to CGI conventions:
			// uppercase, '-' replaced with '_', prefix "HTTP_"
			char *p;
			for (p = Cmd; *p != 0 && *p != ':'; p++) {
				if (*p == '-')
					*p = '_';
				else
					*p = toupper(*p);
			}
			if (*p == ':') {
				*p = 0;
				p = skipspace(++p);
				// don't disclose Authorization header
				if (strcmp(Cmd, "AUTHORIZATION") == 0) {
					char *q;
					for (q = p; *q != 0 && *q != ' '; q++)
						*q = toupper(*q);
					if (p != q) {
						*q = 0;
						SetHeader("AUTH_TYPE", p);
						m_Authorization = (std::string) skipspace(++q);
					}
				}
				else
					SetHeader(Cmd, p, "HTTP_");
			}
		}
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
	// keys for Headers() hash
	const static std::string AUTH_TYPE("AUTH_TYPE");
	const static std::string REQUEST_METHOD("REQUEST_METHOD");
	const static std::string PATH_INFO("PATH_INFO");

	Dprintf("process\n");
	if (!StreamdevHosts.Acceptable(RemoteIpAddr())) {
		bool authOk = opt_auth && !m_Authorization.empty();
		if (authOk) {
			tStrStrMap::const_iterator it = Headers().find(AUTH_TYPE);

			if (it == Headers().end()) {
				// no authorization header present
				authOk = false;
			}
			else if (it->second.compare("BASIC") == 0) {
				// basic auth
				authOk &= m_Authorization.compare(opt_auth) == 0;
			}
			else {
				// unsupported auth type
				authOk = false;
			}
		}
		if (!authOk) {
			isyslog("streamdev-server: HTTP authorization required");
			DeferClose();
			return Respond("HTTP/1.0 401 Authorization Required")
				&& Respond("WWW-authenticate: basic Realm=\"Streamdev-Server\")")
				&& Respond("");
		}
	}

	tStrStrMap::const_iterator it_method = Headers().find(REQUEST_METHOD);
	tStrStrMap::const_iterator it_pathinfo = Headers().find(PATH_INFO);
	if (it_method == Headers().end() || it_pathinfo == Headers().end()) {
		// should never happen
		esyslog("streamdev-server connectionHTTP: Missing method or pathinfo");
	} else if (it_method->second.compare("GET") == 0 && ProcessURI(it_pathinfo->second)) {
		if (m_ChannelList)
			return Respond("%s", true, m_ChannelList->HttpHeader().c_str());
		else if (m_Channel != NULL) {
			cDevice *device = NULL;
			if (ProvidesChannel(m_Channel, 0))
				device = GetDevice(m_Channel, 0);
			if (device != NULL) {
				device->SwitchChannel(m_Channel, false);
				m_LiveStreamer = new cStreamdevLiveStreamer(0, this);
				if (m_LiveStreamer->SetChannel(m_Channel, m_StreamType, m_Apid[0] ? m_Apid : NULL, m_Dpid[0] ? m_Dpid : NULL)) {
					m_LiveStreamer->SetDevice(device);
					if (!SetDSCP())
						LOG_ERROR_STR("unable to set DSCP sockopt");
					if (m_StreamType == stEXT) {
						return Respond("HTTP/1.0 200 OK");
					} else if (m_StreamType == stES && (m_Apid[0] || m_Dpid[0] || ISRADIO(m_Channel))) {
						return Respond("HTTP/1.0 200 OK")
						    && Respond("Content-Type: audio/mpeg")
						    && Respond("icy-name: %s", true, m_Channel->Name())
						    && Respond("");
					} else if (ISRADIO(m_Channel)) {
						return Respond("HTTP/1.0 200 OK")
						    && Respond("Content-Type: audio/mpeg")
						    && Respond("");
					} else {
						return Respond("HTTP/1.0 200 OK")
						    && Respond("Content-Type: video/mpeg")
						    && Respond("");
					}
				}
				DELETENULL(m_LiveStreamer);
			}
			DeferClose();
			return Respond("HTTP/1.0 503 Service unavailable")
				&& Respond("");
		}
		else {
			DeferClose();
			return Respond("HTTP/1.0 404 not found")
				&& Respond("");
		}
	} else if (it_method->second.compare("HEAD") == 0 && ProcessURI(it_pathinfo->second)) {
		DeferClose();
		if (m_ChannelList)
			return Respond("%s", true, m_ChannelList->HttpHeader().c_str());
		else if (m_Channel != NULL) {
			if (ProvidesChannel(m_Channel, 0)) {
				if (m_StreamType == stEXT) {
					// TODO
					return Respond("HTTP/1.0 200 OK")
					    && Respond("");
				} else if (m_StreamType == stES && (m_Apid[0] || m_Dpid[0] || ISRADIO(m_Channel))) {
					return Respond("HTTP/1.0 200 OK")
					    && Respond("Content-Type: audio/mpeg")
					    && Respond("icy-name: %s", true, m_Channel->Name())
					    && Respond("");
				} else if (ISRADIO(m_Channel)) {
					return Respond("HTTP/1.0 200 OK")
					    && Respond("Content-Type: audio/mpeg")
					    && Respond("");
				} else {
					return Respond("HTTP/1.0 200 OK")
					    && Respond("Content-Type: video/mpeg")
					    && Respond("");
				}
			}
			return Respond("HTTP/1.0 503 Service unavailable")
				&& Respond("");
		}
		else {
			return Respond("HTTP/1.0 404 not found")
				&& Respond("");
		}
	}

	DeferClose();
	return Respond("HTTP/1.0 400 Bad Request")
		&& Respond("");
}

void cConnectionHTTP::Flushed(void) 
{
	if (m_Status != hsBody)
		return;

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
	else if (m_Channel != NULL) {
		Dprintf("streamer start\n");
		m_LiveStreamer->Start(this);
		m_Status = hsFinished;
	}
	else {
		// should never be reached
		esyslog("streamdev-server cConnectionHTTP::Flushed(): no job to do");
		m_Status = hsFinished;
	}
}

cChannelList* cConnectionHTTP::ChannelListFromString(const std::string& Path, const std::string& Filebase, const std::string& Fileext) const
{
	// keys for Headers() hash
	const static std::string QUERY_STRING("QUERY_STRING");
	const static std::string HOST("HTTP_HOST");

	tStrStrMap::const_iterator it_query = Headers().find(QUERY_STRING);
	const std::string& query = it_query == Headers().end() ? "" : it_query->second;

	std::string groupTarget;
	cChannelIterator *iterator = NULL;

	if (Filebase.compare("tree") == 0) {
		const cChannel* c = NULL;
		size_t groupIndex = query.find("group=");
		if (groupIndex != std::string::npos)
			c = cChannelList::GetGroup(atoi(query.c_str() + groupIndex + 6));
		iterator = new cListTree(c);
		groupTarget = Filebase + Fileext;
	} else if (Filebase.compare("groups") == 0) {
		iterator = new cListGroups();
		groupTarget = (std::string) "group" + Fileext;
	} else if (Filebase.compare("group") == 0) {
		const cChannel* c = NULL;
		size_t groupIndex = query.find("group=");
		if (groupIndex != std::string::npos)
			c = cChannelList::GetGroup(atoi(query.c_str() + groupIndex + 6));
		iterator = new cListGroup(c);
	} else if (Filebase.compare("channels") == 0) {
		iterator = new cListChannels();
	} else if (Filebase.compare("all") == 0 ||
			(Filebase.empty() && Fileext.empty())) {
		iterator = new cListAll();
	}

	if (iterator) {
		if (Filebase.empty() || Fileext.compare(".htm") == 0 || Fileext.compare(".html") == 0) {
			std::string self = Filebase + Fileext;
			if (!query.empty())
				self += '?' + query;
			return new cHtmlChannelList(iterator, m_StreamType, self.c_str(), groupTarget.c_str());
		} else if (Fileext.compare(".m3u") == 0) {
			std::string base;
			tStrStrMap::const_iterator it = Headers().find(HOST);
			if (it != Headers().end())
				base = "http://" + it->second + "/";
			else
				base = (std::string) "http://" + LocalIp() + ":" +
					(const char*) itoa(StreamdevServerSetup.HTTPServerPort) + "/";
			base += Path;
			return new cM3uChannelList(iterator, base.c_str());
		} else {
			delete iterator;
		}
	}
	return NULL;
}

bool cConnectionHTTP::ProcessURI(const std::string& PathInfo) 
{
	std::string filespec, fileext;
	size_t file_pos = PathInfo.rfind('/');

	if (file_pos != std::string::npos) {
		size_t ext_pos = PathInfo.rfind('.');
		// file basename with leading / stripped off
		filespec = PathInfo.substr(file_pos + 1, ext_pos - file_pos - 1);
		if (ext_pos != std::string::npos)
			// file extension including leading .
			fileext = PathInfo.substr(ext_pos);
	}
	if (fileext.length() > 5) {
		//probably not an extension
		filespec += fileext;
		fileext.clear();
	}

	// Streamtype with leading / stripped off
	std::string type = PathInfo.substr(1, PathInfo.find_first_of("/;", 1) - 1);
	const char* pType = type.c_str();
	if (strcasecmp(pType, "PS") == 0) {
		m_StreamType = stPS;
	} else if (strcasecmp(pType, "PES") == 0) {
		m_StreamType = stPES;
	} else if (strcasecmp(pType, "TS") == 0) {
		m_StreamType = stTS;
	} else if (strcasecmp(pType, "ES") == 0) {
		m_StreamType = stES;
	} else if (strcasecmp(pType, "EXT") == 0) {
		m_StreamType = stEXT;
	}

	Dprintf("before channelfromstring: type(%s) filespec(%s) fileext(%s)\n", type.c_str(), filespec.c_str(), fileext.c_str());

	if ((m_ChannelList = ChannelListFromString(PathInfo.substr(1, file_pos), filespec.c_str(), fileext.c_str())) != NULL) {
		Dprintf("Channel list requested\n");
		return true;
	} else if ((m_Channel = ChannelFromString(filespec.c_str(), &m_Apid[0], &m_Dpid[0])) != NULL) {
		Dprintf("Channel found. Apid/Dpid is %d/%d\n", m_Apid[0], m_Dpid[0]);
		return true;
	} else
		return false;
}

cString cConnectionHTTP::ToText() const
{
	cString str = cServerConnection::ToText();
	return m_LiveStreamer ? cString::sprintf("%s\t%s", *str, *m_LiveStreamer->ToText()) : str;
}
