/*
 *  $Id: socket.c,v 1.3 2005/02/08 15:34:38 lordjaxom Exp $
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

bool cClientSocket::Command(const cTBString &Command, uint Expected, 
		uint TimeoutMs) {
	errno = 0;

	cTBString pkt = Command + "\015\012";
	Dprintf("OUT: |%s|\n", (const char*)Command);

	cTimeMs starttime;
	if (!TimedWrite((const char*)pkt, pkt.Length(), TimeoutMs)) {
		esyslog("Streamdev: Lost connection to %s:%d: %s", 
				(const char*)RemoteIp(), RemotePort(), strerror(errno));
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

bool cClientSocket::Expect(uint Expected, cTBString *Result, uint TimeoutMs) {
	char *buffer;
	char *endptr;
	int bufcount;
	bool res;

	errno = 0;

	buffer = new char[BUFSIZ + 1];

	if ((bufcount = ReadUntil(buffer, BUFSIZ, "\012", TimeoutMs))
			== -1) {
		esyslog("Streamdev: Lost connection to %s:%d: %s", 
				(const char*)RemoteIp(), RemotePort(), strerror(errno));
		Close();
		delete[] buffer;
		return false;
	}
	if (buffer[bufcount - 1] == '\015')
		--bufcount;
	buffer[bufcount] = '\0';
	Dprintf("IN: |%s|\n", buffer);

	if (Result != NULL)
		*Result = buffer;

	res = strtoul(buffer, &endptr, 10) == Expected;
	delete[] buffer;
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
					(const char*)RemoteIp(), RemotePort());
		Close();
		return false;
	}

	if (!Command((cTBString)"CAPS TSPIDS", 220)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't negotiate capabilities on %s:%d", 
					(const char*)RemoteIp(), RemotePort());
		Close();
		return false;
	}

	isyslog("Streamdev: Connected to server %s:%d using capabilities TSPIDS",
	        (const char*)RemoteIp(), RemotePort());
	return true;
}

bool cClientSocket::ProvidesChannel(const cChannel *Channel, int Priority) {
	cTBString buffer;

	if (!CheckConnection()) return false;

	CMD_LOCK;

	if (!Command("PROV " + cTBString::Number(Priority) + " " 
			+ Channel->GetChannelID().ToString()))
		return false;

	if (!Expect(220, &buffer)) {
		if (buffer.Left(3) != "560" && errno == 0)
			esyslog("ERROR: Streamdev: Couldn't check if %s:%d provides channel %s",
					(const char*)RemoteIp(), RemotePort(), Channel->Name());
		return false;
	}
	return true;
}

bool cClientSocket::CreateDataConnection(eSocketId Id) {
	int idx;
	cTBSocket listen(SOCK_STREAM);
	cTBString buffer;

	if (!CheckConnection()) return false;

	if (m_DataSockets[Id] != NULL)
		DELETENULL(m_DataSockets[Id]);

	if (!listen.Listen((const char*)LocalIp(), 0, 1)) {
		esyslog("ERROR: Streamdev: Couldn't create data connection: %s", 
				strerror(errno));
		return false;
	}

	buffer.Format("PORT %d %s,%d,%d", Id, (const char*)LocalIp(), 
			(listen.LocalPort() >> 8) & 0xff, listen.LocalPort() & 0xff);
	idx = 5;
	while ((idx = buffer.Find('.', idx + 1)) != -1)
		buffer[idx] = ',';

	CMD_LOCK;

	if (!Command(buffer, 220)) {
		Dprintf("error: %m\n");
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't establish data connection to %s:%d",
					(const char*)RemoteIp(), RemotePort());
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
				(const char*)RemoteIp(), RemotePort(), errno == 0 ? "" : ": ",
				errno == 0 ? "" : strerror(errno));
		DELETENULL(m_DataSockets[Id]);
		return false;
	}

	return true;
}

bool cClientSocket::SetChannelDevice(const cChannel *Channel) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	if (!Command((cTBString)"TUNE " + Channel->GetChannelID().ToString(), 220)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't tune %s:%d to channel %s",
					(const char*)RemoteIp(), RemotePort(), Channel->Name());
		return false;
	}
	return true;
}

bool cClientSocket::SetPid(int Pid, bool On) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	if (!Command((On ? "ADDP " : "DELP ") + cTBString::Number(Pid), 220)) {
		if (errno == 0)
			esyslog("Streamdev: Pid %d not available from %s:%d", Pid,
					(const char*)LocalIp(), LocalPort());
		return false;
	}
	return true;
}

#if VDRVERSNUM >= 10300
bool cClientSocket::SetFilter(ushort Pid, uchar Tid, uchar Mask, bool On) {
	cTBString cmd;
	if (!CheckConnection()) return false;

	CMD_LOCK;
	cmd.Format("%s %hu %hhu %hhu", On ? "ADDF" : "DELF", Pid, Tid, Mask);
	if (!Command(cmd, 220)) {
		if (errno == 0)
				esyslog("Streamdev: Filter %hu, %hhu, %hhu not available from %s:%d", 
						Pid, Tid, Mask, (const char*)LocalIp(), LocalPort());
		return false;
	}
	return true;
}
#endif

bool cClientSocket::CloseDvr(void) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	if (m_DataSockets[siLive] != NULL) {
		if (!Command("ABRT " + cTBString::Number(siLive), 220)) {
			if (errno == 0)
				esyslog("ERROR: Streamdev: Couldn't cleanly close data connection");
			return false;
		}
		
		DELETENULL(m_DataSockets[siLive]);
	}
	return true;
}

bool cClientSocket::SynchronizeEPG(void) {
	cTBString buffer;
	bool res;
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

	while ((res = Expect(215, &buffer))) {
		if (buffer[3] == ' ') break;
		fputs((const char*)buffer + 4, epgfd);
		fputc('\n', epgfd);
	}

	if (!res) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't fetch EPG data from %s:%d",
					(const char*)RemoteIp(), RemotePort());
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
					(const char*)RemoteIp(), RemotePort());
	}
	Close();
	return res;
}
	
bool cClientSocket::LoadRecordings(cRemoteRecordings &Recordings) {
	cTBString buffer;
	bool res;

	if (!CheckConnection()) return false;
	
	CMD_LOCK;

	if (!Command("LSTR"))
		return false;

	while ((res = Expect(250, &buffer))) {
		cRemoteRecording *rec = new cRemoteRecording((const char*)buffer + 4);
		Dprintf("recording valid: %d\n", rec->IsValid());
		if (rec->IsValid())
			Recordings.Add(rec);
		else
			delete rec;
		if (buffer[3] == ' ') break;
	}

	if (!res && buffer.Left(3) != "550") {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't fetch recordings from %s:%d",
					(const char*)RemoteIp(), RemotePort());
		return false;
	}

	for (cRemoteRecording *r = Recordings.First(); r; r = Recordings.Next(r)) {
		if (!Command("LSTR " + cTBString::Number(r->Index())))
			return false;
			
		if (Expect(250, &buffer))
			r->ParseInfo((const char*)buffer + 4);
		else if (buffer.Left(3) != "550") {
			if (errno == 0)
				esyslog("ERROR: Streamdev: Couldn't fetch details for recording from "
						"%s:%d", (const char*)RemoteIp(), RemotePort());
			return false;
		}
		Dprintf("recording complete: %d\n", r->Index());
	}
	return res;
}

bool cClientSocket::StartReplay(const char *Filename) {
	if (!CheckConnection()) return false;
	
	CMD_LOCK;
		
	if (!Command((cTBString)"PLAY " + Filename, 220)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't replay \"%s\" from %s:%d",
					Filename, (const char*)RemoteIp(), RemotePort());
		return false;
	}
	return true;
}

bool cClientSocket::AbortReplay(void) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	if (m_DataSockets[siReplay] != NULL) {
		if (!Command("ABRT " + cTBString::Number(siReplay), 220)) {
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
	cTBString buffer;
	cRemoteRecording *rec = NULL;

	if (!CheckConnection())
		return false;

	CMD_LOCK;

	if (!Command("LSTR"))
		return false;

	while ((res = Expect(250, &buffer))) {
		if (rec == NULL) {
			rec = new cRemoteRecording((const char*)buffer + 4);
			if (!rec->IsValid() || rec->Index() != Recording->Index())
				DELETENULL(rec);
		}
		if (buffer[3] == ' ') break;
	}

	if (!res && buffer.Left(3) != "550") {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't fetch recordings from %s:%d",
					(const char*)RemoteIp(), RemotePort());
		if (rec != NULL) delete rec;
		return false;
	}

	if (rec == NULL || *rec != *Recording) {
		ERROR(tr("Recordings not in sync! Try again..."));
		return false;
	}

	if (!Command("DELR " + cTBString::Number(Recording->Index()), 250)) {
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
	cTBString buffer;
	bool res;

	if (!CheckConnection()) return false;
	
	CMD_LOCK;

	if (!Command("LSTT"))
		return false;

	while ((res = Expect(250, &buffer))) {
		cRemoteTimer *timer = new cRemoteTimer((const char*)buffer + 4);
		Dprintf("timer valid: %d\n", timer->IsValid());
		if (timer->IsValid())
			Timers.Add(timer);
		if (buffer[3] == ' ') break;
	}

	if (!res && buffer.Left(3) != "550") {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't fetch recordings from %s:%d",
					(const char*)RemoteIp(), RemotePort());
		return false;
	}
	return res;
}

bool cClientSocket::SaveTimer(cRemoteTimer *Old, cRemoteTimer &New) {
	cTBString buffer;

	if (!CheckConnection()) return false;
	
	CMD_LOCK;

	if (New.Index() == -1) { // New timer
		if (!Command((cTBString)"NEWT " + New.ToText(), 250)) {
			ERROR(tr("Couldn't save timer! Try again..."));
			return false;
		}
	} else { // Modified timer
		if (!Command("LSTT " + cTBString::Number(New.Index())))
			return false;
			
		if (!Expect(250, &buffer)) {
			if (errno == 0)
				ERROR(tr("Timers not in sync! Try again..."));
			else
				ERROR(tr("Server error! Try again..."));
			return false;
		}

		cRemoteTimer oldstate((const char*)buffer + 4);
		if (oldstate != *Old) {
			/*Dprintf("old timer: %d,%d,%d,%d,%d,%d,%s,%d,%s,%d\n", oldstate.m_Index,
					oldstate.m_Active,oldstate.m_Day,oldstate.m_Start,oldstate.m_StartTime,oldstate.m_Priority,oldstate.m_File,oldstate.m_FirstDay,(const char*)oldstate.m_Summary,oldstate.m_Channel->Number());
			Dprintf("new timer: %d,%d,%d,%d,%d,%d,%s,%d,%s,%d\n", Old->m_Index,
					Old->m_Active,Old->m_Day,Old->m_Start,Old->m_StartTime,Old->m_Priority,Old->m_File,Old->m_FirstDay,(const char*)Old->m_Summary,Old->m_Channel->Number());*/
			ERROR(tr("Timers not in sync! Try again..."));
			return false;
		}

		if (!Command("MODT " + cTBString::Number(New.Index()) + " " 
				+ New.ToText(), 250)) {
			ERROR(tr("Couldn't save timer! Try again..."));
			return false;
		}
	}
	return true;
}

bool cClientSocket::DeleteTimer(cRemoteTimer *Timer) {
	cTBString buffer;

	if (!CheckConnection()) 
		return false;

	CMD_LOCK;

	if (!Command("LSTT " + cTBString::Number(Timer->Index())))
		return false;
		
	if (!Expect(250, &buffer)) {
		if (errno == 0)
			ERROR(tr("Timers not in sync! Try again..."));
		else
			ERROR(tr("Server error! Try again..."));
		return false;
	}

	cRemoteTimer oldstate((const char*)buffer + 4);
	
	if (oldstate != *Timer) {
		ERROR(tr("Timers not in sync! Try again..."));
		return false;
	}

	if (!Command("DELT " + cTBString::Number(Timer->Index()), 250)) {
		ERROR(tr("Couldn't delete timer! Try again..."));
		return false;
	}
	return true;
}
