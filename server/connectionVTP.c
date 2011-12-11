/*
 *  $Id: connectionVTP.c,v 1.31 2010/08/18 10:26:54 schmirl Exp $
 */
 
#include "server/connectionVTP.h"
#include "server/livestreamer.h"
#include "server/suspend.h"
#include "setup.h"

#include <vdr/tools.h>
#include <vdr/videodir.h>
#include <vdr/menu.h>
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

enum eDumpModeStreamdev { dmsdAll, dmsdPresent, dmsdFollowing, dmsdAtTime, dmsdFromToTime };

// --- cLSTEHandler -----------------------------------------------------------

class cLSTEHandler 
{
private:
#if defined(USE_PARENTALRATING) || defined(PARENTALRATINGCONTENTVERSNUM)
	enum eStates { Channel, Event, Title, Subtitle, Description, Vps, Content,
	               EndEvent, EndChannel, EndEPG };
#elif APIVERSNUM >= 10711
	enum eStates { Channel, Event, Title, Subtitle, Description, Vps, Content, Rating,
	               EndEvent, EndChannel, EndEPG };
#else
	enum eStates { Channel, Event, Title, Subtitle, Description, Vps, 
	               EndEvent, EndChannel, EndEPG };
#endif /* PARENTALRATING */
	cConnectionVTP    *m_Client;
	cSchedulesLock    *m_SchedulesLock;
	const cSchedules  *m_Schedules;
	const cSchedule   *m_Schedule;
	const cEvent      *m_Event;
	int                m_Errno;
	cString            m_Error;
	eStates            m_State;
	bool               m_Traverse;
	time_t             m_ToTime;
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
		m_State(Channel),
		m_Traverse(false),
		m_ToTime(0)
{
	eDumpModeStreamdev dumpmode = dmsdAll;
	time_t attime = 0;
	time_t fromtime = 0;

	if (m_Schedules != NULL && *Option) {
		char buf[strlen(Option) + 1];
		strcpy(buf, Option);
		const char *delim = " \t";
		char *strtok_next;
		char *p = strtok_r(buf, delim, &strtok_next);
		while (p && dumpmode == dmsdAll) {
			if (strcasecmp(p, "NOW") == 0)
				dumpmode = dmsdPresent;
			else if (strcasecmp(p, "NEXT") == 0)
				dumpmode = dmsdFollowing;
			else if (strcasecmp(p, "AT") == 0) {
				dumpmode = dmsdAtTime;
				if ((p = strtok_r(NULL, delim, &strtok_next)) != NULL) {
					if (isnumber(p))
						attime = strtol(p, NULL, 10);
					else {
						m_Errno = 501;
						m_Error = "Invalid time";
						break;
					}
				} else {
					m_Errno = 501;
					m_Error = "Missing time";
					break;
				}
			}
			else if (strcasecmp(p, "FROM") == 0) {
				dumpmode = dmsdFromToTime;
				if ((p = strtok_r(NULL, delim, &strtok_next)) != NULL) {
					if (isnumber(p))
						fromtime = strtol(p, NULL, 10);
					else {
						m_Errno = 501;
						m_Error = "Invalid time";
						break;
					}
					if ((p = strtok_r(NULL, delim, &strtok_next)) != NULL) {
						if (strcasecmp(p, "TO") == 0) {
							if ((p = strtok_r(NULL, delim, &strtok_next)) != NULL) {
								if (isnumber(p))
									m_ToTime = strtol(p, NULL, 10);
								else {
									m_Errno = 501;
								m_Error = "Invalid time";
								break;
								}
							} else {
								m_Errno = 501;
								m_Error = "Missing time";
								break;
							}
						}
					}
				} else {
					m_Errno = 501;
					m_Error = "Missing time";
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
						m_Error = "No schedule found";
						break;
					}
				} else {
					m_Errno = 550;
					m_Error = cString::sprintf("Channel \"%s\" not defined", p);
					break;
				}
			} else {
				m_Errno = 501;
				m_Error = cString::sprintf("Unknown option: \"%s\"", p);
				break;
			}
			p = strtok_r(NULL, delim, &strtok_next);
		}
	} else if (m_Schedules == NULL) {
		m_Errno = 451;
		m_Error = "EPG data is being modified, try again";
	}

	if (*m_Error == NULL) {
		if (m_Schedule != NULL)
			m_Schedules = NULL;
		else if (m_Schedules != NULL)
			m_Schedule = m_Schedules->First();

		if (m_Schedule != NULL && m_Schedule->Events() != NULL) {
			switch (dumpmode) {
			case dmsdAll:       m_Event = m_Schedule->Events()->First();
						m_Traverse = true;
						break;
			case dmsdPresent:   m_Event = m_Schedule->GetPresentEvent();
						break;
			case dmsdFollowing: m_Event = m_Schedule->GetFollowingEvent();
						break;
			case dmsdAtTime:    m_Event = m_Schedule->GetEventAround(attime);
						break;
			case dmsdFromToTime:
						if (m_Schedule->Events()->Count() <= 1) {
							m_Event = m_Schedule->Events()->First();
							break;
						}
						if (fromtime < m_Schedule->Events()->First()->StartTime()) {
							fromtime = m_Schedule->Events()->First()->StartTime();
						}
						if (m_ToTime > m_Schedule->Events()->Last()->EndTime()) {
							m_ToTime = m_Schedule->Events()->Last()->EndTime();
						}
						m_Event = m_Schedule->GetEventAround(fromtime);
						m_Traverse = true;
						break;
			}
		}
	}
}

cLSTEHandler::~cLSTEHandler()
{
	delete m_SchedulesLock;
}

bool cLSTEHandler::Next(bool &Last)
{
	if (*m_Error != NULL) {
		Last = true;
		cString str(m_Error);
		m_Error = NULL;
		return m_Client->Respond(m_Errno, "%s", *str);
	}

	Last = false;
	switch (m_State) {
	case Channel:
		if (m_Schedule != NULL) {
			cChannel *channel = Channels.GetByChannelID(m_Schedule->ChannelID(),
			                                            true);
			if (channel != NULL) {
				m_State = Event;
				return m_Client->Respond(-215, "C %s %s", 
			                         *channel->GetChannelID().ToString(),
			                         channel->Name());
			} else {
				esyslog("ERROR: vdr streamdev: unable to find channel %s by ID",
					         *m_Schedule->ChannelID().ToString());
				m_State = EndChannel;
				return Next(Last);
			}
		} else {
			m_State = EndEPG;
			return Next(Last);
		}
		break;

	case Event:
		if (m_Event != NULL) {
			m_State = Title;
#ifdef __FreeBSD__
			return m_Client->Respond(-215, "E %u %d %d %X", m_Event->EventID(),
#else
			return m_Client->Respond(-215, "E %u %ld %d %X", m_Event->EventID(),
#endif
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
#if defined(USE_PARENTALRATING) || defined(PARENTALRATINGCONTENTVERSNUM) || APIVERSNUM >= 10711
		m_State = Content;
#else
		m_State = EndEvent;
#endif /* PARENTALRATING */
		if (m_Event->Vps())
#ifdef __FreeBSD__
			return m_Client->Respond(-215, "V %d", m_Event->Vps());
#else
			return m_Client->Respond(-215, "V %ld", m_Event->Vps());
#endif
		else
			return Next(Last);
		break;

#if defined(USE_PARENTALRATING) || defined(PARENTALRATINGCONTENTVERSNUM)
	case Content:
		m_State = EndEvent;
		if (!isempty(m_Event->GetContentsString())) {
			char *copy = strdup(m_Event->GetContentsString());
			cString cpy(copy, true);
			strreplace(copy, '\n', '|');
			return m_Client->Respond(-215, "G %i %i %s", m_Event->Contents() & 0xF0, m_Event->Contents() & 0x0F, copy);
		} else
			return Next(Last);
		break;
#elif APIVERSNUM >= 10711
	case Content:
		m_State = Rating;
		if (!isempty(m_Event->ContentToString(m_Event->Contents()))) {
			char *copy = strdup(m_Event->ContentToString(m_Event->Contents()));
			cString cpy(copy, true);
			strreplace(copy, '\n', '|');
			return m_Client->Respond(-215, "G %i %i %s", m_Event->Contents() & 0xF0, m_Event->Contents() & 0x0F, copy);
		} else
			return Next(Last);
		break;

	case Rating:
		m_State = EndEvent;
		if (m_Event->ParentalRating())
			return m_Client->Respond(-215, "R %d", m_Event->ParentalRating());
		else
			return Next(Last);
		break;
#endif

	case EndEvent:
		if (m_Traverse) {
			m_Event = m_Schedule->Events()->Next(m_Event);
			if ((m_Event != NULL) && (m_ToTime != 0) && (m_Event->StartTime() > m_ToTime)) {
				m_Event = NULL;
			}
		}
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
	cString         m_Error;
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
		m_Traverse(false)
{
	if (!Channels.Lock(false, 500)) {
		m_Errno = 451;
		m_Error = "Channels are being modified - try again";
	} else if (*Option) {
		if (isnumber(Option)) {
			m_Channel = Channels.GetByNumber(strtol(Option, NULL, 10));
			if (m_Channel == NULL) {
				m_Errno = 501;
				m_Error = cString::sprintf("Channel \"%s\" not defined", Option);
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
				m_Error = cString::sprintf("Channel \"%s\" not defined", Option);
				return;
			}
		}
	} else if (Channels.MaxNumber() >= 1) {
		m_Channel = Channels.GetByNumber(1, 1);
		m_Traverse = true;
	} else {
		m_Errno = 550;
		m_Error = "No channels defined";
	}
}

cLSTCHandler::~cLSTCHandler()
{
	Channels.Unlock();
	if (m_Option != NULL)
		free(m_Option);
}

bool cLSTCHandler::Next(bool &Last)
{
	if (*m_Error != NULL) {
		Last = true;
		cString str(m_Error);
		m_Error = NULL;
		return m_Client->Respond(m_Errno, "%s", *str);
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
				m_Error = cString::sprintf("Channel \"%d\" not found", i);
			}
		}

		if (i < Channels.MaxNumber() + 1)
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
	cString         m_Error;
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
		m_Traverse(false)
{
	if (*Option) {
		if (isnumber(Option)) {
			m_Timer = Timers.Get(strtol(Option, NULL, 10) - 1);
			if (m_Timer == NULL) {
				m_Errno = 501;
				m_Error = cString::sprintf("Timer \"%s\" not defined", Option);
			}
		} else {
			m_Errno = 501;
			m_Error = cString::sprintf("Error in timer number \"%s\"", Option);
		}
	} else if (Timers.Count()) {
		m_Traverse = true;
		m_Index = 0;
		m_Timer = Timers.Get(m_Index);
		if (m_Timer == NULL) {
			m_Errno = 501;
			m_Error = cString::sprintf("Timer \"%d\" not found", m_Index + 1);
		}
	} else {
		m_Errno = 550;
		m_Error = "No timers defined";
	}
}

cLSTTHandler::~cLSTTHandler()
{
}

bool cLSTTHandler::Next(bool &Last)
{
	if (*m_Error != NULL) {
		Last = true;
		cString str(m_Error);
		m_Error = NULL;
		return m_Client->Respond(m_Errno, "%s", *str);
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
			m_Error = cString::sprintf("Timer \"%d\" not found", m_Index + 1);
		}
	}
	return result;
}

// --- cLSTRHandler -----------------------------------------------------------

class cLSTRHandler 
{
private:
	enum eStates { Recording, Event, Title, Subtitle, Description, Components, Vps, 
	               EndRecording };
	cConnectionVTP *m_Client;
	cRecording     *m_Recording;
	const cEvent   *m_Event;
	int             m_Index;
	int             m_Errno;
	cString         m_Error;
	bool            m_Traverse;
	bool            m_Info;
	eStates         m_State;
	int             m_CurrentComponent;
public:
	cLSTRHandler(cConnectionVTP *Client, const char *Option);
	~cLSTRHandler();
	bool Next(bool &Last);
};

cLSTRHandler::cLSTRHandler(cConnectionVTP *Client, const char *Option):
		m_Client(Client),
		m_Recording(NULL),
		m_Event(NULL),
		m_Index(0),
		m_Errno(0),
		m_Traverse(false),
		m_Info(false),
		m_State(Recording),
		m_CurrentComponent(0)
{
	if (*Option) {
		if (isnumber(Option)) {
			m_Recording = Recordings.Get(strtol(Option, NULL, 10) - 1);
#if defined(USE_STREAMDEVEXT) || APIVERSNUM >= 10705
			m_Event = m_Recording->Info()->GetEvent();
#endif
			m_Info = true;
			if (m_Recording == NULL) {
				m_Errno = 501;
				m_Error = cString::sprintf("Recording \"%s\" not found", Option);
			}
		}
		else {
			m_Errno = 501;
			m_Error = cString::sprintf("Error in Recording number \"%s\"", Option);
		}
	} 
	else if (Recordings.Count()) {
		m_Traverse = true;
		m_Index = 0;
		m_Recording = Recordings.Get(m_Index);
		if (m_Recording == NULL) {
			m_Errno = 501;
			m_Error = cString::sprintf("Recording \"%d\" not found", m_Index + 1);
		}
	} 
	else {
		m_Errno = 550;
		m_Error = "No recordings available";
	}
}

cLSTRHandler::~cLSTRHandler()
{
}

bool cLSTRHandler::Next(bool &Last)
{
	if (*m_Error != NULL) {
		Last = true;
		cString str(m_Error);
		m_Error = NULL;
		return m_Client->Respond(m_Errno, "%s", *str);
	}
	
	if (m_Info) {
		Last = false;
		switch (m_State) {
		case Recording:
			if (m_Recording != NULL) {
				m_State = Event;
				return m_Client->Respond(-215, "C %s%s%s", 
						*m_Recording->Info()->ChannelID().ToString(), 
						m_Recording->Info()->ChannelName() ? " " : "", 
						m_Recording->Info()->ChannelName() ? m_Recording->Info()->ChannelName() : "");
			} 
			else {
				m_State = EndRecording;
				return Next(Last);
			}
			break;

		case Event:
			m_State = Title;
			if (m_Event != NULL) {
				return m_Client->Respond(-215, "E %u %ld %d %X %X", (unsigned int) m_Event->EventID(), 
						m_Event->StartTime(), m_Event->Duration(), 
						m_Event->TableID(), m_Event->Version());
			} 
			return Next(Last);

		case Title:
			m_State = Subtitle;
			return m_Client->Respond(-215, "T %s", m_Recording->Info()->Title());

		case Subtitle:
			m_State = Description;
			if (!isempty(m_Recording->Info()->ShortText())) {
				return m_Client->Respond(-215, "S %s", m_Recording->Info()->ShortText());
			}
			return Next(Last);

		case Description:
			m_State = Components;
			if (!isempty(m_Recording->Info()->Description())) {
				m_State = Components;
				char *copy = strdup(m_Recording->Info()->Description());
				cString cpy(copy, true);
				strreplace(copy, '\n', '|');
				return m_Client->Respond(-215, "D %s", copy);
			}
			return Next(Last);

		case Components:
			if (m_Recording->Info()->Components()) {
				if (m_CurrentComponent < m_Recording->Info()->Components()->NumComponents()) {
					tComponent *p = m_Recording->Info()->Components()->Component(m_CurrentComponent);
					m_CurrentComponent++;
					if (!Setup.UseDolbyDigital && p->stream == 0x02 && p->type == 0x05)
						return Next(Last);

					return m_Client->Respond(-215, "X %s", *p->ToString());
				}
			}
			m_State = Vps;
			return Next(Last);

		case Vps:
			m_State = EndRecording;
			if (m_Event != NULL) {
				if (m_Event->Vps()) {
					return m_Client->Respond(-215, "V %ld", m_Event->Vps());
				}
			}
			return Next(Last);

		case EndRecording:
			Last = true;
			return m_Client->Respond(215, "End of recording information");
		}
	}
	else {
		bool result;
		Last = !m_Traverse || m_Index >= Recordings.Count() - 1;
		result = m_Client->Respond(Last ? 250 : -250, "%d %s", m_Recording->Index() + 1, m_Recording->Title(' ', true));

		if (m_Traverse && !Last) {
			m_Recording = Recordings.Get(++m_Index);
			if (m_Recording == NULL) {
				m_Errno = 501;
				m_Error = cString::sprintf("Recording \"%d\" not found", m_Index + 1);
			}
		}
		return result;
	}
	return false;
}

class cStreamdevLoopPrevention {
private:
	bool Unlock;
public:
	cStreamdevLoopPrevention(const cChannel* Channel, bool LoopPrevention): Unlock(LoopPrevention) {
		if (LoopPrevention)
			cPluginManager::CallAllServices(LOOP_PREVENTION_SERVICE, (void *)Channel);
	}
	~cStreamdevLoopPrevention() {
		if (Unlock)
			cPluginManager::CallAllServices(LOOP_PREVENTION_SERVICE, NULL);
	}
};

// --- cConnectionVTP ---------------------------------------------------------

#define LOOP_PREVENTION(c) cStreamdevLoopPrevention LoopPrevention(c, m_LoopPrevention);

cConnectionVTP::cConnectionVTP(void): 
		cServerConnection("VTP"),
		m_LiveSocket(NULL),
		m_LiveStreamer(NULL),
		m_FilterSocket(NULL),
		m_FilterStreamer(NULL),
		m_RecSocket(NULL),
		m_DataSocket(NULL),
		m_LastCommand(NULL),
		m_StreamType(stTSPIDS),
		m_FiltersSupport(false),
		m_RecPlayer(NULL),
		m_TuneChannel(NULL),
		m_TunePriority(0),
		m_LSTEHandler(NULL),
		m_LSTCHandler(NULL),
		m_LSTTHandler(NULL),
		m_LSTRHandler(NULL)
{
	m_LoopPrevention = StreamdevServerSetup.LoopPrevention;
	if (m_LoopPrevention)
		// Loop prevention enabled - but is there anybody out there?
		m_LoopPrevention = cPluginManager::CallFirstService(LOOP_PREVENTION_SERVICE);
}

cConnectionVTP::~cConnectionVTP() 
{
	if (m_LastCommand != NULL) 
		free(m_LastCommand);
	delete m_LiveStreamer;
	delete m_LiveSocket;
	delete m_RecSocket;
	delete m_FilterStreamer;
	delete m_FilterSocket;
	delete m_DataSocket;
	delete m_LSTTHandler;
	delete m_LSTCHandler;
	delete m_LSTEHandler;
	delete m_LSTRHandler;
	delete m_RecPlayer;
}

bool cConnectionVTP::Abort(void) const
{
	return !IsOpen() || (m_LiveStreamer && m_LiveStreamer->Abort()) ||
		(m_FilterStreamer && m_FilterStreamer->Abort());
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
	if (m_LiveStreamer) m_LiveStreamer->Detach();
	if (m_FilterStreamer) m_FilterStreamer->Detach();
}

void cConnectionVTP::Attach(void) 
{
	if (m_LiveStreamer) m_LiveStreamer->Attach();
	if (m_FilterStreamer) m_FilterStreamer->Attach();
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
	else if (strcasecmp(Cmd, "LSTR") == 0) return CmdLSTR(param);
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
	else if (strcasecmp(Cmd, "READ") == 0) return CmdREAD(param);
	else if (strcasecmp(Cmd, "TUNE") == 0) return CmdTUNE(param);
	else if (strcasecmp(Cmd, "PLAY") == 0) return CmdPLAY(param);
	else if (strcasecmp(Cmd, "PRIO") == 0) return CmdPRIO(param);
	else if (strcasecmp(Cmd, "SGNL") == 0) return CmdSGNL(param);
	else if (strcasecmp(Cmd, "ADDP") == 0) return CmdADDP(param);
	else if (strcasecmp(Cmd, "DELP") == 0) return CmdDELP(param);
	else if (strcasecmp(Cmd, "ADDF") == 0) return CmdADDF(param);
	else if (strcasecmp(Cmd, "DELF") == 0) return CmdDELF(param);
	else if (strcasecmp(Cmd, "ABRT") == 0) return CmdABRT(param);
	else if (strcasecmp(Cmd, "QUIT") == 0) return CmdQUIT();
	else if (strcasecmp(Cmd, "SUSP") == 0) return CmdSUSP();
	// Commands adopted from SVDRP
	else if (strcasecmp(Cmd, "STAT") == 0) return CmdSTAT(param);
	else if (strcasecmp(Cmd, "MODT") == 0) return CmdMODT(param);
	else if (strcasecmp(Cmd, "NEWT") == 0) return CmdNEWT(param);
	else if (strcasecmp(Cmd, "DELT") == 0) return CmdDELT(param);
	else if (strcasecmp(Cmd, "NEXT") == 0) return CmdNEXT(param);
	else if (strcasecmp(Cmd, "NEWC") == 0) return CmdNEWC(param);
	else if (strcasecmp(Cmd, "MODC") == 0) return CmdMODC(param);
	else if (strcasecmp(Cmd, "MOVC") == 0) return CmdMOVC(param);
	else if (strcasecmp(Cmd, "DELC") == 0) return CmdDELC(param);
	else if (strcasecmp(Cmd, "DELR") == 0) return CmdDELR(param);
	else if (strcasecmp(Cmd, "RENR") == 0) return CmdRENR(param);
	else
		return Respond(500, "Unknown Command \"%s\"", Cmd);
}

bool cConnectionVTP::CmdCAPS(char *Opts) 
{
	if (strcasecmp(Opts, "TS") == 0) {
		m_StreamType = stTS;
		return Respond(220, "Capability \"%s\" accepted", Opts);
	}

	if (strcasecmp(Opts, "TSPIDS") == 0) {
		m_StreamType = stTSPIDS;
		return Respond(220, "Capability \"%s\" accepted", Opts);
	}

	if (strcasecmp(Opts, "PS") == 0) {
		m_StreamType = stPS;
		return Respond(220, "Capability \"%s\" accepted", Opts);
	}

	if (strcasecmp(Opts, "PES") == 0) {
		m_StreamType = stPES;
		return Respond(220, "Capability \"%s\" accepted", Opts);
	}

	if (strcasecmp(Opts, "EXT") == 0) {
		m_StreamType = stEXT;
		return Respond(220, "Capability \"%s\" accepted", Opts);
	}

	//
	// Deliver section filters data in separate, channel-independent data stream
	//
	if (strcasecmp(Opts, "FILTERS") == 0) {
		m_FiltersSupport = true;
		return Respond(220, "Capability \"%s\" accepted", Opts);
	}

	// Command PRIO is known
	if (strcasecmp(Opts, "PRIO") == 0) {
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

	LOOP_PREVENTION(chan);

	if (ProvidesChannel(chan, prio)) {
		m_TuneChannel = chan;
		m_TunePriority = prio;
		return Respond(220, "Channel available");
	}
	else {
		m_TuneChannel = NULL;
		return Respond(560, "Channel not available");
	}
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
	
	if (id >= si_Count)
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

	switch (id) {
	case siLiveFilter:
		m_FiltersSupport = true;
		if(m_FilterStreamer)
			m_FilterStreamer->Stop();
		delete m_FilterSocket;

		m_FilterSocket = new cTBSocket(SOCK_STREAM);
		if (!m_FilterSocket->Connect(dataip, dataport)) {
			esyslog("ERROR: Streamdev: Couldn't open data connection to %s:%d: %s",
				dataip, dataport, strerror(errno));
			DELETENULL(m_FilterSocket);
			return Respond(551, "Couldn't open data connection");
		}

		if(!m_FilterStreamer)
			m_FilterStreamer = new cStreamdevFilterStreamer;
		m_FilterStreamer->Start(m_FilterSocket);
		m_FilterStreamer->Activate(true);

		return Respond(220, "Port command ok, data connection opened");
		break;

	case siLive:
		if(m_LiveSocket && m_LiveStreamer)
			m_LiveStreamer->Stop();
		delete m_LiveSocket;

		m_LiveSocket = new cTBSocket(SOCK_STREAM);
		if (!m_LiveSocket->Connect(dataip, dataport)) {
			esyslog("ERROR: Streamdev: Couldn't open data connection to %s:%d: %s",
					dataip, dataport, strerror(errno));
			DELETENULL(m_LiveSocket);
			return Respond(551, "Couldn't open data connection");
		}

		if (!m_LiveSocket->SetDSCP())
			LOG_ERROR_STR("unable to set DSCP sockopt");
		if (m_LiveStreamer)
			m_LiveStreamer->Start(m_LiveSocket);

		return Respond(220, "Port command ok, data connection opened");
		break;

	case siReplay:
		delete m_RecSocket;

		m_RecSocket = new cTBSocket(SOCK_STREAM);
		if (!m_RecSocket->Connect(dataip, dataport)) {
			esyslog("ERROR: Streamdev: Couldn't open data connection to %s:%d: %s",
					dataip, dataport, strerror(errno));
			DELETENULL(m_RecSocket);
			return Respond(551, "Couldn't open data connection");
		}

		if (!m_RecSocket->SetDSCP())
			LOG_ERROR_STR("unable to set DSCP sockopt");

		return Respond(220, "Port command ok, data connection opened");
		break;

	case siDataRespond:
		delete m_DataSocket;

		m_DataSocket = new cTBSocket(SOCK_STREAM);
		if (!m_DataSocket->Connect(dataip, dataport)) {
			esyslog("ERROR: Streamdev: Couldn't open data connection to %s:%d: %s",
					dataip, dataport, strerror(errno));
			DELETENULL(m_DataSocket);
			return Respond(551, "Couldn't open data connection");
		}
		return Respond(220, "Port command ok, data connection opened");
		break;

	default:
		return Respond(501, "No handler for id %u", id);
	}
}

bool cConnectionVTP::CmdREAD(char *Opts)
{
	if (*Opts) {
		char *tail;
		uint64_t position = strtoll(Opts, &tail, 10);
		if (tail && tail != Opts) {
			tail = skipspace(tail);
			if (tail && tail != Opts) {
				int size = strtol(tail, NULL, 10);
				uint8_t* data = (uint8_t*)malloc(size+4);
				unsigned long count_readed = m_RecPlayer->getBlock(data, position, size);
				unsigned long count_written = m_RecSocket->SysWrite(data, count_readed);

				free(data);
				return Respond(220, "%lu Bytes submitted", count_written);
			}
			else {
				return Respond(501, "Missing position");
			}
		}
		else {
			return Respond(501, "Missing size");
		}
	}
	else {
		return Respond(501, "Missing position");
	}
}

bool cConnectionVTP::CmdTUNE(char *Opts) 
{
	const cChannel *chan;
	cDevice *dev;
	int prio = m_TunePriority;
	
	if ((chan = ChannelFromString(Opts)) == NULL)
		return Respond(550, "Undefined channel \"%s\"", Opts);

	LOOP_PREVENTION(chan);

	if (chan != m_TuneChannel) {
		isyslog("streamdev-server TUNE %s: Priority unknown - using 0", Opts);
		prio = 0;
		if (!ProvidesChannel(chan, prio))
			return Respond(560, "Channel not available (ProvidesChannel)");
	}
	if ((dev = GetDevice(chan, prio)) == NULL)
		return Respond(560, "Channel not available (GetDevice)");

	if (!dev->SwitchChannel(chan, false))
		return Respond(560, "Channel not available (SwitchChannel)");

	delete m_LiveStreamer;
	m_LiveStreamer = new cStreamdevLiveStreamer(prio, this);
	m_LiveStreamer->SetChannel(chan, m_StreamType);
	m_LiveStreamer->SetDevice(dev);
	if(m_LiveSocket)
		m_LiveStreamer->Start(m_LiveSocket);
	
	if(m_FiltersSupport) {
		if(!m_FilterStreamer)
			m_FilterStreamer = new cStreamdevFilterStreamer;
		m_FilterStreamer->SetDevice(dev);
		//m_FilterStreamer->SetChannel(chan);
	}

	return Respond(220, "Channel tuned");
}

bool cConnectionVTP::CmdPLAY(char *Opts)
{
	if (*Opts) {
		if (isnumber(Opts)) {
			cRecording *recording = Recordings.Get(strtol(Opts, NULL, 10) - 1);
			if (recording) {
				if (m_RecPlayer) {
					delete m_RecPlayer;
				}
				m_RecPlayer = new RecPlayer(recording);
				return Respond(220, "%llu (Bytes), %u (Frames)", (long long unsigned int) m_RecPlayer->getLengthBytes(), (unsigned int) m_RecPlayer->getLengthFrames());
			}
			else {
				return Respond(550, "Recording \"%s\" not found", Opts);
			}
		}
		else {
			return Respond(500, "Use: PLAY record");
		}
	}
	else {
		return Respond(500, "Use: PLAY record");
	}
}

bool cConnectionVTP::CmdPRIO(char *Opts) 
{
	int prio;
	char *end;

	prio = strtoul(Opts, &end, 10);
	if (end == Opts || (*end != '\0' && *end != ' '))
		return Respond(500, "Use: PRIO Priority");

	if (m_LiveStreamer) {
		m_LiveStreamer->SetPriority(prio);
		return Respond(220, "Priority changed to %d", prio);
	}
	return Respond(550, "Priority not applicable");
}

bool cConnectionVTP::CmdSGNL(char *Opts) 
{
	if (m_LiveStreamer) {
		int devnum = -1;
		int signal = -1;
		int quality = -1;
		m_LiveStreamer->GetSignal(&devnum, &signal, &quality);
		return Respond(220, "%d %d:%d", devnum, signal, quality);
	}
	return Respond(550, "Signal not applicable");
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
	int pid, tid, mask;
	char *ep;

	if (m_FilterStreamer == NULL)
		return Respond(560, "Can't set filters without a filter stream");
	
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

	return m_FilterStreamer->SetFilter(pid, tid, mask, true)
			? Respond(220, "Filter %d transferring", pid)
			: Respond(560, "Filter %d not available", pid);
}

bool cConnectionVTP::CmdDELF(char *Opts) 
{
	int pid, tid, mask;
	char *ep;
	
	if (m_FilterStreamer == NULL)
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

	m_FilterStreamer->SetFilter(pid, tid, mask, false);
	return Respond(220, "Filter %d stopped", pid);
}

bool cConnectionVTP::CmdABRT(char *Opts) 
{
	uint id;
	char *ep;

	id = strtoul(Opts, &ep, 10);
	if (ep == Opts || (*ep != '\0' && *ep != ' '))
		return Respond(500, "Use: ABRT Id");

	switch (id) {
	case siLive: 
		DELETENULL(m_LiveStreamer); 
		DELETENULL(m_LiveSocket);
		break;
	case siLiveFilter:
		DELETENULL(m_FilterStreamer);
		DELETENULL(m_FilterSocket);
		break;
	case siReplay:
		DELETENULL(m_RecPlayer);
		DELETENULL(m_RecSocket);
		break;
	case siDataRespond:
		DELETENULL(m_DataSocket);
		break;
	default:
		return Respond(501, "Wrong connection id %d", id);
		break;

	}

	return Respond(220, "Data connection closed");
}

bool cConnectionVTP::CmdQUIT(void) 
{
	DeferClose();
	return Respond(221, "Video Disk Recorder closing connection");
}

bool cConnectionVTP::CmdSUSP(void) 
{
	if (StreamdevServerSetup.SuspendMode == smAlways || cSuspendCtl::IsActive())
		return Respond(220, "Server is suspended");
	else if (StreamdevServerSetup.SuspendMode == smOffer 
			&& StreamdevServerSetup.AllowSuspend) {
		cControl::Launch(new cSuspendCtl);
		cControl::Attach();
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

	bool last = false;
	bool result = false;
	if (Handler != NULL)
		result = Handler->Next(last);
	else
		esyslog("ERROR: vdr streamdev: Handler in LSTX command is NULL");
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

bool cConnectionVTP::CmdLSTR(char *Option)
{
	return CmdLSTX(m_LSTRHandler, Option);
}

// Functions adopted from SVDRP
#define INIT_WRAPPER() bool _res
#define Reply(c,m...) _res = Respond(c,m)
#define EXIT_WRAPPER() return _res

bool cConnectionVTP::CmdSTAT(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		if (strcasecmp(Option, "DISK") == 0) {
			int FreeMB, UsedMB;
			int Percent = VideoDiskSpace(&FreeMB, &UsedMB);
			Reply(250, "%dMB %dMB %d%%", FreeMB + UsedMB, FreeMB, Percent);
		}
		else if (strcasecmp(Option, "NAME") == 0) {
			Reply(250, "vdr - The Video Disk Recorder with Streamdev-Server");
		}
		else if (strcasecmp(Option, "VERSION") == 0) {
			Reply(250, "VDR: %s | Streamdev: %s", VDRVERSION, VERSION);
		}
		else if (strcasecmp(Option, "RECORDS") == 0) {
			bool recordings = Recordings.Load();
			Recordings.Sort();
			if (recordings) {
				cRecording *recording = Recordings.Last();
				Reply(250, "%d", recording->Index() + 1);
			}
			else {
				Reply(250, "0");
			}
		}
		else if (strcasecmp(Option, "CHANNELS") == 0) {
			Reply(250, "%d", Channels.MaxNumber());
		}
		else if (strcasecmp(Option, "TIMERS") == 0) {
			Reply(250, "%d", Timers.Count());
		}
		else if (strcasecmp(Option, "CHARSET") == 0) {
			Reply(250, "%s", cCharSetConv::SystemCharacterTable());
		}
		else if (strcasecmp(Option, "TIME") == 0) {
			time_t timeNow = time(NULL);
			struct tm* timeStruct = localtime(&timeNow);
			int timeOffset = timeStruct->tm_gmtoff;

			Reply(250, "%lu %i", (unsigned long) timeNow, timeOffset);
		}
		else
			Reply(501, "Invalid Option \"%s\"", Option);
	}
	else
		Reply(501, "No option given");
	EXIT_WRAPPER();
}

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
		int number = 0;
		bool force = false;
		char buf[strlen(Option) + 1];
		strcpy(buf, Option);
		const char *delim = " \t";
		char *strtok_next;
		char *p = strtok_r(buf, delim, &strtok_next);

		if (isnumber(p)) {
			number = strtol(p, NULL, 10) - 1;
		}
		else if (strcasecmp(p, "FORCE") == 0) {
			force = true;
		}
		if ((p = strtok_r(NULL, delim, &strtok_next)) != NULL) {
			if (isnumber(p)) {
				number = strtol(p, NULL, 10) - 1;
			}
			else if (strcasecmp(p, "FORCE") == 0) {
				force = true;
			}
			else {
				Reply(501, "Timer not found or wrong syntax");
			}
		}

		cTimer *timer = Timers.Get(number);
			if (timer) {
			if (timer->Recording()) {
				if (force) {
					timer->Skip();
					cRecordControls::Process(time(NULL));
				}
				else {
					Reply(550, "Timer \"%i\" is recording", number);
					EXIT_WRAPPER();
				}
			}
					isyslog("deleting timer %s", *timer->ToDescr());
					Timers.Del(timer);
					Timers.SetModified();
			Reply(250, "Timer \"%i\" deleted", number);
				} else
			Reply(501, "Timer \"%i\" not defined", number);
			} else
		Reply(501, "Missing timer option");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdNEXT(const char *Option)
{
	INIT_WRAPPER();
	cTimer *t = Timers.GetNextActiveTimer();
	if (t) {
		time_t Start = t->StartTime();
		int Number = t->Index() + 1;
		if (!*Option)
			Reply(250, "%d %s", Number, *TimeToString(Start));
		else if (strcasecmp(Option, "ABS") == 0)
			Reply(250, "%d %ld", Number, Start);
		else if (strcasecmp(Option, "REL") == 0)
			Reply(250, "%d %ld", Number, Start - time(NULL));
		else
			Reply(501, "Unknown option: \"%s\"", Option);
	}
	else
		Reply(550, "No active timers");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdNEWC(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		cChannel ch;
		if (ch.Parse(Option)) {
			if (Channels.HasUniqueChannelID(&ch)) {
				cChannel *channel = new cChannel;
				*channel = ch;
				Channels.Add(channel);
				Channels.ReNumber();
				Channels.SetModified(true);
				isyslog("new channel %d %s", channel->Number(), *channel->ToText());
				Reply(250, "%d %s", channel->Number(), *channel->ToText());
			}
			else {
				Reply(501, "Channel settings are not unique");
			}
		}
		else {
			Reply(501, "Error in channel settings");
		}
	}
	else {
		Reply(501, "Missing channel settings");
	}
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdMODC(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		char *tail;
		int n = strtol(Option, &tail, 10);
		if (tail && tail != Option) {
			tail = skipspace(tail);
			if (!Channels.BeingEdited()) {
				cChannel *channel = Channels.GetByNumber(n);
				if (channel) {
					cChannel ch;
					if (ch.Parse(tail)) {
						if (Channels.HasUniqueChannelID(&ch, channel)) {
							*channel = ch;
							Channels.ReNumber();
							Channels.SetModified(true);
							isyslog("modifed channel %d %s", channel->Number(), *channel->ToText());
							Reply(250, "%d %s", channel->Number(), *channel->ToText());
						}
						else {
							Reply(501, "Channel settings are not unique");
						}
					}
					else {
						Reply(501, "Error in channel settings");
					}
				}
				else {
					Reply(501, "Channel \"%d\" not defined", n);
				}
			}
			else {
				Reply(550, "Channels are being edited - try again later");
			}
		}
		else {
			Reply(501, "Error in channel number");
		}
	}
	else {
		Reply(501, "Missing channel settings");
	}
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdMOVC(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		if (!Channels.BeingEdited() && !Timers.BeingEdited()) {
			char *tail;
			int From = strtol(Option, &tail, 10);
			if (tail && tail != Option) {
				tail = skipspace(tail);
				if (tail && tail != Option) {
					int To = strtol(tail, NULL, 10);
					int CurrentChannelNr = cDevice::CurrentChannel();
					cChannel *CurrentChannel = Channels.GetByNumber(CurrentChannelNr);
					cChannel *FromChannel = Channels.GetByNumber(From);
					if (FromChannel) {
						cChannel *ToChannel = Channels.GetByNumber(To);
						if (ToChannel) {
							int FromNumber = FromChannel->Number();
							int ToNumber = ToChannel->Number();
							if (FromNumber != ToNumber) {
								Channels.Move(FromChannel, ToChannel);
								Channels.ReNumber();
								Channels.SetModified(true);
								if (CurrentChannel && CurrentChannel->Number() != CurrentChannelNr) {
									if (!cDevice::PrimaryDevice()->Replaying() || cDevice::PrimaryDevice()->Transferring()) {
										Channels.SwitchTo(CurrentChannel->Number());
									}
									else {
										cDevice::SetCurrentChannel(CurrentChannel);
									}
								}
								isyslog("channel %d moved to %d", FromNumber, ToNumber);
								Reply(250,"Channel \"%d\" moved to \"%d\"", From, To);
							}
							else {
								Reply(501, "Can't move channel to same postion");
							}
						}
						else {
							Reply(501, "Channel \"%d\" not defined", To);
						}
					}
					else {
						Reply(501, "Channel \"%d\" not defined", From);
					}
				}
				else {
					Reply(501, "Error in channel number");
				}
			}
			else {
				Reply(501, "Error in channel number");
			}
		}
		else {
			Reply(550, "Channels or timers are being edited - try again later");
		}
	}
	else {
		Reply(501, "Missing channel number");
	}
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdDELC(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		if (isnumber(Option)) {
			if (!Channels.BeingEdited()) {
				cChannel *channel = Channels.GetByNumber(strtol(Option, NULL, 10));
				if (channel) {
					for (cTimer *timer = Timers.First(); timer; timer = Timers.Next(timer)) {
						if (timer->Channel() == channel) {
							Reply(550, "Channel \"%s\" is in use by timer %d", Option, timer->Index() + 1);
							return false;
						}
					}
					int CurrentChannelNr = cDevice::CurrentChannel();
					cChannel *CurrentChannel = Channels.GetByNumber(CurrentChannelNr);
					if (CurrentChannel && channel == CurrentChannel) {
						int n = Channels.GetNextNormal(CurrentChannel->Index());
						if (n < 0)
							n = Channels.GetPrevNormal(CurrentChannel->Index());
						CurrentChannel = Channels.Get(n);
						CurrentChannelNr = 0; // triggers channel switch below
					}
					Channels.Del(channel);
					Channels.ReNumber();
					Channels.SetModified(true);
					isyslog("channel %s deleted", Option);
					if (CurrentChannel && CurrentChannel->Number() != CurrentChannelNr) {
						if (!cDevice::PrimaryDevice()->Replaying() || cDevice::PrimaryDevice()->Transferring())
							Channels.SwitchTo(CurrentChannel->Number());
						else
							cDevice::SetCurrentChannel(CurrentChannel);
					}
					Reply(250, "Channel \"%s\" deleted", Option);
				}
				else
					Reply(501, "Channel \"%s\" not defined", Option);
			}
			else
				Reply(550, "Channels are being edited - try again later");
		}
		else
			Reply(501, "Error in channel number \"%s\"", Option);
	}
	else {
		Reply(501, "Missing channel number");
	}
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdDELR(const char *Option)
{
	INIT_WRAPPER();
	if (*Option) {
		if (isnumber(Option)) {
			cRecording *recording = Recordings.Get(strtol(Option, NULL, 10) - 1);
			if (recording) {
				cRecordControl *rc = cRecordControls::GetRecordControl(recording->FileName());
				if (!rc) {
					if (recording->Delete()) {
						Reply(250, "Recording \"%s\" deleted", Option);
						::Recordings.DelByName(recording->FileName());
					}
					else
						Reply(554, "Error while deleting recording!");
				}
				else
					Reply(550, "Recording \"%s\" is in use by timer %d", Option, rc->Timer()->Index() + 1);
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
}

bool cConnectionVTP::CmdRENR(const char *Option)
{
	INIT_WRAPPER();
#if defined(LIEMIKUUTIO) && LIEMIKUUTIO < 132
	bool recordings = Recordings.Update(true);
	if (recordings) {
		if (*Option) {
			char *tail;
			int n = strtol(Option, &tail, 10);
			cRecording *recording = Recordings.Get(n - 1);
			if (recording && tail && tail != Option) {
#if APIVERSNUM < 10704
				int priority = recording->priority;
				int lifetime = recording->lifetime;
#endif
				char *oldName = strdup(recording->Name());
				tail = skipspace(tail);
#if APIVERSNUM < 10704
				if (recording->Rename(tail, &priority, &lifetime)) {
#else
				if (recording->Rename(tail)) {
#endif
					Reply(250, "Renamed \"%s\" to \"%s\"", oldName, recording->Name());
					Recordings.ChangeState();
					Recordings.TouchUpdate();
				}
				else {
					Reply(501, "Renaming \"%s\" to \"%s\" failed", oldName, tail);
				}
				free(oldName);
			}
			else {
				Reply(501, "Recording not found or wrong syntax");
			}
		}
		else {
			Reply(501, "Missing Input settings");
		}
	}
	else {
		Reply(550, "No recordings available");
	}
#else
	Reply(501, "Rename not supported, please use LIEMIKUUTIO < 1.32");
#endif /* LIEMIKUUTIO */
	EXIT_WRAPPER();
}

bool cConnectionVTP::Respond(int Code, const char *Message, ...)
{
	va_list ap;
	va_start(ap, Message);
	cString str = cString::sprintf(Message, ap);
	va_end(ap);

	if (Code >= 0 && m_LastCommand != NULL) {
		free(m_LastCommand);
		m_LastCommand = NULL;
	}

	return cServerConnection::Respond("%03d%c%s", Code >= 0, 
				Code < 0 ? -Code : Code,
				Code < 0 ? '-' : ' ', *str);
}

cString cConnectionVTP::ToText() const
{
	cString str = cServerConnection::ToText();
	if (m_LiveStreamer)
		return cString::sprintf("%s\t%s", *str, *m_LiveStreamer->ToText());
	else if (m_RecPlayer)
		return cString::sprintf("%s\t%s", *str, m_RecPlayer->getCurrentRecording()->Name());
	else
		return str;
}
