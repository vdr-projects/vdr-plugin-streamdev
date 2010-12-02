/*
 *  $Id: livefilter.c,v 1.7 2009/02/13 13:02:40 schmirl Exp $
 */

#include "server/livefilter.h"
#include "server/streamer.h"
#include "common.h"

#ifndef TS_SYNC_BYTE
#    define TS_SYNC_BYTE     0x47
#endif

cStreamdevLiveFilter::cStreamdevLiveFilter(cStreamdevStreamer *Streamer) {
	m_Streamer = Streamer;
}

void cStreamdevLiveFilter::Process(u_short Pid, u_char Tid, const u_char *Data, int Length) 
{
	uchar buffer[TS_SIZE];
	int length = Length;
	int pos = 0;

	while (length > 0) {
		int chunk = min(length, TS_SIZE - 5);
		buffer[0] = TS_SYNC_BYTE;
		buffer[1] = ((Pid >> 8) & 0x3f) | (pos==0 ? 0x40 : 0); /* bit 6: payload unit start indicator (PUSI) */
		buffer[2] = Pid & 0xff;
		buffer[3] = Tid;
		// this makes it a proprietary stream
		buffer[4] = (uchar)chunk;
		memcpy(buffer + 5, Data + pos, chunk);
		length -= chunk;
		pos += chunk;

		int p = m_Streamer->Put(buffer, TS_SIZE);
		if (p != TS_SIZE)
			m_Streamer->ReportOverflow(TS_SIZE - p);
	}
}
