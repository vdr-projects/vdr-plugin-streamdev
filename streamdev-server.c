/*
 * streamdev.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: streamdev-server.c,v 1.1 2004/12/30 22:43:59 lordjaxom Exp $
 */

#include "streamdev-server.h"
#include "server/setup.h"
#include "server/server.h"
#include "server/suspend.h"
#include "i18n.h"

const char *cPluginStreamdevServer::DESCRIPTION = "VDR Streaming Server";

cPluginStreamdevServer::cPluginStreamdevServer(void) {
}

cPluginStreamdevServer::~cPluginStreamdevServer() {
	cStreamdevServer::Exit();
}

const char *cPluginStreamdevServer::Description(void) {
	return tr(DESCRIPTION);
}

bool cPluginStreamdevServer::Start(void) {
	i18n_name = Name();
	RegisterI18n(Phrases);

	if (!StreamdevHosts.Load(STREAMDEVHOSTS, true, true)) {
		esyslog("streamdev-server: error while loading %s", STREAMDEVHOSTS);
		fprintf(stderr, "streamdev-server: error while loading %s\n");
		if (access(STREAMDEVHOSTS, F_OK) != 0)
			fprintf(stderr, "  Please install streamdevhosts.conf into the path "
			                "printed above. Without it\n" 
                      "  no client will be able to access your streaming-"
                      "server. An example can be\n"
                      "  found together with this plugin's sources.\n");
		return false;
	}

	cStreamdevServer::Init();

  return true;
}

bool cPluginStreamdevServer::Active(void) {
	return cStreamdevServer::Active();
}

const char *cPluginStreamdevServer::MainMenuEntry(void) {
	if (StreamdevServerSetup.SuspendMode == smOffer && !cSuspendCtl::IsActive())
		return tr("Suspend Live TV");
	return NULL;
}

cOsdObject *cPluginStreamdevServer::MainMenuAction(void) {
	cControl::Launch(new cSuspendCtl);
	return NULL;
}

cMenuSetupPage *cPluginStreamdevServer::SetupMenu(void) {
  return new cStreamdevServerMenuSetupPage;
}

bool cPluginStreamdevServer::SetupParse(const char *Name, const char *Value) {
  return StreamdevServerSetup.SetupParse(Name, Value);
}

VDRPLUGINCREATOR(cPluginStreamdevServer); // Don't touch this!
