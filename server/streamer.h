/*
 *  $Id: streamer.h,v 1.1 2004/12/30 22:44:21 lordjaxom Exp $
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

	virtual void Detach(void) = 0;
	virtual void Attach(void) = 0;
};

#endif // VDR_STREAMDEV_STREAMER_H

