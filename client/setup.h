/*
 *  $Id: setup.h,v 1.1 2004/12/30 22:44:03 lordjaxom Exp $
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
	int  StreamPIDS;
#if VDRVERSNUM >= 10300
	int  StreamFilters;
#endif
	int  SyncEPG;
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
