/*
 *  $Id: streamer.c,v 1.17 2008/10/22 11:59:32 schmirl Exp $
 */
 
#include <vdr/ringbuffer.h>
#include <vdr/device.h>
#include <sys/types.h>
#include <unistd.h>

#include "server/streamer.h"
#include "server/suspend.h"
#include "server/setup.h"
#include "tools/socket.h"
#include "tools/select.h"
#include "common.h"

// --- cStreamdevWriter -------------------------------------------------------

cStreamdevWriter::cStreamdevWriter(cTBSocket *Socket, 
                                   cStreamdevStreamer *Streamer):
		cThread("streamdev-writer"),
		m_Streamer(Streamer),
		m_Socket(Socket)
{
}

cStreamdevWriter::~cStreamdevWriter()
{
	Dprintf("destructing writer\n");
	if (Running())
		Cancel(3);
}

void cStreamdevWriter::Action(void)
{
	cTBSelect sel;
	Dprintf("Writer start\n");
	int max = 0;
	uchar *block = NULL;
	int count, offset = 0;

	sel.Clear();
	sel.Add(*m_Socket, true);
	while (Running()) {
		if (block == NULL) {
			block = m_Streamer->Get(count);
			offset = 0;
		}

		if (block != NULL) {
			if (sel.Select(15000) == -1) {
				esyslog("ERROR: streamdev-server: couldn't send data: %m");
				break;
			}

			if (sel.CanWrite(*m_Socket)) {
				int written;
				if ((written = m_Socket->Write(block + offset, count)) == -1) {
					esyslog("ERROR: streamdev-server: couldn't send data: %m");
					break;
				}
				if (count > max)
					max = count;

				offset += written;
				count -= written;
				if (count == 0) {
					m_Streamer->Del(offset);
					block = NULL;
				}
			}
		}
	}
	Dprintf("Max. Transmit Blocksize was: %d\n", max);
}

// --- cStreamdevStreamer -----------------------------------------------------

cStreamdevStreamer::cStreamdevStreamer(const char *Name):
		cThread(Name),
		m_Running(false),
		m_Writer(NULL),
		m_RingBuffer(new cRingBufferLinear(STREAMERBUFSIZE, TS_SIZE * 2,
		             true, "streamdev-streamer")),
		m_SendBuffer(new cRingBufferLinear(WRITERBUFSIZE, TS_SIZE * 2))
{
	m_RingBuffer->SetTimeouts(0, 100);
	m_SendBuffer->SetTimeouts(0, 100);
}

cStreamdevStreamer::~cStreamdevStreamer() 
{
	Dprintf("Desctructing streamer\n");
	delete m_RingBuffer;
	delete m_SendBuffer;
}

void cStreamdevStreamer::Start(cTBSocket *Socket) 
{
	Dprintf("start streamer\n");
	m_Writer = new cStreamdevWriter(Socket, this);
	m_Running = true;
	Attach();
}

void cStreamdevStreamer::Activate(bool On) 
{
	if (On && !Active()) {
		Dprintf("activate streamer\n");
		m_Writer->Start();
		cThread::Start();
	}
}

void cStreamdevStreamer::Stop(void) 
{
	if (Running()) {
		Dprintf("stopping streamer\n");
		Cancel(3);
	}
	if (m_Running) {
		Detach();
		m_Running = false;
		DELETENULL(m_Writer);
	}
}

void cStreamdevStreamer::Action(void) 
{
	while (Running()) {
		int got;
		uchar *block = m_RingBuffer->Get(got);

		if (block) {
			int count = Put(block, got);
			if (count)
				m_RingBuffer->Del(count);
			else
				cCondWait::SleepMs(100);
		}
	}
}

