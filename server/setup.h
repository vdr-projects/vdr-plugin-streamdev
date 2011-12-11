/*
 *  $Id: setup.h,v 1.4 2010/07/19 13:49:31 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_SETUPSERVER_H
#define VDR_STREAMDEV_SETUPSERVER_H

#include "common.h"

struct cStreamdevServerSetup {
	cStreamdevServerSetup(void);

	bool SetupParse(const char *Name, const char *Value);

	int HideMenuEntry;
	int MaxClients;
	int StartVTPServer;
	int VTPServerPort;
	char VTPBindIP[20];
	int LoopPrevention;
	int StartHTTPServer;
	int HTTPServerPort;
	int HTTPStreamType;
	char HTTPBindIP[20];
	int StartIGMPServer;
	int IGMPClientPort;
	int IGMPStreamType;
	char IGMPBindIP[20];
	int SuspendMode;
	int AllowSuspend;
};

extern cStreamdevServerSetup StreamdevServerSetup;

class cStreamdevServerMenuSetupPage: public cMenuSetupPage {
private:
	static const char* StreamTypes[];
	static const char* SuspendModes[];
	cStreamdevServerSetup m_NewSetup;

	void AddCategory(const char *Title);
	void Set();
protected:
	virtual void Store(void);
	virtual eOSState ProcessKey(eKeys Key);

public:
	cStreamdevServerMenuSetupPage(void);
	virtual ~cStreamdevServerMenuSetupPage();
};

#endif // VDR_STREAMDEV_SETUPSERVER_H
