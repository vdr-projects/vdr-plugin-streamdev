#include "remux/ts2ps.h"
#include "remux/ts2pes.h"
#include "remux/ts2es.h"
#include "remux/extern.h"

#include <vdr/ringbuffer.h>
#include "server/recstreamer.h"
#include "server/connection.h"
#include "common.h"

using namespace Streamdev;

// --- cStreamdevRecStreamer -------------------------------------------------

cStreamdevRecStreamer::cStreamdevRecStreamer(RecPlayer *RecPlayer, const cServerConnection *Connection, int64_t StartOffset):
		cStreamdevStreamer("streamdev-recstreaming", Connection),
		m_RecPlayer(RecPlayer),
		m_StartOffset(StartOffset),
		m_From(0L)
{
	Dprintf("New rec streamer\n");
	m_To = (int64_t) m_RecPlayer->getLengthBytes() - StartOffset - 1;
}

cStreamdevRecStreamer::~cStreamdevRecStreamer() 
{
	Dprintf("Desctructing rec streamer\n");
	Stop();
}

int64_t cStreamdevRecStreamer::SetRange(int64_t &From, int64_t &To)
{
	int64_t l = (int64_t) GetLength();
	if (From < 0L) {
		From += l;
		if (From < 0L)
			From = 0L;
		To = l - 1;
	}
	else {
		if (To < 0L)
			To += l;
		else if (To >= l)
			To = l - 1;
		if (From > To) {
			// invalid range - return whole content
			From = 0L;
			To = l - 1;
		}
	}
	m_From = From;
	m_To = To;
	return m_To - m_From + 1;
}

uchar* cStreamdevRecStreamer::GetFromReceiver(int &Count)
{
	if (m_From <= m_To) {
		Count = (int) m_RecPlayer->getBlock(m_Buffer, m_StartOffset + m_From, sizeof(m_Buffer));
		return m_Buffer;
	}
	return NULL;
}

cString cStreamdevRecStreamer::ToText() const
{
	return "REPLAY";
}
