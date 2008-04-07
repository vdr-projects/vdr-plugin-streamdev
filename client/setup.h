/*
 *  $Id: setup.h,v 1.4 2008/04/07 14:50:32 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_SETUPCLIENT_H
#define VDR_STREAMDEV_SETUPCLIENT_H

#include "common.h"

struct cStreamdevClientSetup {
	cStreamdevClientSetup(void);

	bool SetupParse(const char *Name, const char *Value);

	int  StartClient;
	char RemoteIp[20];
	int  RemotePort;
	int  StreamFilters;
	int  SyncEPG;
	int  HideMenuEntry;
};

extern cStreamdevClientSetup StreamdevClientSetup;

class cStreamdevClientMenuSetupPage: public cStreamdevMenuSetupPage {
private:
	cStreamdevClientSetup m_NewSetup;
	
protected:
	virtual void Store(void);

public:
	cStreamdevClientMenuSetupPage(void);
	virtual ~cStreamdevClientMenuSetupPage();
};

#endif // VDR_STREAMDEV_SETUPCLIENT_H
