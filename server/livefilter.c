/*
 *  $Id: livefilter.c,v 1.1 2004/12/30 22:44:27 lordjaxom Exp $
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

void cStreamdevLiveFilter::Process(u_short Pid, u_char Tid, const u_char *Data,
		int Length) {
	static time_t firsterr = 0;
	static int errcnt = 0;
	static bool showerr = true;

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
		if (p != TS_SIZE) {
			++errcnt;
			if (showerr) {
				if (firsterr == 0)
					firsterr = time_ms();
				else if (firsterr + BUFOVERTIME > time_ms() && errcnt > BUFOVERCOUNT) {
					esyslog("ERROR: too many buffer overflows, logging stopped");
					showerr = false;
					firsterr = time_ms();
				}
			} else if (firsterr + BUFOVERTIME < time_ms()) {
				showerr = true;
				firsterr = 0;
				errcnt = 0;
			}

			if (showerr)
				esyslog("ERROR: ring buffer overflow (%d bytes dropped)", TS_SIZE - p);
			else
				firsterr = time_ms();
		}
	}
}

#endif // VDRVERSNUM >= 10300
