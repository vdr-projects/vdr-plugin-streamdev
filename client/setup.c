/*
 *  $Id: setup.c,v 1.6 2008/04/08 14:18:16 schmirl Exp $
 */
 
#include <vdr/menuitems.h>

#include "client/setup.h"
#include "client/device.h"

cStreamdevClientSetup StreamdevClientSetup;

cStreamdevClientSetup::cStreamdevClientSetup(void) {
	StartClient   = false;
	RemotePort    = 2004;
	StreamFilters = false;
	SyncEPG       = false;
	HideMenuEntry = false;
	strcpy(RemoteIp, "");
}

bool cStreamdevClientSetup::SetupParse(const char *Name, const char *Value) {
	if      (strcmp(Name, "StartClient") == 0)   StartClient = atoi(Value);
	else if (strcmp(Name, "RemoteIp") == 0) {
		if (strcmp(Value, "-none-") == 0)
			strcpy(RemoteIp, "");
		else
			strcpy(RemoteIp, Value);
	}
	else if (strcmp(Name, "RemotePort") == 0)    RemotePort = atoi(Value);
	else if (strcmp(Name, "StreamFilters") == 0) StreamFilters = atoi(Value);
	else if (strcmp(Name, "SyncEPG") == 0)       SyncEPG = atoi(Value);
	else if (strcmp(Name, "HideMenuEntry") == 0) HideMenuEntry = atoi(Value);
	else return false;
	return true;
}

cStreamdevClientMenuSetupPage::cStreamdevClientMenuSetupPage(void) {
	m_NewSetup = StreamdevClientSetup;

	AddBoolEdit (tr("Hide Mainmenu Entry"),m_NewSetup.HideMenuEntry);
	AddBoolEdit (tr("Start Client"),       m_NewSetup.StartClient);
	AddIpEdit   (tr("Remote IP"),          m_NewSetup.RemoteIp);
	AddShortEdit(tr("Remote Port"),        m_NewSetup.RemotePort);
	AddBoolEdit (tr("Filter Streaming"),   m_NewSetup.StreamFilters);
	AddBoolEdit (tr("Synchronize EPG"),    m_NewSetup.SyncEPG);
	SetCurrent(Get(0));
}

cStreamdevClientMenuSetupPage::~cStreamdevClientMenuSetupPage() {
}

void cStreamdevClientMenuSetupPage::Store(void) {
	if (m_NewSetup.StartClient != StreamdevClientSetup.StartClient) {
		if (m_NewSetup.StartClient)
			cStreamdevDevice::Init();
	}

	SetupStore("StartClient", m_NewSetup.StartClient);
	if (strcmp(m_NewSetup.RemoteIp, "") == 0)
		SetupStore("RemoteIp", "-none-");
	else
		SetupStore("RemoteIp",    m_NewSetup.RemoteIp);
	SetupStore("RemotePort",    m_NewSetup.RemotePort);
	SetupStore("StreamFilters", m_NewSetup.StreamFilters);
	SetupStore("SyncEPG",       m_NewSetup.SyncEPG);
	SetupStore("HideMenuEntry", m_NewSetup.HideMenuEntry);

	StreamdevClientSetup = m_NewSetup;

	cStreamdevDevice::ReInit();
}

