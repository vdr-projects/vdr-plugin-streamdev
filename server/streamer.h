/*
 *  $Id: streamer.h,v 1.12 2010/07/19 13:49:32 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_STREAMER_H
#define VDR_STREAMDEV_STREAMER_H

#include <vdr/thread.h>
#include <vdr/ringbuffer.h>
#include <vdr/tools.h>

class cTBSocket;
class cStreamdevStreamer;
class cServerConnection;

#ifndef TS_SIZE
#define TS_SIZE 188
#endif

#define STREAMERBUFSIZE (20000 * TS_SIZE)
#define WRITERBUFSIZE (20000 * TS_SIZE)

// --- cStreamdevBuffer -------------------------------------------------------

class cStreamdevBuffer: public cRingBufferLinear {
public:
	// make public
	void WaitForPut(void) { cRingBuffer::WaitForPut(); }
	// Always write complete TS packets
	// (assumes Count is a multiple of TS_SIZE)
	int PutTS(const uchar *Data, int Count);
	cStreamdevBuffer(int Size, int Margin = 0, bool Statistics = false, const char *Description = NULL);
};

inline int cStreamdevBuffer::PutTS(const uchar *Data, int Count)
{
	int free = Free();
	if (free < Count)
		Count = free;

	Count -= Count % TS_SIZE;
	if (Count)
		Count = Put(Data, Count);
	else
		WaitForPut();
	return Count;
}

// --- cStreamdevWriter -------------------------------------------------------

class cStreamdevWriter: public cThread {
private:
	cStreamdevStreamer *m_Streamer;
	cTBSocket          *m_Socket;

protected:
	virtual void Action(void);

public:
	cStreamdevWriter(cTBSocket *Socket, cStreamdevStreamer *Streamer);
	virtual ~cStreamdevWriter();
};

// --- cStreamdevStreamer -----------------------------------------------------

class cStreamdevStreamer: public cThread {
private:
	const cServerConnection *m_Connection;
	cStreamdevWriter  *m_Writer;
	cStreamdevBuffer  *m_RingBuffer;
	cStreamdevBuffer  *m_SendBuffer;

protected:
	virtual void Action(void);

	bool IsRunning(void) const { return m_Writer; }

public:
	cStreamdevStreamer(const char *Name, const cServerConnection *Connection = NULL);
	virtual ~cStreamdevStreamer();

	const cServerConnection* Connection(void) const { return m_Connection; }

	virtual void Start(cTBSocket *Socket);
	virtual void Stop(void);
	bool Abort(void);

	void Activate(bool On);
	int Receive(uchar *Data, int Length) { return m_RingBuffer->PutTS(Data, Length); }
	void ReportOverflow(int Bytes) { m_RingBuffer->ReportOverflow(Bytes); }
	
	virtual int Put(const uchar *Data, int Count) { return m_SendBuffer->PutTS(Data, Count); }
	virtual uchar *Get(int &Count) { return m_SendBuffer->Get(Count); }
	virtual void Del(int Count) { m_SendBuffer->Del(Count); }

	virtual void Detach(void) {}
	virtual void Attach(void) {}
};

inline bool cStreamdevStreamer::Abort(void)
{
	return Active() && !m_Writer->Active();
}

#endif // VDR_STREAMDEV_STREAMER_H

