/*
 *  $Id: streamer.c,v 1.6 2005/02/10 22:24:26 lordjaxom Exp $
 */
 
#include <vdr/ringbuffer.h>
#include <vdr/device.h>
#include <sys/types.h>
#include <unistd.h>

#include "server/streamer.h"
#include "server/suspend.h"
#include "server/setup.h"
#include "tools/socket.h"
#include "common.h"

// --- cStreamdevWriter -------------------------------------------------------

cStreamdevWriter::cStreamdevWriter(cTBSocket *Socket, cStreamdevStreamer *Streamer):
		cThread("streamdev-writer"),
		m_Streamer(Streamer),
		m_Socket(Socket),
		m_Active(false)
{
}

cStreamdevWriter::~cStreamdevWriter()
{
	m_Active = false;
	Cancel(3);
}

void cStreamdevWriter::Action(void)
{
	Dprintf("Writer start\n");
	int max = 0;
	m_Active = true;
	while (m_Active) {
		int count;
		uchar *block = m_Streamer->Get(count);

		if (block) {
			if (!m_Socket->TimedWrite(block, count, 2000)) {
				esyslog("ERROR: streamdev-server: couldn't send data: %m");
				break;
			}
			if (count > max)
				max = count;
			m_Streamer->Del(count);
		}
	}
	m_Active = false;
	Dprintf("Max. Transmit Blocksize was: %d\n", max);
}

// --- cStreamdevStreamer -----------------------------------------------------

cStreamdevStreamer::cStreamdevStreamer(const char *Name):
		cThread(Name),
		m_Active(false),
		m_Writer(NULL),
		m_RingBuffer(new cRingBufferLinear(STREAMERBUFSIZE, TS_SIZE * 2, true, 
	                                       "streamdev-streamer")),
		m_SendBuffer(new cRingBufferLinear(WRITERBUFSIZE, TS_SIZE * 2))
{
	m_RingBuffer->SetTimeouts(0, 100);
	m_SendBuffer->SetTimeouts(0, 100);
}

cStreamdevStreamer::~cStreamdevStreamer() 
{
	Stop();
	delete m_RingBuffer;
	delete m_Writer;
	delete m_SendBuffer;
}

void cStreamdevStreamer::Start(cTBSocket *Socket) 
{
	Dprintf("start streamer\n");
	m_Writer = new cStreamdevWriter(Socket, this);
	Attach();
}

void cStreamdevStreamer::Activate(bool On) 
{
	Dprintf("activate streamer\n");
	if (On && !m_Active) {
		m_Writer->Start();
		cThread::Start();
	}
}

void cStreamdevStreamer::Stop(void) 
{
	if (m_Active) {
		Dprintf("stopping live streamer\n");
		m_Active = false;
		Cancel(3);
	}
}

void cStreamdevStreamer::Action(void) 
{
	int max = 0;

	m_Active = true;
	while (m_Active) {
		int got;
		uchar *block = m_RingBuffer->Get(got);

		if (block) {
			int count = Put(block, got);
			if (count)
				m_RingBuffer->Del(count);
		}
	}
}

