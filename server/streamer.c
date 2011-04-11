/*
 *  $Id: streamer.c,v 1.21 2010/07/30 10:01:11 schmirl Exp $
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

// --- cStreamdevBuffer -------------------------------------------------------

cStreamdevBuffer::cStreamdevBuffer(int Size, int Margin, bool Statistics, const char *Description):
		cRingBufferLinear(Size, Margin, Statistics, Description)
{
}

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
	int timeout = 0;

#if APIVERSNUM >= 10705
	SetPriority(-3);
#endif
	sel.Clear();
	sel.Add(*m_Socket, true);
	while (Running()) {
		if (block == NULL) {
			block = m_Streamer->Get(count);
			offset = 0;
		}

		if (block != NULL) {
			if (sel.Select(600) == -1) {
				if (errno == ETIMEDOUT && timeout++ < 20)
					continue;	// still Running()?
				esyslog("ERROR: streamdev-server: couldn't send data: %m");
				break;
			}
			timeout = 0;

			if (sel.CanWrite(*m_Socket)) {
				int written;
				int pkgsize = count;
				// SOCK_DGRAM indicates multicast
				if (m_Socket->Type() == SOCK_DGRAM) {
					// don't fragment multicast packets
					// max. payload on standard local ethernet is 1416 to 1456 bytes
					// and some STBs expect complete TS packets
					// so let's always limit to 7 * TS_SIZE = 1316
					if (pkgsize > 7 * TS_SIZE)
						pkgsize = 7 * TS_SIZE;
					else
						pkgsize -= pkgsize % TS_SIZE;
				}
				if ((written = m_Socket->Write(block + offset, pkgsize)) == -1) {
					esyslog("ERROR: streamdev-server: couldn't send %d bytes: %m", pkgsize);
					break;
				}

				// statistics
				if (count > max)
					max = count;

				offset += written;
				count -= written;

				// less than one TS packet left:
				// delete what we've written so far and get next chunk
				if (count < TS_SIZE) {
					m_Streamer->Del(offset);
					block = NULL;
				}
			}
		}
	}
	Dprintf("Max. Transmit Blocksize was: %d\n", max);
}

// --- cStreamdevStreamer -----------------------------------------------------

cStreamdevStreamer::cStreamdevStreamer(const char *Name, const cServerConnection *Connection):
		cThread(Name),
		m_Connection(Connection),
		m_Writer(NULL),
		m_RingBuffer(new cStreamdevBuffer(STREAMERBUFSIZE, TS_SIZE * 2,
		             true, "streamdev-streamer")),
		m_SendBuffer(new cStreamdevBuffer(WRITERBUFSIZE, TS_SIZE * 2))
{
	m_RingBuffer->SetTimeouts(0, 100);
	m_SendBuffer->SetTimeouts(100, 100);
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
	if (m_Writer) {
		Detach();
		DELETENULL(m_Writer);
	}
}

void cStreamdevStreamer::Action(void) 
{
#if APIVERSNUM >= 10705
	SetPriority(-3);
#endif
	while (Running()) {
		int got;
		uchar *block = m_RingBuffer->Get(got);

		if (block) {
			int count = Put(block, got);
			if (count)
				m_RingBuffer->Del(count);
		}
	}
}

