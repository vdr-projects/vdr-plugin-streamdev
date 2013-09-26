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

cStreamdevRecStreamer::cStreamdevRecStreamer(cRecording *Rec, const cServerConnection *Connection, std::string pos):
		cStreamdevStreamer("streamdev-recstreaming", Connection),
		m_RecPlayer(Rec),
		m_From(0L)
{
	Dprintf("New rec streamer\n");
	m_To = (int64_t) m_RecPlayer.getLengthBytes() - 1;
	m_Pos = pos;
}

cStreamdevRecStreamer::~cStreamdevRecStreamer() 
{
	Dprintf("Desctructing rec streamer\n");
	Stop();
}

int64_t cStreamdevRecStreamer::SetRange(int64_t &From, int64_t &To)
{
	int64_t l = (int64_t) m_RecPlayer.getLengthBytes();
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

int32_t cStreamdevRecStreamer::getIFrameBeforeFrame(int32_t frame)
{
	uint32_t iframe, len;
	uint64_t pos;
	m_RecPlayer.getNextIFrame(frame + 1, 0, &pos, &iframe, &len);
	Dprintf("pos: frame %i -> start at iFrame %i\n", frame, iframe);
	return iframe;
}

int64_t cStreamdevRecStreamer::GetFromByPos()
{
	if (m_Pos.empty()) return 0;
	
	std::string pos = m_Pos;
	
	// cut prefix (if any)
	if (pos.find('_') != std::string::npos) {
		pos = pos.substr(pos.find('_') + 1);
	}

	// resume file
	if (pos == "resume") {
		int frame = getIFrameBeforeFrame(m_RecPlayer.frameFromResume());
		Dprintf("pos: frame from resume: %i\n", frame);
		return m_RecPlayer.positionFromFrameNumber(frame);
	}
	
	// mark
	if (pos.find("mark.") == 0) {
		int index = atoi(pos.substr(5).c_str());
		int frame = getIFrameBeforeFrame(m_RecPlayer.frameFromMark(index));
		Dprintf("pos: mark %i - frame %i\n", index, frame);
		return m_RecPlayer.positionFromFrameNumber(frame);
	}

	// time
	if (pos.find("time.") == 0) {
		int seconds = atoi(pos.substr(5).c_str());
		int frame = getIFrameBeforeFrame(m_RecPlayer.frameFromSeconds(seconds));
		Dprintf("pos: %i seconds - frame %i\n", seconds, frame);
		return m_RecPlayer.positionFromFrameNumber(frame);
	}
	
	// frame number
	if (pos.find("frame.") == 0) {
		int frame = getIFrameBeforeFrame(atoi(pos.substr(6).c_str()));
		Dprintf("pos: frame %i\n", frame);
		return m_RecPlayer.positionFromFrameNumber(frame);
	}

	// default: byte index or percent
	// as "%" is the url escape character, interpret <100 as percent
	// if (pos.find("%") != std::string::npos) {
	// int percent = atoi(pos.substr(0, pos.find("%")).c_str());
	int64_t number = atol(pos.c_str());
	if (number < 100) {
		Dprintf("pos: %lld percent\n", (long long)number);
		int64_t offset = m_RecPlayer.getLengthBytes() * number / 100;
		return offset;
	}
	return number;
}

uchar* cStreamdevRecStreamer::GetFromReceiver(int &Count)
{
	if (m_From <= m_To) {
		Count = (int) m_RecPlayer.getBlock(m_Buffer, m_From, sizeof(m_Buffer));
		return m_Buffer;
	}
	return NULL;
}

cString cStreamdevRecStreamer::ToText() const
{
	return "REPLAY";
}
