/*
 *  $Id: streamer.h,v 1.3 2005/02/08 19:54:52 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_STREAMER_H
#define VDR_STREAMDEV_STREAMER_H

#include <vdr/thread.h>
#include <vdr/ringbuffer.h>
#include <vdr/tools.h>

class cTBSocket;
class cStreamdevStreamer;

#define MAXTRANSMITBLOCKSIZE TS_SIZE*10
#define STREAMERBUFSIZE MEGABYTE(4)
#define WRITERBUFSIZE KILOBYTE(192)

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
};

class cStreamdevStreamer: public cThread {
private:
	bool               m_Active;
	cStreamdevWriter  *m_Writer;
	cRingBufferLinear *m_RingBuffer;
	cRingBufferLinear *m_SendBuffer;

protected:
	virtual void Action(void);

	//const cTBSocket *Socket(void) const { return m_Socket; }

public:
	cStreamdevStreamer(const char *Name);
	virtual ~cStreamdevStreamer();

	virtual void Start(cTBSocket *Socket);
	virtual void Stop(void);

	void Activate(bool On);
	int Receive(uchar *Data, int Length) { return m_RingBuffer->Put(Data, Length); }
	void ReportOverflow(int Bytes) { m_RingBuffer->ReportOverflow(Bytes); }
	
	virtual int Put(const uchar *Data, int Count);
	virtual uchar *Get(int &Count);
	virtual void Del(int Count);

	virtual void Detach(void) = 0;
	virtual void Attach(void) = 0;
};

#endif // VDR_STREAMDEV_STREAMER_H

