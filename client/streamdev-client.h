/*
 *  $Id: streamdev-client.h,v 1.3 2010/08/18 10:26:56 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEVCLIENT_H
#define VDR_STREAMDEVCLIENT_H

#include "common.h"

#include <vdr/plugin.h>

class cPluginStreamdevClient : public cPlugin {
private:
	static const char *DESCRIPTION;

public:
  cPluginStreamdevClient(void);
  virtual ~cPluginStreamdevClient();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void);
  virtual bool Start(void);
  virtual const char *MainMenuEntry(void);
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual void MainThreadHook(void);
};

#endif // VDR_STREAMDEVCLIENT_H
