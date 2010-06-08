/*
 * streamdev.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: streamdev-client.c,v 1.5.2.1 2010/06/08 05:56:14 schmirl Exp $
 */

#include "streamdev-client.h"
#include "client/device.h"
#include "client/setup.h"
#include "i18n.h"

#if VDRVERSNUM < 10400
#error "VDR-1.4.0 or greater is required"
#endif

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

const char *cPluginStreamdevClient::MainMenuEntry(void) {
	return StreamdevClientSetup.StartClient && !StreamdevClientSetup.HideMenuEntry ? tr("Suspend Server") : NULL;
}

cOsdObject *cPluginStreamdevClient::MainMenuAction(void) {
	if (ClientSocket.SuspendServer())
		Skins.Message(mtInfo, tr("Server is suspended"));
	else
		Skins.Message(mtError, tr("Couldn't suspend Server!"));
	return NULL;
}

cMenuSetupPage *cPluginStreamdevClient::SetupMenu(void) {
  return new cStreamdevClientMenuSetupPage;
}

bool cPluginStreamdevClient::SetupParse(const char *Name, const char *Value) {
  return StreamdevClientSetup.SetupParse(Name, Value);
}

VDRPLUGINCREATOR(cPluginStreamdevClient); // Don't touch this!
