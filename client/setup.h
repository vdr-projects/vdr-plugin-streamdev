/*
 *  $Id: setup.h,v 1.2 2005/02/08 15:34:38 lordjaxom Exp $
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
