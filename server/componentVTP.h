/*
 *  $Id: componentVTP.h,v 1.1 2004/12/30 22:44:19 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVERS_SERVERVTP_H
#define VDR_STREAMDEV_SERVERS_SERVERVTP_H

#include "server/component.h"
#include "server/connectionVTP.h"

#include <tools/socket.h>
#include <tools/select.h>

class cComponentVTP: public cServerComponent {
private:
	cTBSocket m_Listen;

public:
	cComponentVTP(void);
	virtual ~cComponentVTP();

	virtual cServerConnection *NewConnection(void) const;
};

inline cServerConnection *cComponentVTP::NewConnection(void) const {
	return new cConnectionVTP;
}

#endif // VDR_STREAMDEV_SERVERS_SERVERVTP_H
