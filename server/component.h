/*
 *  $Id: component.h,v 1.1 2004/12/30 22:44:18 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVERS_COMPONENT_H
#define VDR_STREAMDEV_SERVERS_COMPONENT_H

#include "tools/socket.h"
#include "tools/select.h"

#include <vdr/tools.h>

class cServerConnection;

/* Basic TCP listen server, all functions virtual if a derivation wants to do 
   things different */

class cServerComponent: public cListObject {
private:
	cTBSocket m_Listen;
	const char *m_Protocol;
	const char *m_ListenIp;
	uint m_ListenPort;

public:
	cServerComponent(const char *Protocol, const char *ListenIp, uint ListenPort);
	virtual ~cServerComponent();

	/* Starts listening on the specified Port, override if you want to do things
	   different */
	virtual bool Init(void);

	/* Stops listening, override if you want to do things different */
	virtual void Exit(void);

	/* Adds the listening socket to the Select object */
	virtual void AddSelect(cTBSelect &Select) const { Select.Add(m_Listen); }
	
	/* Accepts the connection on a NewConnection() object and calls the 
	   Welcome() on it, override if you want to do things different */
	virtual cServerConnection *CanAct(const cTBSelect &Select);

	/* Returns a new connection object for CanAct */
	virtual cServerConnection *NewConnection(void) const = 0;
};

class cServerComponents: public cList<cServerComponent> {
};

#endif // VDR_STREAMDEV_SERVERS_COMPONENT_H
