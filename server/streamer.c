/*
 *  $Id: streamer.c,v 1.3 2005/02/08 17:22:35 lordjaxom Exp $
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

#define VIDEOBUFSIZE MEGABYTE(4)
#define MAXBLOCKSIZE TS_SIZE*10

cStreamdevStreamer::cStreamdevStreamer(const char *Name):
		cThread(((std::string)"Streamdev: " + Name).c_str())
{
	m_Active     = false;
	m_Receivers  = 0;
	m_Buffer     = NULL;
	m_Name       = Name;
	m_Socket     = NULL;
	m_RingBuffer = new cRingBufferLinear(VIDEOBUFSIZE, TS_SIZE * 2, true);
}

cStreamdevStreamer::~cStreamdevStreamer() {
	Stop();
	if (m_Buffer != NULL) delete[] m_Buffer;
	delete m_RingBuffer;
}

void cStreamdevStreamer::Start(cTBSocket *Socket) {
	m_Socket = Socket;
	Attach();
	if (!m_Active)
		cThread::Start();
}

void cStreamdevStreamer::Stop(void) {
	if (m_Active) {
		Dprintf("stopping live streamer\n");
		m_Active = false;
		Cancel(3);
	}
}

uchar *cStreamdevStreamer::Process(const uchar *Data, int &Count, int &Result) {
	if (m_Buffer == NULL)
		m_Buffer = new uchar[MAXBLOCKSIZE];

	if (Count > MAXBLOCKSIZE)
		Count = MAXBLOCKSIZE;
	memcpy(m_Buffer, Data, Count);
	Result = Count;
	return m_Buffer;
}

void cStreamdevStreamer::Action(void) {
	int max = 0;

#if VDRVERSNUM < 10300
	isyslog("Streamdev: %s thread started (pid=%d)", m_Name, getpid());
#endif

	m_Active = true;
	while (m_Active) {
		int recvd;
		const uchar *block = m_RingBuffer->Get(recvd);

		if (block && recvd > 0) {
			int result = 0;
			uchar *sendBlock = Process(block, recvd, result);

			m_RingBuffer->Del(recvd);
			if (result > max) max = result;

			if (!m_Socket->TimedWrite(sendBlock, result, 150)) {
				if (errno != ETIMEDOUT) {
					esyslog("ERROR: Streamdev: Couldn't write data: %s", strerror(errno));
					m_Active = false;
				}
			}
		} else
			usleep(1); // this keeps the CPU load low (XXX: waiting buffers)
	}

	Dprintf("Max. Transmit Blocksize was: %d\n", max);

#if VDRVERSNUM < 10300
	isyslog("Streamdev: %s thread stopped", m_Name);
#endif
}

