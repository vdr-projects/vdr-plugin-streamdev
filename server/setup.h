/*
 *  $Id: setup.h,v 1.1.1.1 2004/12/30 22:44:21 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_SETUPSERVER_H
#define VDR_STREAMDEV_SETUPSERVER_H

#include "common.h"

struct cStreamdevServerSetup {
	cStreamdevServerSetup(void);

	bool SetupParse(const char *Name, const char *Value);

	int MaxClients;
	int StartVTPServer;
	int VTPServerPort;
	char VTPBindIP[20];
	int StartHTTPServer;
	int HTTPServerPort;
	int HTTPStreamType;
	char HTTPBindIP[20];
	int SuspendMode;
	int AllowSuspend;
};

extern cStreamdevServerSetup StreamdevServerSetup;

class cStreamdevServerMenuSetupPage: public cStreamdevMenuSetupPage {
private:
	cStreamdevServerSetup m_NewSetup;

protected:
	virtual void Store(void);

public:
	cStreamdevServerMenuSetupPage(void);
	virtual ~cStreamdevServerMenuSetupPage();
};

#endif // VDR_STREAMDEV_SETUPSERVER_H
