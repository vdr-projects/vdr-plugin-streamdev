#ifndef VDR_STREAMDEV_RECSTREAMER_H
#define VDR_STREAMDEV_RECSTREAMER_H

#include "server/streamer.h"
#include "server/recplayer.h"

#define RECBUFSIZE (174 * TS_SIZE)

// --- cStreamdevRecStreamer -------------------------------------------------

class cStreamdevRecStreamer: public cStreamdevStreamer {
private:
	//Streamdev::cTSRemux    *m_Remux;
	RecPlayer               m_RecPlayer;
	int64_t                 m_From;
	int64_t                 m_To;
	std::string		m_Pos;
	uchar                   m_Buffer[RECBUFSIZE];

protected:
	virtual uchar* GetFromReceiver(int &Count);
	virtual void DelFromReceiver(int Count) { m_From += Count; };

public:
	virtual bool IsReceiving(void) const { return m_From <= m_To; };
	inline uint64_t GetLength() { return m_RecPlayer.getLengthBytes(); }
	int64_t SetRange(int64_t &From, int64_t &To);
	virtual cString ToText() const;
	int64_t GetFromByPos();
	int32_t getIFrameBeforeFrame(int32_t frame);
	cStreamdevRecStreamer(cRecording *Recording, const cServerConnection *Connection, std::string pos);
	virtual ~cStreamdevRecStreamer();
};

#endif // VDR_STREAMDEV_RECSTREAMER_H
