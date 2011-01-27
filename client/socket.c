/*
 *  $Id: socket.c,v 1.15 2010/08/18 10:26:55 schmirl Exp $
 */
 
#include <tools/select.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define MINLOGREPEAT 10	//don't log connect failures too often (seconds)

#include "client/socket.h"
#include "client/setup.h"
#include "common.h"

cClientSocket ClientSocket;

cClientSocket::cClientSocket(void) 
{
	memset(m_DataSockets, 0, sizeof(cTBSocket*) * si_Count);
	m_Prio = false;
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

	uint64_t elapsed = starttime.Elapsed();
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
		static time_t lastTime = 0;
		if (time(NULL) - lastTime > MINLOGREPEAT) {
			esyslog("ERROR: Streamdev: Couldn't connect to %s:%d: %s", 
				(const char*)StreamdevClientSetup.RemoteIp,
				StreamdevClientSetup.RemotePort, strerror(errno));
			lastTime = time(NULL);
		}
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

	const char *Filters = "";
	if(Command("CAPS FILTERS", 220))
		Filters = ",FILTERS";

	const char *Prio = "";
	if(Command("CAPS PRIO", 220)) {
		Prio = ",PRIO";
		m_Prio = true;
	}

	isyslog("Streamdev: Connected to server %s:%d using capabilities TSPIDS%s%s",
	        RemoteIp().c_str(), RemotePort(), Filters, Prio);
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

bool cClientSocket::CloseDataConnection(eSocketId Id) {
	//if (!CheckConnection()) return false;

	CMD_LOCK;

	if(Id == siLive || Id == siLiveFilter)
		if (m_DataSockets[Id] != NULL) {
			std::string command = (std::string)"ABRT " + (const char*)itoa(Id);
			if (!Command(command, 220)) {
				if (errno == 0)
					esyslog("ERROR: Streamdev: Couldn't cleanly close data connection");
				//return false;
			}		
			DELETENULL(m_DataSockets[Id]);
		}
	return true;
}

bool cClientSocket::SetChannelDevice(const cChannel *Channel) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	std::string command = (std::string)"TUNE " 
				+ (const char*)Channel->GetChannelID().ToString();
	if (!Command(command, 220, 10000)) {
		if (errno == 0)
			esyslog("ERROR: Streamdev: Couldn't tune %s:%d to channel %s",
			        RemoteIp().c_str(), RemotePort(), Channel->Name());
		return false;
	}
	return true;
}

bool cClientSocket::SetPriority(int Priority) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	std::string command = (std::string)"PRIO " + (const char*)itoa(Priority);
	if (!Command(command, 220)) {
		if (errno == 0)
			esyslog("Streamdev: Failed to update priority on %s:%d", RemoteIp().c_str(), 
			        RemotePort());
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
			esyslog("Streamdev: Pid %d not available from %s:%d", Pid, RemoteIp().c_str(), 
			        RemotePort());
		return false;
	}
	return true;
}

bool cClientSocket::SetFilter(ushort Pid, uchar Tid, uchar Mask, bool On) {
	if (!CheckConnection()) return false;

	CMD_LOCK;

	std::string command = (std::string)(On ? "ADDF " : "DELF ") + (const char*)itoa(Pid)
	                    + " " + (const char*)itoa(Tid) + " " + (const char*)itoa(Mask);
	if (!Command(command, 220)) {
		if (errno == 0)
				esyslog("Streamdev: Filter %hu, %hhu, %hhu not available from %s:%d", 
						Pid, Tid, Mask, RemoteIp().c_str(), RemotePort());
		return false;
	}
	return true;
}

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
