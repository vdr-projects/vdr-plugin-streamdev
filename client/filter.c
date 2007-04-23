/*
 *  $Id: filter.c,v 1.5 2007/04/23 11:25:59 schmirl Exp $
 */

#include "client/filter.h"
#include "client/socket.h"
#include "tools/select.h"
#include "common.h"

#include <vdr/device.h>

#if VDRVERSNUM >= 10300

// --- cStreamdevFilter ------------------------------------------------------

class cStreamdevFilter: public cListObject {
private:
	uchar              m_Buffer[4096];
	int                m_Used;
	int                m_Pipe[2];
	u_short            m_Pid;
	u_char             m_Tid;
	u_char             m_Mask;

public:
	cStreamdevFilter(u_short Pid, u_char Tid, u_char Mask);
	virtual ~cStreamdevFilter();

	bool Matches(u_short Pid, u_char Tid);
	bool PutSection(const uchar *Data, int Length);
	int  ReadPipe(void) const { return m_Pipe[0]; }

	bool IsClosed(void);
	void Reset(void);

	u_short Pid(void) const { return m_Pid; }
	u_char Tid(void) const { return m_Tid; }
	u_char Mask(void) const { return m_Mask; }
};

inline bool cStreamdevFilter::Matches(u_short Pid, u_char Tid) {
	return m_Pid == Pid && m_Tid == (Tid & m_Mask);
}

cStreamdevFilter::cStreamdevFilter(u_short Pid, u_char Tid, u_char Mask) {
	m_Used = 0;
	m_Pid  = Pid;
	m_Tid  = Tid;
	m_Mask = Mask;
	m_Pipe[0] = m_Pipe[1] = -1;

#ifdef SOCK_SEQPACKET  
	// SOCK_SEQPACKET (since kernel 2.6.4)
	if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, m_Pipe) != 0) {
	        esyslog("streamev-client: socketpair(SOCK_SEQPACKET) failed: %m, trying SOCK_DGRAM");
	}
#endif
	if (m_Pipe[0] < 0 && socketpair(AF_UNIX, SOCK_DGRAM, 0, m_Pipe) != 0) {
	        esyslog("streamev-client: coudln't open section filter socket: %m");
	} 

	else if(fcntl(m_Pipe[0], F_SETFL, O_NONBLOCK) != 0 ||
		fcntl(m_Pipe[1], F_SETFL, O_NONBLOCK) != 0) {
	        esyslog("streamev-client: coudln't set section filter socket to non-blocking mode: %m");
	}
}

cStreamdevFilter::~cStreamdevFilter() {
	Dprintf("~cStreamdevFilter %p\n", this);
	if (m_Pipe[0] >= 0)
		close(m_Pipe[0]);
	if (m_Pipe[1] >= 0)
		close(m_Pipe[1]);
}

bool cStreamdevFilter::PutSection(const uchar *Data, int Length) {
	if (m_Used + Length >= (int)sizeof(m_Buffer)) {
		esyslog("ERROR: Streamdev: Section handler buffer overflow (%d bytes lost)",
				Length);
		m_Used = 0;
		return true;
	}
	memcpy(m_Buffer + m_Used, Data, Length);
	m_Used += Length;

	if (m_Used > 3) {
		int length = (((m_Buffer[1] & 0x0F) << 8) | m_Buffer[2]) + 3;
		if (m_Used == length) {
			if (write(m_Pipe[1], m_Buffer, length) < 0)
				return false;
			m_Used = 0;
		}
	}
	return true;
}

void cStreamdevFilter::Reset(void) {
	if(m_Used)
		dsyslog("cStreamdevFilter::Reset skipping %d bytes", m_Used);
	m_Used = 0;
}

bool cStreamdevFilter::IsClosed(void) {
	char m_Buffer[3] = {0,0,0}; /* tid 0, 0 bytes */

	// Test if pipe/socket has been closed by writing empty section
	if (write(m_Pipe[1], m_Buffer, 3) < 0 &&
	    errno != EAGAIN &&  
	    errno != EWOULDBLOCK) {

		if (errno != ECONNREFUSED &&
		    errno != ECONNRESET &&
		    errno != EPIPE)
			esyslog("cStreamdevFilter::TestPipe: failed: %m");

		return true;
	}

	return false;
}

// --- cStreamdevFilters -----------------------------------------------------

cStreamdevFilters::cStreamdevFilters(void):
		cThread("streamdev-client: sections assembler") {
	m_Active = false;
	m_RingBuffer = new cRingBufferLinear(MEGABYTE(1), TS_SIZE * 2, true);
	Start();
}

cStreamdevFilters::~cStreamdevFilters() {
	if (m_Active) {
		m_Active = false;
		Cancel(3);
	}
	delete m_RingBuffer;
}

int cStreamdevFilters::OpenFilter(u_short Pid, u_char Tid, u_char Mask) {
	cStreamdevFilter *f = new cStreamdevFilter(Pid, Tid, Mask);
	Add(f);
	return f->ReadPipe();
}

cStreamdevFilter *cStreamdevFilters::Matches(u_short Pid, u_char Tid) {
	for (cStreamdevFilter *f = First(); f; f = Next(f)) {
		if (f->Matches(Pid, Tid))
			return f;
	}
	return NULL;
}

void cStreamdevFilters::Put(const uchar *Data) {
	int p = m_RingBuffer->Put(Data, TS_SIZE);
	if (p != TS_SIZE)
		m_RingBuffer->ReportOverflow(TS_SIZE - p);
}

void cStreamdevFilters::Action(void) {
	m_Active = true;
	while (m_Active) {
		int recvd;
		const uchar *block = m_RingBuffer->Get(recvd);

		if (block && recvd > 0) {
			cStreamdevFilter *f;
			u_short pid = (((u_short)block[1] & PID_MASK_HI) << 8) | block[2];
			u_char tid = block[3];
		
			if ((f = Matches(pid, tid)) != NULL) {
				int len = block[4];
				if (!f->PutSection(block + 5, len)) {
					if (errno != EPIPE) {
						esyslog("streamdev-client: couldn't send section packet: %m");
						Dprintf("FATAL ERROR: %m\n");
					}
					ClientSocket.SetFilter(f->Pid(), f->Tid(), f->Mask(), false);
					Del(f);
				}
			}
			m_RingBuffer->Del(TS_SIZE);
		} else
			usleep(1);
	}
}

#endif // VDRVERSNUM >= 10300
