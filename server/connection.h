/*
 *  $Id: connection.h,v 1.2 2005/02/08 17:22:35 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVER_CONNECTION_H
#define VDR_STREAMDEV_SERVER_CONNECTION_H

#include "tools/socket.h"
#include "tools/select.h"

#include "common.h"

class cChannel;
class cDevice;

/* Basic capabilities of a straight text-based protocol, most functions
   virtual to support more complicated protocols */

class cServerConnection: public cListObject, public cTBSocket {
private:
	char m_RdBuf[8192];
	uint m_RdBytes;
	
	char m_WrBuf[8192];
	uint m_WrBytes;
	uint m_WrOffs;

	const char *m_Protocol;

	bool m_DeferClose;

public:
	/* If you derive, specify a short string such as HTTP for Protocol, which
	   will be displayed in error messages */
	cServerConnection(const char *Protocol);
	virtual ~cServerConnection();

	/* Gets called if the client has been accepted by the core */
	virtual void Welcome(void) { }
	
	/* Gets called if the client has been rejected by the core */
	virtual void Reject(void) { DeferClose(); }

	/* Adds itself to the Select object, if data can be received or if data is 
	   to be sent. Override if necessary */
	virtual void AddSelect(cTBSelect &Select) const;

	/* Receives incoming data and calls ParseBuffer on it. Also writes queued
	   output data if possible. Override if necessary */
	virtual bool CanAct(const cTBSelect &Select);

	/* Called by CanAct(), parses the input buffer for full lines (terminated 
	   either by '\012' or '\015\012') and calls Command on them, if any */
	virtual bool ParseBuffer(void);

	/* Will be called when a command terminated by a newline has been received */
	virtual bool Command(char *Cmd) = 0;

	/* Will put Message into the response queue, which will be sent in the next
	   server cycle. Note that Message will be line-terminated by Respond */
	bool Respond(const std::string &Message);

	/* Will make the socket close after sending all queued output data */
	void DeferClose(void) { m_DeferClose = true; }

	/* Will retrieve an unused device for transmitting data. Use the returned
	   cDevice in a following call to StartTransfer */
	cDevice *GetDevice(const cChannel *Channel, int Priority);

	virtual void Flushed(void) {}

	virtual void Detach(void) = 0;
	virtual void Attach(void) = 0;
};

class cServerConnections: public cList<cServerConnection> {
};

inline void cServerConnection::AddSelect(cTBSelect &Select) const {
	if (m_WrBytes > 0) 
		Select.Add(*this, true);
		
	if (m_WrBytes == 0 && m_RdBytes < sizeof(m_RdBuf) - 1) 
		Select.Add(*this, false);
}

#endif // VDR_STREAMDEV_SERVER_CONNECTION_H
