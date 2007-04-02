/*
 *  $Id: streamer.h,v 1.8 2007/04/02 10:32:34 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_STREAMER_H
#define VDR_STREAMDEV_STREAMER_H

#include <vdr/thread.h>
#include <vdr/ringbuffer.h>
#include <vdr/tools.h>

class cTBSocket;
class cStreamdevStreamer;

#define STREAMERBUFSIZE MEGABYTE(4)
#define WRITERBUFSIZE KILOBYTE(256)

// --- cStreamdevWriter -------------------------------------------------------

class cStreamdevWriter: public cThread {
private:
	cStreamdevStreamer *m_Streamer;
	cTBSocket          *m_Socket;
	bool                m_Active;

protected:
	virtual void Action(void);

public:
	cStreamdevWriter(cTBSocket *Socket, cStreamdevStreamer *Streamer);
	virtual ~cStreamdevWriter();

	bool IsActive(void) const { return m_Active; }
};

// --- cStreamdevStreamer -----------------------------------------------------

class cStreamdevStreamer: public cThread {
private:
	bool               m_Active;
	bool               m_Running;
	cStreamdevWriter  *m_Writer;
	cRingBufferLinear *m_RingBuffer;
	cRingBufferLinear *m_SendBuffer;

protected:
	virtual void Action(void);

	bool IsRunning(void) const { return m_Running; }

public:
	cStreamdevStreamer(const char *Name);
	virtual ~cStreamdevStreamer();

	virtual void Start(cTBSocket *Socket);
	virtual void Stop(void);
	bool Abort(void) const;

	void Activate(bool On);
	int Receive(uchar *Data, int Length) { return m_RingBuffer->Put(Data, Length); }
	void ReportOverflow(int Bytes) { m_RingBuffer->ReportOverflow(Bytes); }
	
	virtual int Put(const uchar *Data, int Count) { return m_SendBuffer->Put(Data, Count); }
	virtual uchar *Get(int &Count) { return m_SendBuffer->Get(Count); }
	virtual void Del(int Count) { m_SendBuffer->Del(Count); }

	virtual void Detach(void) {}
	virtual void Attach(void) {}
};

inline bool cStreamdevStreamer::Abort(void) const
{
	return m_Active && !m_Writer->IsActive();
}

#endif // VDR_STREAMDEV_STREAMER_H

