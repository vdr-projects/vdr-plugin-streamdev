/*
 * streamdev.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: streamdev-server.c,v 1.10 2008/10/13 11:30:05 schmirl Exp $
 */

#include <getopt.h>
#include "remux/extern.h"
#include "streamdev-server.h"
#include "server/setup.h"
#include "server/server.h"
#include "server/suspend.h"

#if !defined(APIVERSNUM) || APIVERSNUM < 10509
#error "VDR-1.5.9 API version or greater is required!"
#endif

const char *cPluginStreamdevServer::DESCRIPTION = trNOOP("VDR Streaming Server");

cPluginStreamdevServer::cPluginStreamdevServer(void) 
{
}

cPluginStreamdevServer::~cPluginStreamdevServer() 
{
	free(opt_remux);
}

const char *cPluginStreamdevServer::Description(void) 
{
	return tr(DESCRIPTION);
}

const char *cPluginStreamdevServer::CommandLineHelp(void)
{
	// return a string that describes all known command line options.
	return "  -r <CMD>, --remux=<CMD>  Define an external command for remuxing.\n";
}

bool cPluginStreamdevServer::ProcessArgs(int argc, char *argv[])
{
	// implement command line argument processing here if applicable.
	static const struct option long_options[] = {
		{ "remux", required_argument, NULL, 'r' },
		{ NULL, 0, NULL, 0 }
	};

	int c;
	while((c = getopt_long(argc, argv, "r:", long_options, NULL)) != -1) {
		switch (c) {
			case 'r':
				if (opt_remux)
				    free(opt_remux);
				opt_remux = strdup(optarg);
				break;
			default:
				return false;
		}
	}
	return true;
}

bool cPluginStreamdevServer::Start(void) 
{
	I18nRegister(PLUGIN_NAME_I18N);
	if (!StreamdevHosts.Load(STREAMDEVHOSTSPATH, true, true)) {
		esyslog("streamdev-server: error while loading %s", STREAMDEVHOSTSPATH);
		fprintf(stderr, "streamdev-server: error while loading %s\n", STREAMDEVHOSTSPATH);
		if (access(STREAMDEVHOSTSPATH, F_OK) != 0) {
			fprintf(stderr, "  Please install streamdevhosts.conf into the path "
			                "printed above. Without it\n" 
			                "  no client will be able to access your streaming-"
			                "server. An example can be\n"
			                "  found together with this plugin's sources.\n");
		}
		return false;
	}
	if (!opt_remux)
		opt_remux = strdup(DEFAULT_EXTERNREMUX);

	cStreamdevServer::Initialize();

	return true;
}

void cPluginStreamdevServer::Stop(void) 
{
	cStreamdevServer::Destruct();
}

cString cPluginStreamdevServer::Active(void) 
{
	if (cStreamdevServer::Active())
	{
		static const char *Message = NULL;
		if (!Message) Message = tr("Streaming active");
		return Message;
	}
	return NULL;
}

const char *cPluginStreamdevServer::MainMenuEntry(void) 
{
	if (StreamdevServerSetup.SuspendMode == smOffer && !cSuspendCtl::IsActive())
		return tr("Suspend Live TV");
	return NULL;
}

cOsdObject *cPluginStreamdevServer::MainMenuAction(void) 
{
	cControl::Launch(new cSuspendCtl);
	return NULL;
}

cMenuSetupPage *cPluginStreamdevServer::SetupMenu(void) 
{
	return new cStreamdevServerMenuSetupPage;
}

bool cPluginStreamdevServer::SetupParse(const char *Name, const char *Value) 
{
	return StreamdevServerSetup.SetupParse(Name, Value);
}

VDRPLUGINCREATOR(cPluginStreamdevServer); // Don't touch this!
