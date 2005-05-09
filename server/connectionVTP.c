/*
 *  $Id: connectionVTP.c,v 1.7 2005/05/09 20:22:29 lordjaxom Exp $
 */
 
#include "server/connectionVTP.h"
#include "server/livestreamer.h"
#include "server/suspend.h"
#include "setup.h"

#include <vdr/tools.h>
#include <tools/select.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

/* VTP Response codes:
	220: Service ready
	221: Service closing connection
	451: Requested action aborted: try again
	500: Syntax error or Command unrecognized
	501: Wrong parameters or missing parameters
	550: Requested action not taken
	551: Data connection not accepted
	560: Channel not available currently
	561: Capability not known
	562: Pid not available currently
	563: Recording not available (currently?)
*/

// --- cLSTEHandler -----------------------------------------------------------

class cLSTEHandler 
{
private:
	enum eStates { Channel, Event, Title, Subtitle, Description, Vps, 
	               EndEvent, EndChannel, EndEPG };
	cConnectionVTP    *m_Client;
	cSchedulesLock    *m_SchedulesLock;
	const cSchedules  *m_Schedules;
	const cSchedule   *m_Schedule;
	const cEvent      *m_Event;
	int                m_Errno;
	char              *m_Error;
	eStates            m_State;
	bool               m_Traverse;
public:
	cLSTEHandler(cConnectionVTP *Client, const char *Option);
	~cLSTEHandler();
	bool Next(bool &Last);
};

cLSTEHandler::cLSTEHandler(cConnectionVTP *Client, const char *Option):
		m_Client(Client),
		m_SchedulesLock(new cSchedulesLock(false, 500)),
		m_Schedules(cSchedules::Schedules(*m_SchedulesLock)),
		m_Schedule(NULL),
		m_Event(NULL),
		m_Errno(0),
		m_Error(NULL),
		m_State(Channel),
		m_Traverse(false)
{
	eDumpMode dumpmode = dmAll;
	time_t attime = 0;

	if (m_Schedules != NULL && *Option) {
		char buf[strlen(Option) + 1];
		strcpy(buf, Option);
		const char *delim = " \t";
		char *strtok_next;
		char *p = strtok_r(buf, delim, &strtok_next);
		while (p && dumpmode == dmAll) {
			if (strcasecmp(p, "NOW") == 0)
				dumpmode = dmPresent;
			else if (strcasecmp(p, "NEXT") == 0)
				dumpmode = dmFollowing;
			else if (strcasecmp(p, "AT") == 0) {
				dumpmode = dmAtTime;
				if ((p = strtok_r(NULL, delim, &strtok_next)) != NULL) {
					if (isnumber(p))
						attime = strtol(p, NULL, 10);
					else {
						m_Errno = 501;
						m_Error = strdup("Invalid time");
						break;
					}
				} else {
					m_Errno = 501;
					m_Error = strdup("Missing time");
					break;
				}
			} else if (!m_Schedule) {
				cChannel* Channel = NULL;
				if (isnumber(p))
					Channel = Channels.GetByNumber(strtol(Option, NULL, 10));
				else
					Channel = Channels.GetByChannelID(tChannelID::FromString(
					                                  Option));
				if (Channel) {
					m_Schedule = m_Schedules->GetSchedule(Channel->GetChannelID());
					if (!m_Schedule) {
						m_Errno = 550;
						m_Error = strdup("No schedule found");
						break;
					}
				} else {
					m_Errno = 550;
					asprintf(&m_Error, "Channel \"%s\" not defined", p);
					break;
				}
			} else {
				m_Errno = 501;
				asprintf(&m_Error, "Unknown option: \"%s\"", p);
				break;
			}
			p = strtok_r(NULL, delim, &strtok_next);
		}
	} else if (m_Schedules == NULL) {
		m_Errno = 451;
		m_Error = strdup("EPG data is being modified, try again");
	}

	if (m_Error == NULL) {
		if (m_Schedule != NULL)
			m_Schedules = NULL;
		else if (m_Schedules != NULL)
			m_Schedule = m_Schedules->First();

		if (m_Schedule != NULL && m_Schedule->Events() != NULL) {
			switch (dumpmode) {
			case dmAll:       m_Event = m_Schedule->Events()->First();
							  m_Traverse = true;
							  break;
			case dmPresent:   m_Event = m_Schedule->GetPresentEvent();
							  break;
			case dmFollowing: m_Event = m_Schedule->GetFollowingEvent();
							  break;
			case dmAtTime:    m_Event = m_Schedule->GetEventAround(attime);
							  break;

			}
		}
	}
}

cLSTEHandler::~cLSTEHandler()
{
	delete m_SchedulesLock;
	if (m_Error != NULL)
		free(m_Error);
}

bool cLSTEHandler::Next(bool &Last)
{
	char *buffer;

	if (m_Error != NULL) {
		Last = true;
		cString str(m_Error, true);
		m_Error = NULL;
		return m_Client->Respond(m_Errno, *str);
	}

	Last = false;
	switch (m_State) {
	case Channel:
		if (m_Schedule != NULL) {
			cChannel *channel = Channels.GetByChannelID(m_Schedule->ChannelID(),
			                                            true);
			m_State = Event;
			return m_Client->Respond(-215, "C %s %s", 
			                         *channel->GetChannelID().ToString(),
			                         channel->Name());
		} else {
			m_State = EndEPG;
			return Next(Last);
		}
		break;

	case Event:
		if (m_Event != NULL) {
			m_State = Title;
			return m_Client->Respond(-215, "E %u %ld %d %X", m_Event->EventID(),
			                         m_Event->StartTime(), m_Event->Duration(), 
			                         m_Event->TableID());
		} else {
			m_State = EndChannel;
			return Next(Last);
		}
		break;

	case Title:
		m_State = Subtitle;
		if (!isempty(m_Event->Title()))
			return m_Client->Respond(-215, "T %s", m_Event->Title());
		else
			return Next(Last);
		break;

	case Subtitle:
		m_State = Description;
		if (!isempty(m_Event->ShortText()))
			return m_Client->Respond(-215, "S %s", m_Event->ShortText());
		else
			return Next(Last);
		break;

	case Description:
		m_State = Vps;
		if (!isempty(m_Event->Description())) {
			char *copy = strdup(m_Event->Description());
			cString cpy(copy, true);
			strreplace(copy, '\n', '|');
			return m_Client->Respond(-215, "D %s", copy);
		} else
			return Next(Last);
		break;

	case Vps:
		m_State = EndEvent;
		if (m_Event->Vps())
			return m_Client->Respond(-215, "V %ld", m_Event->Vps());
		else
			return Next(Last);
		break;

	case EndEvent:
		if (m_Traverse)
			m_Event = m_Schedule->Events()->Next(m_Event);
		else
			m_Event = NULL;

		if (m_Event != NULL)
			m_State = Event;
		else
			m_State = EndChannel;

		return m_Client->Respond(-215, "e");

	case EndChannel:
		if (m_Schedules != NULL) {
			m_Schedule = m_Schedules->Next(m_Schedule);
			if (m_Schedule != NULL) {
				if (m_Schedule->Events() != NULL)
					m_Event = m_Schedule->Events()->First();
				m_State = Channel;
			}
		} 
		
		if (m_Schedules == NULL || m_Schedule == NULL)
			m_State = EndEPG;

		return m_Client->Respond(-215, "c");

	case EndEPG:
		Last = true;
		return m_Client->Respond(215, "End of EPG data");
	}
	return false;
}

// --- cLSTCHandler -----------------------------------------------------------

class cLSTCHandler 
{
private:
	cConnectionVTP *m_Client;
	const cChannel *m_Channel;
	char           *m_Option;
	int             m_Errno;
	char           *m_Error;
	bool            m_Traverse;
public:
	cLSTCHandler(cConnectionVTP *Client, const char *Option);
	~cLSTCHandler();
	bool Next(bool &Last);
};

cLSTCHandler::cLSTCHandler(cConnectionVTP *Client, const char *Option):
		m_Client(Client),
		m_Channel(NULL),
		m_Option(NULL),
		m_Errno(0),
		m_Error(NULL),
		m_Traverse(false)
{
	if (!Channels.Lock(false, 500)) {
		m_Errno = 451;
		m_Error = strdup("Channels are being modified - try again");
	} else if (*Option) {
		if (isnumber(Option)) {
			m_Channel = Channels.GetByNumber(strtol(Option, NULL, 10));
			if (m_Channel == NULL) {
				m_Errno = 501;
				asprintf(&m_Error, "Channel \"%s\" not defined", Option);
				return;
			}
		} else {
			int i = 1;
			m_Traverse = true;
			m_Option = strdup(Option);
			while (i <= Channels.MaxNumber()) {
				m_Channel = Channels.GetByNumber(i, 1);
				if (strcasestr(m_Channel->Name(), Option) != NULL)
					break;
				i = m_Channel->Number() + 1;
			}

			if (i > Channels.MaxNumber()) {
				m_Errno = 501;
				asprintf(&m_Error, "Channel \"%s\" not defined", Option);
				return;
			}
		}
	} else if (Channels.MaxNumber() >= 1) {
		m_Channel = Channels.GetByNumber(1, 1);
		m_Traverse = true;
	} else {
		m_Errno = 550;
		m_Error = strdup("No channels defined");
	}
}

cLSTCHandler::~cLSTCHandler()
{
	Channels.Unlock();
	if (m_Error != NULL)
		free(m_Error);
	if (m_Option != NULL)
		free(m_Option);
}

bool cLSTCHandler::Next(bool &Last)
{
	if (m_Error != NULL) {
		Last = true;
		cString str(m_Error, true);
		m_Error = NULL;
		return m_Client->Respond(m_Errno, *str);
	}

	int number;
	char *buffer;

	number = m_Channel->Number();
	buffer = strdup(*m_Channel->ToText());
	buffer[strlen(buffer) - 1] = '\0'; // remove \n
	cString str(buffer, true);

	Last = true;
	if (m_Traverse) {
		int i = m_Channel->Number() + 1;
		while (i <= Channels.MaxNumber()) {
			m_Channel = Channels.GetByNumber(i, 1);
			if (m_Channel != NULL) {
				if (m_Option == NULL || strcasestr(m_Channel->Name(), 
												   m_Option) != NULL)
					break;
				i = m_Channel->Number() + 1;
			} else {
				m_Errno = 501;
				asprintf(&m_Error, "Channel \"%d\" not found", i);
			}
		}

		if (i < Channels.MaxNumber())
			Last = false;
	}

	return m_Client->Respond(Last ? 250 : -250, "%d %s", number, buffer);
}

// --- cLSTTHandler -----------------------------------------------------------

class cLSTTHandler 
{
private:
	cConnectionVTP *m_Client;
	cTimer         *m_Timer;
	int             m_Index;
	int             m_Errno;
	char           *m_Error;
	bool            m_Traverse;
public:
	cLSTTHandler(cConnectionVTP *Client, const char *Option);
	~cLSTTHandler();
	bool Next(bool &Last);
};

cLSTTHandler::cLSTTHandler(cConnectionVTP *Client, const char *Option):
		m_Client(Client),
		m_Timer(NULL),
		m_Index(0),
		m_Errno(0),
		m_Error(NULL),
		m_Traverse(false)
{
	if (*Option) {
		if (isnumber(Option)) {
			m_Timer = Timers.Get(strtol(Option, NULL, 10) - 1);
			if (m_Timer == NULL) {
				m_Errno = 501;
				asprintf(&m_Error, "Timer \"%s\" not defined", Option);
			}
		} else {
			m_Errno = 501;
			asprintf(&m_Error, "Error in timer number \"%s\"", Option);
		}
	} else if (Timers.Count()) {
		m_Traverse = true;
		m_Index = 0;
		m_Timer = Timers.Get(m_Index);
		if (m_Timer == NULL) {
			m_Errno = 501;
			asprintf(&m_Error, "Timer \"%d\" not found", m_Index + 1);
		}
	} else {
		m_Errno = 550;
		m_Error = strdup("No timers defined");
	}
}

cLSTTHandler::~cLSTTHandler()
{
	if (m_Error != NULL)
		free(m_Error);
}

bool cLSTTHandler::Next(bool &Last)
{
	if (m_Error != NULL) {
		Last = true;
		cString str(m_Error, true);
		m_Error = NULL;
		return m_Client->Respond(m_Errno, *str);
	}

	bool result;
	char *buffer;
	Last = !m_Traverse || m_Index >= Timers.Count() - 1;
	buffer = strdup(*m_Timer->ToText());
	buffer[strlen(buffer) - 1] = '\0'; // strip \n
	result = m_Client->Respond(Last ? 250 : -250, "%d %s", m_Timer->Index() + 1,
	                           buffer);
	free(buffer);

	if (m_Traverse && !Last) {
		m_Timer = Timers.Get(++m_Index);
		if (m_Timer == NULL) {
			m_Errno = 501;
			asprintf(&m_Error, "Timer \"%d\" not found", m_Index + 1);
		}
	}
	return result;
}

// --- cConnectionVTP ---------------------------------------------------------

cConnectionVTP::cConnectionVTP(void): 
		cServerConnection("VTP"),
		m_LiveSocket(NULL),
		m_LiveStreamer(NULL),
		m_LastCommand(NULL),
		m_NoTSPIDS(false),
		m_LSTEHandler(NULL),
		m_LSTCHandler(NULL),
		m_LSTTHandler(NULL)
{
}

cConnectionVTP::~cConnectionVTP() 
{
	if (m_LastCommand != NULL) 
		free(m_LastCommand);
	delete m_LiveStreamer;
	delete m_LiveSocket;
	delete m_LSTTHandler;
	delete m_LSTCHandler;
	delete m_LSTEHandler;
}

void cConnectionVTP::Welcome(void) 
{
	Respond(220, "Welcome to Video Disk Recorder (VTP)");
}

void cConnectionVTP::Reject(void)
{
	Respond(221, "Too many clients or client not allowed to connect");
	cServerConnection::Reject();
}

void cConnectionVTP::Detach(void) 
{
	if (m_LiveStreamer != NULL) m_LiveStreamer->Detach();
}

void cConnectionVTP::Attach(void) 
{
	if (m_LiveStreamer != NULL) m_LiveStreamer->Attach();
}

bool cConnectionVTP::Command(char *Cmd) 
{
	char *param = NULL;

	if (Cmd != NULL) {
		if (m_LastCommand != NULL) {
			esyslog("ERROR: streamdev: protocol violation (VTP) from %s:%d",
					RemoteIp().c_str(), RemotePort());
			return false;
		}

		if ((param = strchr(Cmd, ' ')) != NULL)
			*(param++) = '\0';
		else 
			param = Cmd + strlen(Cmd);
		m_LastCommand = strdup(Cmd);
	} else {
		Cmd = m_LastCommand;
		param = NULL;
	}
	
	if      (strcasecmp(Cmd, "LSTE") == 0) return CmdLSTE(param);
	//else if (strcasecmp(Cmd, "LSTR") == 0) return CmdLSTR(param);
	else if (strcasecmp(Cmd, "LSTT") == 0) return CmdLSTT(param);
	else if (strcasecmp(Cmd, "LSTC") == 0) return CmdLSTC(param);

	if (param == NULL) {
		esyslog("ERROR: streamdev: this seriously shouldn't happen at %s:%d",
		        __FILE__, __LINE__);
		return false;
	}

	if      (strcasecmp(Cmd, "CAPS") == 0) return CmdCAPS(param);
	else if (strcasecmp(Cmd, "PROV") == 0) return CmdPROV(param);
	else if (strcasecmp(Cmd, "PORT") == 0) return CmdPORT(param);
	else if (strcasecmp(Cmd, "TUNE") == 0) return CmdTUNE(param);
	else if (strcasecmp(Cmd, "ADDP") == 0) return CmdADDP(param);
	else if (strcasecmp(Cmd, "DELP") == 0) return CmdDELP(param);
	else if (strcasecmp(Cmd, "ADDF") == 0) return CmdADDF(param);
	else if (strcasecmp(Cmd, "DELF") == 0) return CmdDELF(param);
	else if (strcasecmp(Cmd, "ABRT") == 0) return CmdABRT(param);
	else if (strcasecmp(Cmd, "QUIT") == 0) return CmdQUIT(param);
	else if (strcasecmp(Cmd, "SUSP") == 0) return CmdSUSP(param);
	// Commands adopted from SVDRP
	//else if (strcasecmp(Cmd, "DELR") == 0) return CmdDELR(param);
	else if (strcasecmp(Cmd, "MODT") == 0) return CmdMODT(param);
	else if (strcasecmp(Cmd, "NEWT") == 0) return CmdNEWT(param);
	else if (strcasecmp(Cmd, "DELT") == 0) return CmdDELT(param);
	else
		return Respond(500, "Unknown Command \"%s\"", Cmd);
}

bool cConnectionVTP::CmdCAPS(char *Opts) 
{
	char *buffer;

	if (strcasecmp(Opts, "TS") == 0) {
		m_NoTSPIDS = true;
		return Respond(220, "Ignored, capability \"%s\" accepted for "
		               "compatibility", Opts);
	}

	if (strcasecmp(Opts, "TSPIDS") == 0) {
		m_NoTSPIDS = false;
		return Respond(220, "Capability \"%s\" accepted", Opts);
	}

	return Respond(561, "Capability \"%s\" not known", Opts);
}

bool cConnectionVTP::CmdPROV(char *Opts) 
{
	const cChannel *chan;
	int prio;
	char *ep;
	
	prio = strtol(Opts, &ep, 10);
	if (ep == Opts || !isspace(*ep))
		return Respond(501, "Use: PROV Priority Channel");

	Opts = skipspace(ep);
	if ((chan = ChannelFromString(Opts)) == NULL)
		return Respond(550, "Undefined channel \"%s\"", Opts);

	return GetDevice(chan, prio) != NULL
			? Respond(220, "Channel available")
			: Respond(560, "Channel not available");
}

bool cConnectionVTP::CmdPORT(char *Opts) 
{
	uint id, dataport = 0;
	char dataip[20];
	char *ep, *ipoffs;
	int n;

	id = strtoul(Opts, &ep, 10);
	if (ep == Opts || !isspace(*ep))
		return Respond(500, "Use: PORT Id Destination");

	if (id != 0)
		return Respond(501, "Wrong connection id %d", id);
	
	Opts = skipspace(ep);
	n = 0;
	ipoffs = dataip;
	while ((ep = strchr(Opts, ',')) != NULL) {
		if (n < 4) {
			memcpy(ipoffs, Opts, ep - Opts);
			ipoffs += ep - Opts;
			if (n < 3) *(ipoffs++) = '.';
		} else if (n == 4) {
			*ep = 0;
			dataport = strtoul(Opts, NULL, 10) << 8;
		} else
			break;
		Opts = ep + 1;
		++n;
	}
	*ipoffs = '\0';

	if (n != 5)
		return Respond(501, "Argument count invalid (must be 6 values)");

	dataport |= strtoul(Opts, NULL, 10);

	isyslog("Streamdev: Setting data connection to %s:%d", dataip, dataport);

	m_LiveSocket = new cTBSocket(SOCK_STREAM);
	if (!m_LiveSocket->Connect(dataip, dataport)) {
		esyslog("ERROR: Streamdev: Couldn't open data connection to %s:%d: %s",
				dataip, dataport, strerror(errno));
		DELETENULL(m_LiveSocket);
		return Respond(551, "Couldn't open data connection");
	}

	if (id == siLive)
		m_LiveStreamer->Start(m_LiveSocket);

  return Respond(220, "Port command ok, data connection opened");
}

bool cConnectionVTP::CmdTUNE(char *Opts) 
{
	const cChannel *chan;
	cDevice *dev;
	
	if ((chan = ChannelFromString(Opts)) == NULL)
		return Respond(550, "Undefined channel \"%s\"", Opts);

	if ((dev = GetDevice(chan, 0)) == NULL)
		return Respond(560, "Channel not available");

	if (!dev->SwitchChannel(chan, false))
		return Respond(560, "Channel not available");

	delete m_LiveStreamer;
	m_LiveStreamer = new cStreamdevLiveStreamer(1);
	m_LiveStreamer->SetChannel(chan, m_NoTSPIDS ? stTS : stTSPIDS);
	m_LiveStreamer->SetDevice(dev);
	
	return Respond(220, "Channel tuned");
}

bool cConnectionVTP::CmdADDP(char *Opts) 
{
	int pid;
	char *end;

	pid = strtoul(Opts, &end, 10);
	if (end == Opts || (*end != '\0' && *end != ' '))
		return Respond(500, "Use: ADDP Pid");

	return m_LiveStreamer && m_LiveStreamer->SetPid(pid, true)
			? Respond(220, "Pid %d available", pid)
			: Respond(560, "Pid %d not available", pid);
}

bool cConnectionVTP::CmdDELP(char *Opts) 
{
	int pid;
	char *end;

	pid = strtoul(Opts, &end, 10);
	if (end == Opts || (*end != '\0' && *end != ' '))
		return Respond(500, "Use: DELP Pid");

	return m_LiveStreamer && m_LiveStreamer->SetPid(pid, false)
			? Respond(220, "Pid %d stopped", pid)
			: Respond(560, "Pid %d not transferring", pid);
}

bool cConnectionVTP::CmdADDF(char *Opts) 
{
#if VDRVERSNUM >= 10300
	int pid, tid, mask;
	char *ep;

	if (m_LiveStreamer == NULL)
		return Respond(560, "Can't set filters without a stream");
	
	pid = strtol(Opts, &ep, 10);
	if (ep == Opts || (*ep != ' '))
		return Respond(500, "Use: ADDF Pid Tid Mask");
	Opts = skipspace(ep);
	tid = strtol(Opts, &ep, 10);
	if (ep == Opts || (*ep != ' '))
		return Respond(500, "Use: ADDF Pid Tid Mask");
	Opts = skipspace(ep);
	mask = strtol(Opts, &ep, 10);
	if (ep == Opts || (*ep != '\0' && *ep != ' '))
		return Respond(500, "Use: ADDF Pid Tid Mask");

	return m_LiveStreamer->SetFilter(pid, tid, mask, true)
			? Respond(220, "Filter %d transferring", pid)
			: Respond(560, "Filter %d not available", pid);
#else
	return Respond(500, "ADDF known but unimplemented with VDR < 1.3.0");
#endif
}

bool cConnectionVTP::CmdDELF(char *Opts) 
{
#if VDRVERSNUM >= 10307
	int pid, tid, mask;
	char *ep;
	
	if (m_LiveStreamer == NULL)
		return Respond(560, "Can't delete filters without a stream");
	
	pid = strtol(Opts, &ep, 10);
	if (ep == Opts || (*ep != ' '))
		return Respond(500, "Use: DELF Pid Tid Mask");
	Opts = skipspace(ep);
	tid = strtol(Opts, &ep, 10);
	if (ep == Opts || (*ep != ' '))
		return Respond(500, "Use: DELF Pid Tid Mask");
	Opts = skipspace(ep);
	mask = strtol(Opts, &ep, 10);
	if (ep == Opts || (*ep != '\0' && *ep != ' '))
		return Respond(500, "Use: DELF Pid Tid Mask");

	return m_LiveStreamer->SetFilter(pid, tid, mask, false)
			? Respond(220, "Filter %d stopped", pid)
			: Respond(560, "Filter %d not transferring", pid);
#else
	return Respond(500, "DELF known but unimplemented with VDR < 1.3.0");
#endif
}

bool cConnectionVTP::CmdABRT(char *Opts) 
{
	uint id;
	char *ep;

	id = strtoul(Opts, &ep, 10);
	if (ep == Opts || (*ep != '\0' && *ep != ' '))
		return Respond(500, "Use: ABRT Id");

	switch (id) {
	case 0: DELETENULL(m_LiveStreamer); break;
	}

	DELETENULL(m_LiveSocket);
	return Respond(220, "Data connection closed");
}

bool cConnectionVTP::CmdQUIT(char *Opts) 
{
	DeferClose();
	return Respond(221, "Video Disk Recorder closing connection");
}

bool cConnectionVTP::CmdSUSP(char *Opts) 
{
	if (StreamdevServerSetup.SuspendMode == smAlways || cSuspendCtl::IsActive())
		return Respond(220, "Server is suspended");
	else if (StreamdevServerSetup.SuspendMode == smOffer 
			&& StreamdevServerSetup.AllowSuspend) {
		cControl::Launch(new cSuspendCtl);
		return Respond(220, "Server is suspended");
	} else
		return Respond(550, "Client may not suspend server");
}

// Functions extended from SVDRP

template<class cHandler>
bool cConnectionVTP::CmdLSTX(cHandler *&Handler, char *Option)
{
	if (Option != NULL) {
		delete Handler;
		Handler = new cHandler(this, Option);
	}

	bool last, result = Handler->Next(last);
	if (!result || last)
		DELETENULL(Handler);

	return result;
}

bool cConnectionVTP::CmdLSTE(char *Option) 
{
	return CmdLSTX(m_LSTEHandler, Option);
}

bool cConnectionVTP::CmdLSTC(char *Option)
{
	return CmdLSTX(m_LSTCHandler, Option);
}

bool cConnectionVTP::CmdLSTT(char *Option)
{
	return CmdLSTX(m_LSTTHandler, Option);
}

// Functions adopted from SVDRP
#define INIT_WRAPPER() bool _res
#define Reply(c,m...) _res = Respond(c,m)
#define EXIT_WRAPPER() return _res

bool cConnectionVTP::CmdMODT(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		char *tail;
		int n = strtol(Option, &tail, 10);
		if (tail && tail != Option) {
			tail = skipspace(tail);
			cTimer *timer = Timers.Get(n - 1);
			if (timer) {
				cTimer t = *timer;
				if (strcasecmp(tail, "ON") == 0)
					t.SetFlags(tfActive);
				else if (strcasecmp(tail, "OFF") == 0)
					t.ClrFlags(tfActive);
				else if (!t.Parse(tail)) {
					Reply(501, "Error in timer settings");
					EXIT_WRAPPER();
				}
				*timer = t;
				Timers.SetModified();
				isyslog("timer %s modified (%s)", *timer->ToDescr(), 
				        timer->HasFlags(tfActive) ? "active" : "inactive");
				Reply(250, "%d %s", timer->Index() + 1, *timer->ToText());
			} else
				Reply(501, "Timer \"%d\" not defined", n);
		} else
			Reply(501, "Error in timer number");
	} else
		Reply(501, "Missing timer settings");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdNEWT(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		cTimer *timer = new cTimer;
		if (timer->Parse(Option)) {
			cTimer *t = Timers.GetTimer(timer);
			if (!t) {
				Timers.Add(timer);
				Timers.SetModified();
				isyslog("timer %s added", *timer->ToDescr());
				Reply(250, "%d %s", timer->Index() + 1, *timer->ToText());
				EXIT_WRAPPER();
			} else
				Reply(550, "Timer already defined: %d %s", t->Index() + 1, 
				      *t->ToText());
		} else
			Reply(501, "Error in timer settings");
		delete timer;
	} else
		Reply(501, "Missing timer settings");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdDELT(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		if (isnumber(Option)) {
			cTimer *timer = Timers.Get(strtol(Option, NULL, 10) - 1);
			if (timer) {
				if (!timer->Recording()) {
					isyslog("deleting timer %s", *timer->ToDescr());
					Timers.Del(timer);
					Timers.SetModified();
					Reply(250, "Timer \"%s\" deleted", Option);
				} else
					Reply(550, "Timer \"%s\" is recording", Option);
			} else
				Reply(501, "Timer \"%s\" not defined", Option);
		} else
			Reply(501, "Error in timer number \"%s\"", Option);
	} else
		Reply(501, "Missing timer number");
	EXIT_WRAPPER();
}

/*bool cConnectionVTP::CmdLSTR(char *Option) {
	INIT_WRAPPER();
  bool recordings = Recordings.Load();
	Recordings.Sort();
  if (*Option) {
     if (isnumber(Option)) {
        cRecording *recording = Recordings.Get(strtol(Option, NULL, 10) - 1);
        if (recording) {
           if (recording->Summary()) {
              char *summary = strdup(recording->Summary());
              Reply(250, "%s", strreplace(summary,'\n','|'));
              free(summary);
              }
           else
              Reply(550, "No summary availabe");
           }
        else
           Reply(550, "Recording \"%s\" not found", Option);
        }
     else
        Reply(501, "Error in recording number \"%s\"", Option);
     }
  else if (recordings) {
     cRecording *recording = Recordings.First();
     while (recording) {
           Reply(recording == Recordings.Last() ? 250 : -250, "%d %s", recording->Index() + 1, recording->Title(' ', true));
           recording = Recordings.Next(recording);
           }
     }
  else
     Reply(550, "No recordings available");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdDELR(char *Option) {
	INIT_WRAPPER();
  if (*Option) {
     if (isnumber(Option)) {
        cRecording *recording = Recordings.Get(strtol(Option, NULL, 10) - 1);
        if (recording) {
           if (recording->Delete())
              Reply(250, "Recording \"%s\" deleted", Option);
           else
              Reply(554, "Error while deleting recording!");
           }
        else
           Reply(550, "Recording \"%s\" not found%s", Option, Recordings.Count() ? "" : " (use LSTR before deleting)");
        }
     else
        Reply(501, "Error in recording number \"%s\"", Option);
     }
  else
     Reply(501, "Missing recording number");
	EXIT_WRAPPER();
}*/

bool cConnectionVTP::Respond(int Code, const char *Message, ...)
{
	char *buffer;
	va_list ap;
	va_start(ap, Message);
	vasprintf(&buffer, Message, ap);
	va_end(ap);
	cString str(buffer, true);

	if (Code >= 0 && m_LastCommand != NULL) {
		free(m_LastCommand);
		m_LastCommand = NULL;
	}

	return cServerConnection::Respond("%03d%c%s", Code >= 0, 
	                                  Code < 0 ? -Code : Code,
									  Code < 0 ? '-' : ' ', buffer);
}
