/*
 * streamdev.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: streamdev-client.c,v 1.1.2.1 2010/06/14 10:40:11 schmirl Exp $
 */

#include "streamdev-client.h"
#include "client/device.h"
#include "client/setup.h"

#if !defined(APIVERSNUM) || APIVERSNUM < 10509
#error "VDR-1.5.9 API version or greater is required!"
#endif

const char *cPluginStreamdevClient::DESCRIPTION = trNOOP("VTP Streaming Client");

cPluginStreamdevClient::cPluginStreamdevClient(void) {
}

cPluginStreamdevClient::~cPluginStreamdevClient() {
}

const char *cPluginStreamdevClient::Description(void) {
	return tr(DESCRIPTION);
}

bool cPluginStreamdevClient::Start(void) {
	I18nRegister(PLUGIN_NAME_I18N);
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
