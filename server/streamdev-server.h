/*
 *  $Id: streamdev-server.h,v 1.1.2.1 2010/06/14 10:40:20 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEVSERVER_H
#define VDR_STREAMDEVSERVER_H

#include "common.h"

#include <vdr/plugin.h>

class cPluginStreamdevServer : public cPlugin {
private:
	static const char *DESCRIPTION;

public:
	cPluginStreamdevServer(void);
	virtual ~cPluginStreamdevServer();

	virtual const char *Version(void) { return VERSION; }
	virtual const char *Description(void);
	virtual const char *CommandLineHelp(void);
	virtual bool ProcessArgs(int argc, char *argv[]);
	virtual bool Start(void);
	virtual void Stop(void);
	virtual cString Active(void);
	virtual const char *MainMenuEntry(void);
	virtual cOsdObject *MainMenuAction(void);
	virtual cMenuSetupPage *SetupMenu(void);
	virtual bool SetupParse(const char *Name, const char *Value);
};

#endif // VDR_STREAMDEVSERVER_H
