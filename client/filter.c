/*
 *  $Id: filter.c,v 1.2 2005/02/08 13:59:16 lordjaxom Exp $
 */

#include "client/filter.h"
#include "client/socket.h"
#include "tools/select.h"
#include "common.h"

#include <vdr/ringbuffer.h>

#if VDRVERSNUM >= 10300

cStreamdevFilter::cStreamdevFilter(u_short Pid, u_char Tid, u_char Mask) {
	m_Used = 0;
	m_Pid  = Pid;
	m_Tid  = Tid;
	m_Mask = Mask;

	if (pipe(m_Pipe) != 0 || fcntl(m_Pipe[0], F_SETFL, O_NONBLOCK) != 0) {
		esyslog("streamev-client: coudln't open section filter pipe: %m");
		m_Pipe[0] = m_Pipe[1] = -1;
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
