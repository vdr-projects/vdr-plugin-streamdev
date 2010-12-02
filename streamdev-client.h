/*
 *  $Id: streamdev-client.h,v 1.1.1.1 2004/12/30 22:43:59 lordjaxom Exp $
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
  virtual void Housekeeping(void);
  virtual const char *MainMenuEntry(void);
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
};

#endif // VDR_STREAMDEVCLIENT_H
