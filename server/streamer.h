/*
 *  $Id: streamer.h,v 1.2 2005/02/08 13:59:16 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_STREAMER_H
#define VDR_STREAMDEV_STREAMER_H

#include <vdr/thread.h>
#include <vdr/ringbuffer.h>
#include <vdr/tools.h>

class cTBSocket;

class cStreamdevStreamer: public cThread {
private:
	bool               m_Active;
	int                m_Receivers;
	uchar             *m_Buffer;
	const char        *m_Name;
	cTBSocket         *m_Socket;
	cRingBufferLinear *m_RingBuffer;

protected:
	virtual uchar *Process(const uchar *Data, int &Count, int &Result);
	virtual void Action(void);

	const cTBSocket *Socket(void) const { return m_Socket; }

public:
	cStreamdevStreamer(const char *Name);
	virtual ~cStreamdevStreamer();

	virtual void Start(cTBSocket *Socket);
	virtual void Stop(void);

	int Put(uchar *Data, int Length) { return m_RingBuffer->Put(Data, Length); }
	void ReportOverflow(int Bytes) { m_RingBuffer->ReportOverflow(Bytes); }

	virtual void Detach(void) = 0;
	virtual void Attach(void) = 0;
};

#endif // VDR_STREAMDEV_STREAMER_H

