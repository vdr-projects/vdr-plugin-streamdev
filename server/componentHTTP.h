/*
 *  $Id: componentHTTP.h,v 1.1 2004/12/30 22:44:19 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_HTTPSERVER_H
#define VDR_STREAMDEV_HTTPSERVER_H

#include "server/component.h"
#include "server/connectionHTTP.h"

#include <tools/socket.h>
#include <tools/select.h>

class cComponentHTTP: public cServerComponent {
public:
	cComponentHTTP(void);
	~cComponentHTTP(void);
	
	virtual cServerConnection *NewConnection(void) const;
};

inline cServerConnection *cComponentHTTP::NewConnection(void) const {
	return new cConnectionHTTP;
}

#endif // VDR_STREAMDEV_HTTPSERVER_H
