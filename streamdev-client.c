/*
 * streamdev.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: streamdev-client.c,v 1.1 2004/12/30 22:43:59 lordjaxom Exp $
 */

#include "streamdev-client.h"
#include "client/device.h"
#include "client/setup.h"
#include "client/menu.h"
#include "i18n.h"

const char *cPluginStreamdevClient::DESCRIPTION = "VTP Streaming Client";

cPluginStreamdevClient::cPluginStreamdevClient(void) {
}

cPluginStreamdevClient::~cPluginStreamdevClient() {
}

const char *cPluginStreamdevClient::Description(void) {
	return tr(DESCRIPTION);
}

bool cPluginStreamdevClient::Start(void) {
	i18n_name = Name();
	RegisterI18n(Phrases);

	cStreamdevDevice::Init();

  return true;
}

void cPluginStreamdevClient::Housekeeping(void) {
	if (StreamdevClientSetup.StartClient && StreamdevClientSetup.SyncEPG)
		ClientSocket.SynchronizeEPG();
}

const char *cPluginStreamdevClient::MainMenuEntry(void) {
	return StreamdevClientSetup.StartClient ? tr("Streaming Control") : NULL;
}

cOsdObject *cPluginStreamdevClient::MainMenuAction(void) {
	return StreamdevClientSetup.StartClient ? new cStreamdevMenu : NULL;
}

cMenuSetupPage *cPluginStreamdevClient::SetupMenu(void) {
  return new cStreamdevClientMenuSetupPage;
}

bool cPluginStreamdevClient::SetupParse(const char *Name, const char *Value) {
  return StreamdevClientSetup.SetupParse(Name, Value);
}

VDRPLUGINCREATOR(cPluginStreamdevClient); // Don't touch this!
