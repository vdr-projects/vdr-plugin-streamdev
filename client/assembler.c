/*
 *  $Id: assembler.c,v 1.2 2005/01/25 14:14:43 lordjaxom Exp $
 */

#include "client/assembler.h"
#include "common.h"

#include "tools/socket.h"
#include "tools/select.h"

#include <vdr/tools.h>
#include <vdr/device.h>
#include <vdr/ringbuffer.h>

#include <unistd.h>

cStreamdevAssembler::cStreamdevAssembler(cTBSocket *Socket)
#if VDRVERSNUM >= 10300
		:cThread("Streamdev: UDP-TS Assembler")
#endif
{
	m_Socket = Socket;
	if (pipe(m_Pipe) != 0) {
		esyslog("streamdev-client: Couldn't open assembler pipe: %m");
		return;
	}
	fcntl(m_Pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(m_Pipe[1], F_SETFL, O_NONBLOCK);
	m_Mutex.Lock();
	Start();
}

cStreamdevAssembler::~cStreamdevAssembler() {
	if (m_Active) {
		m_Active = false;
/*      WakeUp();*/
		Cancel(3);
	}
	close(m_Pipe[0]);
	close(m_Pipe[1]);
}

void cStreamdevAssembler::Action(void) {
	cTBSelect sel;
	uchar buffer[2048];
	bool fillup = true;

	const int rbsize = TS_SIZE * 5600;
	const int rbmargin = TS_SIZE * 2;
	const int rbminfill = rbmargin * 50;
	cRingBufferLinear ringbuf(rbsize, rbmargin, true);

#if VDRVERSNUM < 10300
	isyslog("streamdev-client: UDP-TS Assembler thread started (pid=%d)", 
			getpid());
#endif

	m_Mutex.Lock();

	m_Active = true;
	while (m_Active) {
		sel.Clear();

		if (ringbuf.Available() < rbsize * 80 / 100)
			sel.Add(*m_Socket, false);
		if (ringbuf.Available() > rbminfill) {
			if (fillup) {
				Dprintf("giving signal\n");
				m_WaitFill.Broadcast();
				m_Mutex.Unlock();
				fillup = false;
			}
			sel.Add(m_Pipe[1], true);
		}

		if (sel.Select(1500) < 0) {
			if (!m_Active) // Exit was requested
				break;
			esyslog("streamdev-client: Fatal error: %m");
			Dprintf("streamdev-client: select failed (%m)\n");
			m_Active = false;
			break;
		}

		if (sel.CanRead(*m_Socket)) {
			int b;
			if ((b = m_Socket->Read(buffer, sizeof(buffer))) < 0) {
				esyslog("streamdev-client: Couldn't read from server: %m");
				Dprintf("streamdev-client: read failed (%m)\n");
				m_Active = false;
				break;
			}
			if (b == 0)
				m_Active = false;
			else
				ringbuf.Put(buffer, b);
		}

		if (sel.CanWrite(m_Pipe[1])) {
			int recvd;
			const uchar *block = ringbuf.Get(recvd);
			if (block && recvd > 0) {
				int result;
				if (recvd > ringbuf.Available() - rbminfill)
					recvd = ringbuf.Available() - rbminfill;
				if ((result = write(m_Pipe[1], block, recvd)) == -1) {
					esyslog("streamdev-client: Couldn't write to VDR: %m"); // TODO
					Dprintf("streamdev-client: write failed (%m)\n");
					m_Active = false;
					break;
				}
				ringbuf.Del(result);
			}
		}
	}

#if VDRVERSNUM < 10300
	isyslog("streamdev-client: UDP-TS Assembler thread stopped", getpid());
#endif
}

void cStreamdevAssembler::WaitForFill(void) {
	m_WaitFill.Wait(m_Mutex);
	m_Mutex.Unlock();
}
