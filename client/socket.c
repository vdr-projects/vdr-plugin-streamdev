/*
 *  $Id: socket.c,v 1.4 2005/02/08 17:22:35 lordjaxom Exp $
 */
 
#include <tools/select.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "client/socket.h"
#include "client/setup.h"
#include "client/remote.h"
#include "common.h"
#include "i18n.h"

cClientSocket ClientSocket;

cClientSocket::cClientSocket(void) 
{
	memset(m_DataSockets, 0, sizeof(cTBSocket*) * si_Count);
	Reset();
}

cClientSocket::~cClientSocket() 
{
	Reset();
	if (IsOpen()) Quit();
}

void cClientSocket::Reset(void) 
{
	for (int it = 0; it < si_Count; ++it) {
		if (m_DataSockets[it] != NULL)
			DELETENULL(m_DataSockets[it]);
	}
}

cTBSocket *cClientSocket::DataSocket(eSocketId Id) const {	
	return m_DataSockets[Id];
}

bool cClientSocket::Command(const std::string &Command, uint Expected, uint TimeoutMs) 
{
	errno = 0;

	std::string pkt = Command + "\015\012";
	Dprintf("OUT: |%s|\n", Command.c_str());

	cTimeMs starttime;
	if (!TimedWrite(pkt.c_str(), pkt.size(), TimeoutMs)) {
		esyslog("Streamdev: Lost connection to %s:%d: %s", RemoteIp().c_str(), RemotePort(), 
		        strerror(errno));
		Close();
		return false;
	}

	uint64 elapsed = starttime.Elapsed();
	if (Expected != 0) { // XXX+ What if elapsed > TimeoutMs?
		TimeoutMs -= elapsed;
		return Expect(Expected, NULL, TimeoutMs);
	}

	return true;
}

bool cClientSocket::Expect(uint Expected, std::string *Result, uint TimeoutMs) {
	char *endptr;
	int bufcount;
	bool res;

	errno = 0;

	if ((bufcount = ReadUntil(m_Buffer, sizeof(m_Buffer) - 1, "\012", TimeoutMs)) == -1) {
		esyslog("Streamdev: Lost connection to %s:%d: %s", RemoteIp().c_str(), RemotePort(), 
		        strerror(errno));
		Close();
		return false;
	}
	if (m_Buffer[bufcount - 1] == '\015')
		--bufcount;
	m_Buffer[bufcount] = '\0';
	Dprintf("IN: |%s|\n", m_Buffer);

	if (Result != NULL)
		*Result = m_Buffer;

	res = strtoul(m_Buffer, &endptr, 10) == Expected;
	return res;
}

bool cClientSocket::CheckConnection(void) {
	CMD_LOCK;

	if (IsOpen()) {
		cTBSelect select;

		Dprintf("connection open\n");

		// XXX+ check if connection is still alive (is there a better way?)
		// There REALLY shouldn't be anything readable according to PROTOCOL here
		// If there is, assume it's an eof signal (subseq. read would return 0)
		select.Add(*this, false);
		int res;
		if ((res = select.Select(0)) == 0) {
			Dprintf("select said nothing happened\n");
			return true;
		}
		Dprintf("closing connection (res was %d)", res);
		Close();
	}

	if (!Connect(StreamdevClientSetup.RemoteIp, StreamdevClientSetup.RemotePort)){
		esyslog("ERROR: Streamdev: Couldn't connect to %s:%d: %s", 
				(const char*)StreamdevClientSetup.RemoteIp,
				StreamdevClientSetup.RemotePort, strerror(errno));
		return false;
	}

	if (!Expect(220)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Didn't receive greeting from %s:%d", 
			        RemoteIp().c_str(), RemotePort());
		Close();
		return false;
	}

	if (!Command("CAPS TSPIDS", 220)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't negotiate capabilities on %s:%d", 
					RemoteIp().c_str(), RemotePort());
		Close();
		return false;
	}

	isyslog("Streamdev: Connected to server %s:%d using capabilities TSPIDS",
	        RemoteIp().c_str(), RemotePort());
	return true;
}

bool cClientSocket::ProvidesChannel(const cChannel *Channel, int Priority) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	std::string command = (std::string)"PROV " + (const char*)itoa(Priority) + " " 
	                    + (const char*)Channel->GetChannelID().ToString();
	if (!Command(command))
		return false;

	std::string buffer;
	if (!Expect(220, &buffer)) {
		if (buffer.substr(0, 3) != "560" && errno == 0)
			esyslog("ERROR: Streamdev: Couldn't check if %s:%d provides channel %s",
			        RemoteIp().c_str(), RemotePort(), Channel->Name());
		return false;
	}
	return true;
}

bool cClientSocket::CreateDataConnection(eSocketId Id) {
	cTBSocket listen(SOCK_STREAM);

	if (!CheckConnection()) return false;

	if (m_DataSockets[Id] != NULL)
		DELETENULL(m_DataSockets[Id]);

	if (!listen.Listen(LocalIp(), 0, 1)) {
		esyslog("ERROR: Streamdev: Couldn't create data connection: %s", 
				strerror(errno));
		return false;
	}

	std::string command = (std::string)"PORT " + (const char*)itoa(Id) + " " 
	                    + LocalIp().c_str() + "," 
	                    + (const char*)itoa((listen.LocalPort() >> 8) & 0xff) + ","
	                    + (const char*)itoa(listen.LocalPort() & 0xff);
	size_t idx = 4;
	while ((idx = command.find('.', idx + 1)) != (size_t)-1)
		command[idx] = ',';

	CMD_LOCK;

	if (!Command(command, 220)) {
		Dprintf("error: %m\n");
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't establish data connection to %s:%d",
					RemoteIp().c_str(), RemotePort());
		return false;
	}

	/* The server SHOULD do the following:
	 * - get PORT command
	 * - connect to socket
	 * - return 220
	 */

	m_DataSockets[Id] = new cTBSocket;
	if (!m_DataSockets[Id]->Accept(listen)) {
		esyslog("ERROR: Streamdev: Couldn't establish data connection to %s:%d%s%s",
				RemoteIp().c_str(), RemotePort(), errno == 0 ? "" : ": ",
				errno == 0 ? "" : strerror(errno));
		DELETENULL(m_DataSockets[Id]);
		return false;
	}

	return true;
}

bool cClientSocket::SetChannelDevice(const cChannel *Channel) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	std::string command = (std::string)"TUNE " 
	                    + (const char*)Channel->GetChannelID().ToString();
	if (!Command(command, 220)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't tune %s:%d to channel %s",
			        RemoteIp().c_str(), RemotePort(), Channel->Name());
		return false;
	}
	return true;
}

bool cClientSocket::SetPid(int Pid, bool On) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	std::string command = (std::string)(On ? "ADDP " : "DELP ") + (const char*)itoa(Pid);
	if (!Command(command, 220)) {
		if (errno == 0)
			esyslog("Streamdev: Pid %d not available from %s:%d", Pid, LocalIp().c_str(), 
			        LocalPort());
		return false;
	}
	return true;
}

#if VDRVERSNUM >= 10300
bool cClientSocket::SetFilter(ushort Pid, uchar Tid, uchar Mask, bool On) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	std::string command = (std::string)(On ? "ADDF " : "DELF ") + (const char*)itoa(Pid)
	                    + " " + (const char*)itoa(Tid) + " " + (const char*)itoa(Mask);
	if (!Command(command, 220)) {
		if (errno == 0)
				esyslog("Streamdev: Filter %hu, %hhu, %hhu not available from %s:%d", 
						Pid, Tid, Mask, LocalIp().c_str(), LocalPort());
		return false;
	}
	return true;
}
#endif

bool cClientSocket::CloseDvr(void) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	if (m_DataSockets[siLive] != NULL) {
		std::string command = (std::string)"ABRT " + (const char*)itoa(siLive);
		if (!Command(command, 220)) {
			if (errno == 0)
				esyslog("ERROR: Streamdev: Couldn't cleanly close data connection");
			return false;
		}
		
		DELETENULL(m_DataSockets[siLive]);
	}
	return true;
}

bool cClientSocket::SynchronizeEPG(void) {
	std::string buffer;
	bool result;
	FILE *epgfd;

	if (!CheckConnection()) return false;

	isyslog("Streamdev: Synchronizing EPG from server\n");

	CMD_LOCK;

	if (!Command("LSTE"))
		return false;

	if ((epgfd = tmpfile()) == NULL) {
		esyslog("ERROR: Streamdev: Error while processing EPG data: %s", 
				strerror(errno));
		return false;
	}

	while ((result = Expect(215, &buffer))) {
		if (buffer[3] == ' ') break;
		fputs(buffer.c_str() + 4, epgfd);
		fputc('\n', epgfd);
	}

	if (!result) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't fetch EPG data from %s:%d",
			        RemoteIp().c_str(), RemotePort());
		fclose(epgfd);
		return false;
	}

	rewind(epgfd);
	if (cSchedules::Read(epgfd))
#if VDRVERSNUM < 10300
		cSIProcessor::TriggerDump();
#else
		cSchedules::Cleanup(true);
#endif
	else {
		esyslog("ERROR: Streamdev: Parsing EPG data failed");
		fclose(epgfd);
		return false;
	}
	fclose(epgfd);
	return true;
}

bool cClientSocket::Quit(void) {
	bool res;

	if (!CheckConnection()) return false;

	if (!(res = Command("QUIT", 221))) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't quit command connection to %s:%d",
					RemoteIp().c_str(), RemotePort());
	}
	Close();
	return res;
}
	
bool cClientSocket::LoadRecordings(cRemoteRecordings &Recordings) {
	bool res;

	if (!CheckConnection()) return false;
	
	CMD_LOCK;

	if (!Command("LSTR"))
		return false;

	std::string buffer;
	while ((res = Expect(250, &buffer))) {
		cRemoteRecording *rec = new cRemoteRecording(buffer.c_str() + 4);
		Dprintf("recording valid: %d\n", rec->IsValid());
		if (rec->IsValid())
			Recordings.Add(rec);
		else
			delete rec;
		if (buffer[3] == ' ') break;
	}

	if (!res && buffer.substr(0, 3) != "550") {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't fetch recordings from %s:%d",
					RemoteIp().c_str(), RemotePort());
		return false;
	}

	for (cRemoteRecording *r = Recordings.First(); r; r = Recordings.Next(r)) {
		std::string command = (std::string)"LSTR " + (const char*)itoa(r->Index());
		if (!Command(command))
			return false;
			
		if (Expect(250, &buffer))
			r->ParseInfo(buffer.c_str() + 4);
		else if (buffer.substr(0, 3) != "550") {
			if (errno == 0)
				esyslog("ERROR: Streamdev: Couldn't fetch details for recording from %s:%d",
				        RemoteIp().c_str(), RemotePort());
			return false;
		}
		Dprintf("recording complete: %d\n", r->Index());
	}
	return res;
}

bool cClientSocket::StartReplay(const char *Filename) {
	if (!CheckConnection()) return false;
	
	CMD_LOCK;
	
	std::string command = (std::string)"PLAY " + Filename;
	if (!Command(command, 220)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't replay \"%s\" from %s:%d",
					Filename, RemoteIp().c_str(), RemotePort());
		return false;
	}
	return true;
}

bool cClientSocket::AbortReplay(void) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	if (m_DataSockets[siReplay] != NULL) {
		std::string command = (std::string)"ABRT " + (const char*)itoa(siReplay);
		if (!Command(command, 220)) {
			if (errno == 0)
				esyslog("ERROR: Streamdev: Couldn't cleanly close data connection");
			return false;
		}
		
		DELETENULL(m_DataSockets[siReplay]);
	}
	return true;
}

bool cClientSocket::DeleteRecording(cRemoteRecording *Recording) {
	bool res;
	cRemoteRecording *rec = NULL;

	if (!CheckConnection())
		return false;

	CMD_LOCK;

	if (!Command("LSTR"))
		return false;

	std::string buffer;
	while ((res = Expect(250, &buffer))) {
		if (rec == NULL) {
			rec = new cRemoteRecording(buffer.c_str() + 4);
			if (!rec->IsValid() || rec->Index() != Recording->Index())
				DELETENULL(rec);
		}
		if (buffer[3] == ' ') break;
	}

	if (!res && buffer.substr(0, 3) != "550") {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't fetch recordings from %s:%d",
					RemoteIp().c_str(), RemotePort());
		if (rec != NULL) delete rec;
		return false;
	}

	if (rec == NULL || *rec != *Recording) {
		ERROR(tr("Recordings not in sync! Try again..."));
		return false;
	}

	std::string command = (std::string)"DELR " + (const char*)itoa(Recording->Index());
	if (!Command(command, 250)) {
		ERROR(tr("Couldn't delete recording! Try again..."));
		return false;
	}
	return true;
}

bool cClientSocket::SuspendServer(void) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	if (!Command("SUSP", 220)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't suspend server");
		return false;
	}
	return true;
}

bool cClientSocket::LoadTimers(cRemoteTimers &Timers) {
	if (!CheckConnection()) return false;
	
	CMD_LOCK;

	if (!Command("LSTT"))
		return false;

	bool res;
	std::string buffer;
	while ((res = Expect(250, &buffer))) {
		cRemoteTimer *timer = new cRemoteTimer(buffer.c_str() + 4);
		Dprintf("timer valid: %d\n", timer->IsValid());
		if (timer->IsValid())
			Timers.Add(timer);
		if (buffer[3] == ' ') break;
	}

	if (!res && buffer.substr(0, 3) != "550") {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't fetch recordings from %s:%d",
					RemoteIp().c_str(), RemotePort());
		return false;
	}
	return res;
}

bool cClientSocket::SaveTimer(cRemoteTimer *Old, cRemoteTimer &New) {
	if (!CheckConnection()) return false;
	
	CMD_LOCK;

	if (New.Index() == -1) { // New timer
		std::string command = (std::string)"NEWT " + (const char*)New.ToText();
		if (!Command(command, 250)) {
			ERROR(tr("Couldn't save timer! Try again..."));
			return false;
		}
	} else { // Modified timer
		std::string command = (std::string)"LSTT " + (const char*)itoa(New.Index());
		if (!Command(command))
			return false;
		
		std::string buffer;
		if (!Expect(250, &buffer)) {
			if (errno == 0)
				ERROR(tr("Timers not in sync! Try again..."));
			else
				ERROR(tr("Server error! Try again..."));
			return false;
		}

		cRemoteTimer oldstate(buffer.c_str() + 4);
		if (oldstate != *Old) {
			/*Dprintf("old timer: %d,%d,%d,%d,%d,%d,%s,%d,%s,%d\n", oldstate.m_Index,
					oldstate.m_Active,oldstate.m_Day,oldstate.m_Start,oldstate.m_StartTime,oldstate.m_Priority,oldstate.m_File,oldstate.m_FirstDay,(const char*)oldstate.m_Summary,oldstate.m_Channel->Number());
			Dprintf("new timer: %d,%d,%d,%d,%d,%d,%s,%d,%s,%d\n", Old->m_Index,
					Old->m_Active,Old->m_Day,Old->m_Start,Old->m_StartTime,Old->m_Priority,Old->m_File,Old->m_FirstDay,(const char*)Old->m_Summary,Old->m_Channel->Number());*/
			ERROR(tr("Timers not in sync! Try again..."));
			return false;
		}


		command = (std::string)"MODT " + (const char*)itoa(New.Index()) + " " 
		        + (const char*)New.ToText();
		if (!Command(command, 250)) {
			ERROR(tr("Couldn't save timer! Try again..."));
			return false;
		}
	}
	return true;
}

bool cClientSocket::DeleteTimer(cRemoteTimer *Timer) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	std::string command = (std::string)"LSTT " + (const char*)itoa(Timer->Index());
	if (!Command(command))
		return false;
	
	std::string buffer;
	if (!Expect(250, &buffer)) {
		if (errno == 0)
			ERROR(tr("Timers not in sync! Try again..."));
		else
			ERROR(tr("Server error! Try again..."));
		return false;
	}

	cRemoteTimer oldstate(buffer.c_str() + 4);
	if (oldstate != *Timer) {
		ERROR(tr("Timers not in sync! Try again..."));
		return false;
	}

	command = (std::string)"DELT " + (const char*)itoa(Timer->Index());
	if (!Command(command, 250)) {
		ERROR(tr("Couldn't delete timer! Try again..."));
		return false;
	}
	return true;
}
