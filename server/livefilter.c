/*
 *  $Id: livefilter.c,v 1.2 2005/02/08 13:59:16 lordjaxom Exp $
 */

#include "server/livefilter.h"
#include "server/livestreamer.h"
#include "common.h"

#if VDRVERSNUM >= 10300

cStreamdevLiveFilter::cStreamdevLiveFilter(cStreamdevLiveStreamer *Streamer) {
	m_Streamer = Streamer;
}

cStreamdevLiveFilter::~cStreamdevLiveFilter() {
}

void cStreamdevLiveFilter::Process(u_short Pid, u_char Tid, const u_char *Data, int Length) 
{
	uchar buffer[TS_SIZE];
	int length = Length;
	int pos = 0;

	while (length > 0) {
		int chunk = min(length, TS_SIZE - 5);
		buffer[0] = TS_SYNC_BYTE;
		buffer[1] = (Pid >> 8) & 0xff;
		buffer[2] = Pid & 0xff;
		buffer[3] = Tid;
		buffer[4] = (uchar)chunk;
		memcpy(buffer + 5, Data + pos, chunk);
		length -= chunk;
		pos += chunk;

		int p = m_Streamer->Put(buffer, TS_SIZE);
		if (p != TS_SIZE)
			m_Streamer->ReportOverflow(TS_SIZE - p);
	}
}

#endif // VDRVERSNUM >= 10300
