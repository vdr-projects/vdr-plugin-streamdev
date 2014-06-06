/*
 *  $Id: streamdev-server.h,v 1.2 2010/07/19 13:49:32 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEVSERVER_H
#define VDR_STREAMDEVSERVER_H

#include "common.h"

#include <vdr/tools.h>
#include <vdr/plugin.h>

class cMainThreadHookSubscriber: public cListObject {
private:
	static cList<cMainThreadHookSubscriber> m_Subscribers;
	static cMutex m_Mutex;
public:
	static const cList<cMainThreadHookSubscriber>& Subscribers(cMutexLock& Lock);

	virtual void MainThreadHook() = 0;

	cMainThreadHookSubscriber();
	virtual ~cMainThreadHookSubscriber();
};

class cPluginStreamdevServer : public cPlugin {
private:
	static const char *DESCRIPTION;
	bool m_Suspend;

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
	virtual void MainThreadHook(void);
	virtual cMenuSetupPage *SetupMenu(void);
	virtual bool SetupParse(const char *Name, const char *Value);
};

#endif // VDR_STREAMDEVSERVER_H
