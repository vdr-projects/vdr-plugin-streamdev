/*
 *  $Id: connectionVTP.c,v 1.2 2005/02/08 13:59:16 lordjaxom Exp $
 */
 
#include "server/connectionVTP.h"
#include "server/livestreamer.h"
#include "server/suspend.h"
#include "setup.h"

#include <vdr/tools.h>
#include <tools/select.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

/* VTP Response codes:
	220: Service ready
	221: Service closing connection
	500: Syntax error or Command unrecognized
	501: Wrong parameters or missing parameters
	550: Action not done
	551: Data connection not accepted
	560: Channel not available currently
  561: Capability not known
	562: Pid not available currently
	563: Recording not available (currently?)
*/

cConnectionVTP::cConnectionVTP(void): cServerConnection("VTP") {
	m_StreamPIDS   = false;
	m_LiveStreamer = NULL;
	m_ClientCaps   = stTS;
	
	memset(m_DataSockets, 0, sizeof(cTBSocket*) * si_Count);
}

cConnectionVTP::~cConnectionVTP() {
	if (m_LiveStreamer != NULL) delete m_LiveStreamer;

	for (int idx = 0; idx < si_Count; ++idx)
		if (m_DataSockets[idx] != NULL) delete m_DataSockets[idx];
}

void cConnectionVTP::Welcome(void) {
	Respond(220, "Welcome to Video Disk Recorder (VTP)");
}

void cConnectionVTP::Reject(void) {
	Respond(221, "Too many clients or client not allowed to connect");
	cServerConnection::Reject();
}

void cConnectionVTP::Detach(void) {
	if (m_LiveStreamer != NULL) m_LiveStreamer->Detach();
}

void cConnectionVTP::Attach(void) {
	if (m_LiveStreamer != NULL) m_LiveStreamer->Attach();
}

bool cConnectionVTP::Command(char *Cmd) {
	char *ep;
	if ((ep = strchr(Cmd, ' ')) != NULL)
		*(ep++) = '\0';
	else 
		ep = Cmd + strlen(Cmd);

	if      (strcasecmp(Cmd, "CAPS") == 0) return CmdCAPS(ep);
	else if (strcasecmp(Cmd, "PROV") == 0) return CmdPROV(ep);
	else if (strcasecmp(Cmd, "PORT") == 0) return CmdPORT(ep);
	else if (strcasecmp(Cmd, "TUNE") == 0) return CmdTUNE(ep);
	else if (strcasecmp(Cmd, "ADDP") == 0) return CmdADDP(ep);
	else if (strcasecmp(Cmd, "DELP") == 0) return CmdDELP(ep);
	else if (strcasecmp(Cmd, "ADDF") == 0) return CmdADDF(ep);
	else if (strcasecmp(Cmd, "DELF") == 0) return CmdDELF(ep);
	else if (strcasecmp(Cmd, "ABRT") == 0) return CmdABRT(ep);
	else if (strcasecmp(Cmd, "QUIT") == 0) return CmdQUIT(ep);
	else if (strcasecmp(Cmd, "SUSP") == 0) return CmdSUSP(ep);
	// Commands adopted from SVDRP
	else if (strcasecmp(Cmd, "LSTE") == 0) return CmdLSTE(ep);
	else if (strcasecmp(Cmd, "LSTR") == 0) return CmdLSTR(ep);
	else if (strcasecmp(Cmd, "DELR") == 0) return CmdDELR(ep);
	else if (strcasecmp(Cmd, "LSTT") == 0) return CmdLSTT(ep);
	else if (strcasecmp(Cmd, "MODT") == 0) return CmdMODT(ep);
	else if (strcasecmp(Cmd, "NEWT") == 0) return CmdNEWT(ep);
	else if (strcasecmp(Cmd, "DELT") == 0) return CmdDELT(ep);
	else
		return Respond(500, (cTBString)"Unknown Command '" + Cmd + "'");
}

bool cConnectionVTP::CmdCAPS(char *Opts) {
	if (strcasecmp(Opts, "TSPIDS") == 0) m_StreamPIDS = true;
	else {
		int idx = 0;
		while (idx < st_Count && strcasecmp(Opts, StreamTypes[idx]) != 0)
				++idx;
		
		if (idx == st_Count)
			return Respond(561, (cTBString)"Capability \"" + Opts + "\" not known");

		m_ClientCaps = (eStreamType)idx;
	}
	return Respond(220, (cTBString)"Capability \"" + Opts + "\" accepted");
}

bool cConnectionVTP::CmdPROV(char *Opts) {
	cChannel *chan;
	int prio;
	char *ep;
	
	prio = strtol(Opts, &ep, 10);
	if (ep == Opts || !isspace(*ep))
		return Respond(501, "Use: PROV Priority Channel");

	Opts = skipspace(ep);
	if ((chan = ChannelFromString(Opts)) == NULL)
		return Respond(550, (cTBString)"Undefined channel \"" + Opts + "\"");

	return GetDevice(chan, prio) != NULL
			? Respond(220, "Channel available")
			: Respond(560, "Channel not available");
}

bool cConnectionVTP::CmdPORT(char *Opts) {
	uint id, dataport = 0;
	char dataip[20];
	char *ep, *ipoffs;
	int n;

	id = strtoul(Opts, &ep, 10);
	if (ep == Opts || !isspace(*ep))
		return Respond(500, "Use: PORT Id Destination");

	if (id >= si_Count)
		return Respond(501, "Wrong connection id " + cTBString::Number(id));
	
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

	m_DataSockets[id] = new cTBSocket(SOCK_STREAM);
	if (!m_DataSockets[id]->Connect(dataip, dataport)) {
		esyslog("ERROR: Streamdev: Couldn't open data connection to %s:%d: %s",
				dataip, dataport, strerror(errno));
		DELETENULL(m_DataSockets[id]);
		return Respond(551, "Couldn't open data connection");
	}

	if (id == siLive)
		m_LiveStreamer->Start(m_DataSockets[id]);

  return Respond(220, "Port command ok, data connection opened");
}

bool cConnectionVTP::CmdTUNE(char *Opts) {
	const cChannel *chan;
	cDevice *dev;
	
	if ((chan = ChannelFromString(Opts)) == NULL)
		return Respond(550, (cTBString)"Undefined channel \"" + Opts + "\"");

	if ((dev = GetDevice(chan, 0)) == NULL)
		return Respond(560, "Channel not available");

	if (!dev->SwitchChannel(chan, false))
		return Respond(560, "Channel not available");

	delete m_LiveStreamer;
	m_LiveStreamer = new cStreamdevLiveStreamer(1);
	m_LiveStreamer->SetChannel(chan, m_ClientCaps, m_StreamPIDS);
	m_LiveStreamer->SetDevice(dev);
	
	return Respond(220, "Channel tuned");
}

bool cConnectionVTP::CmdADDP(char *Opts) {
	int pid;
	char *end;

	pid = strtoul(Opts, &end, 10);
	if (end == Opts || (*end != '\0' && *end != ' '))
		return Respond(500, "Use: ADDP Pid");

	return m_LiveStreamer && m_LiveStreamer->SetPid(pid, true)
			? Respond(220, "Pid " + cTBString::Number(pid) + " available")
			: Respond(560, "Pid " + cTBString::Number(pid) + " not available");
}

bool cConnectionVTP::CmdDELP(char *Opts) {
	int pid;
	char *end;

	pid = strtoul(Opts, &end, 10);
	if (end == Opts || (*end != '\0' && *end != ' '))
		return Respond(500, "Use: DELP Pid");

	return m_LiveStreamer && m_LiveStreamer->SetPid(pid, false)
			? Respond(220, "Pid " + cTBString::Number(pid) + " stopped")
			: Respond(560, "Pid " + cTBString::Number(pid) + " not transferring");
}

bool cConnectionVTP::CmdADDF(char *Opts) {
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
			? Respond(220, "Filter " + cTBString::Number(pid) + " transferring")
			: Respond(560, "Filter " + cTBString::Number(pid) + " not available");
#else
	return Respond(500, "ADDF known but unimplemented with VDR < 1.3.0");
#endif
}

bool cConnectionVTP::CmdDELF(char *Opts) {
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
			? Respond(220, "Filter " + cTBString::Number(pid) + " stopped")
			: Respond(560, "Filter " + cTBString::Number(pid) + " not transferring");
#else
	return Respond(500, "DELF known but unimplemented with VDR < 1.3.0");
#endif
}

bool cConnectionVTP::CmdABRT(char *Opts) {
	uint id;
	char *ep;

	id = strtoul(Opts, &ep, 10);
	if (ep == Opts || (*ep != '\0' && *ep != ' '))
		return Respond(500, "Use: ABRT Id");

	cTimeMs starttime;
	if (id == siLive)
		DELETENULL(m_LiveStreamer);

	Dprintf("ABRT took %ld ms\n", starttime.Elapsed());
	DELETENULL(m_DataSockets[id]);
	return Respond(220, "Data connection closed");
}

bool cConnectionVTP::CmdQUIT(char *Opts) {
	if (!Respond(221, "Video Disk Recorder closing connection"))
		return false;
	DeferClose();
	return true;
}

bool cConnectionVTP::CmdSUSP(char *Opts) {
	if (StreamdevServerSetup.SuspendMode == smAlways || cSuspendCtl::IsActive())
		return Respond(220, "Server is suspended");
	else if (StreamdevServerSetup.SuspendMode == smOffer 
			&& StreamdevServerSetup.AllowSuspend) {
		cControl::Launch(new cSuspendCtl);
		return Respond(220, "Server is suspended");
	} else
		return Respond(550, "Client may not suspend server");
}

// Functions adopted from SVDRP
#define INIT_WRAPPER() bool _res = true
#define Reply(x...) _res &= ReplyWrapper(x)
#define EXIT_WRAPPER() return _res

bool cConnectionVTP::ReplyWrapper(int Code, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char *buffer;
	vasprintf(&buffer, fmt, ap);
	int npos;
	if (buffer[npos = strlen(buffer)-1] == '\n')
		buffer[npos] = '\0';
	bool res = Respond(Code, buffer);
	free(buffer);
	return res;
}

bool cConnectionVTP::CmdLSTE(char *Option) {
#if VDRVERSNUM < 10300
	cMutexLock MutexLock;
#else
  cSchedulesLock MutexLock;
#endif
	INIT_WRAPPER();
	/* we need to create a blocking copy of the socket here */
	int dupfd = dup(*this);
	fcntl(dupfd, F_SETFL, fcntl(dupfd, F_GETFL) & ~O_NONBLOCK);
#if VDRVERSNUM < 10300
  const cSchedules *Schedules = cSIProcessor::Schedules(MutexLock);
#else
  const cSchedules *Schedules = cSchedules::Schedules(MutexLock);
#endif
  if (Schedules) {
     FILE *f = fdopen(dupfd, "w");
     if (f) {
        Schedules->Dump(f, "215-");
        fflush(f);
        Reply(215, "End of EPG data");
				fclose(f);
        }
     else
        Reply(451, "Can't open file connection");
     }
  else
     Reply(451, "Can't get EPG data");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdLSTR(char *Option) {
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
}

bool cConnectionVTP::CmdLSTT(char *Option) {
	INIT_WRAPPER();
  if (*Option) {
     if (isnumber(Option)) {
        cTimer *timer = Timers.Get(strtol(Option, NULL, 10) - 1);
        if (timer)
           Reply(250, "%d %s", timer->Index() + 1, (const char*)timer->ToText(true));
        else
           Reply(501, "Timer \"%s\" not defined", Option);
        }
     else
        Reply(501, "Error in timer number \"%s\"", Option);
     }
  else if (Timers.Count()) {
     for (int i = 0; i < Timers.Count(); i++) {
         cTimer *timer = Timers.Get(i);
        if (timer)
           Reply(i < Timers.Count() - 1 ? -250 : 250, "%d %s", timer->Index() + 1, (const char*)timer->ToText(true));
        else
           Reply(501, "Timer \"%d\" not found", i + 1);
         }
     }
  else
     Reply(550, "No timers defined");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdMODT(char *Option) {
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
#if VDRVERSNUM < 10300
              t.SetActive(taActive);
#else
              t.SetFlags(tfActive);
#endif
           else if (strcasecmp(tail, "OFF") == 0)
#if VDRVERSNUM < 10300
              t.SetActive(taInactive);
#else
              t.ClrFlags(tfActive);
#endif
           else if (!t.Parse(tail)) {
              Reply(501, "Error in timer settings");
              EXIT_WRAPPER();
              }
           *timer = t;
           Timers.Save();
#if VDRVERSNUM < 10300
           isyslog("timer %d modified (%s)", timer->Index() + 1, 
							 timer->Active() ? "active" : "inactive");
#else
           isyslog("timer %d modified (%s)", timer->Index() + 1, 
							 timer->HasFlags(tfActive) ? "active" : "inactive");
#endif
           Reply(250, "%d %s", timer->Index() + 1, (const char*)timer->ToText(true));
           }
        else
           Reply(501, "Timer \"%d\" not defined", n);
        }
     else
        Reply(501, "Error in timer number");
     }
  else
     Reply(501, "Missing timer settings");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdNEWT(char *Option) {
	INIT_WRAPPER();
  if (*Option) {
     cTimer *timer = new cTimer;
     if (timer->Parse(Option)) {
        cTimer *t = Timers.GetTimer(timer);
        if (!t) {
           Timers.Add(timer);
           Timers.Save();
           isyslog("timer %d added", timer->Index() + 1);
           Reply(250, "%d %s", timer->Index() + 1, (const char*)timer->ToText(true));
           EXIT_WRAPPER();
           }
        else
           Reply(550, "Timer already defined: %d %s", t->Index() + 1, (const char*)t->ToText(true));
        }
     else
        Reply(501, "Error in timer settings");
     delete timer;
     }
  else
     Reply(501, "Missing timer settings");
	EXIT_WRAPPER();
}

bool cConnectionVTP::CmdDELT(char *Option) {
	INIT_WRAPPER();
  if (*Option) {
     if (isnumber(Option)) {
        cTimer *timer = Timers.Get(strtol(Option, NULL, 10) - 1);
        if (timer) {
           if (!timer->Recording()) {
              Timers.Del(timer);
              Timers.Save();
              isyslog("timer %s deleted", Option);
              Reply(250, "Timer \"%s\" deleted", Option);
              }
           else
              Reply(550, "Timer \"%s\" is recording", Option);
           }
        else
           Reply(501, "Timer \"%s\" not defined", Option);
        }
     else
        Reply(501, "Error in timer number \"%s\"", Option);
     }
  else
     Reply(501, "Missing timer number");
	EXIT_WRAPPER();
}

bool cConnectionVTP::Respond(int Code, const char *Message) {
	cTBString pkt;
	if (Code < 0)
		pkt.Format("%03d-%s", -Code, Message);
	else	
		pkt.Format("%03d %s", Code, Message);
	return cServerConnection::Respond((const char*)pkt);
}
	
