/*
 *  componentVTP.h
 */
 
#ifndef VDR_STREAMDEV_SERVERS_SERVERVTP_H
#define VDR_STREAMDEV_SERVERS_SERVERVTP_H

#include "server/component.h"

class cComponentVTP: public cServerComponent {
protected:
	virtual cServerConnection *NewClient(void);

public:
	cComponentVTP(void);
};

#endif // VDR_STREAMDEV_SERVERS_SERVERVTP_H
