/*
 *  $Id: setup.c,v 1.2 2005/02/08 15:34:38 lordjaxom Exp $
 */
 
#include <vdr/menuitems.h>

#include "client/setup.h"
#include "client/device.h"
#include "i18n.h"

cStreamdevClientSetup StreamdevClientSetup;

cStreamdevClientSetup::cStreamdevClientSetup(void) {
	StartClient   = false;
	RemotePort    = 2004;
#if VDRVERSNUM >= 10300
	StreamFilters = false;
#endif
	SyncEPG       = false;
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
#if VDRVERSNUM >= 10300
	else if (strcmp(Name, "StreamFilters") == 0) StreamFilters = atoi(Value);
#endif
	else if (strcmp(Name, "SyncEPG") == 0)       SyncEPG = atoi(Value);
	else return false;
	return true;
}

cStreamdevClientMenuSetupPage::cStreamdevClientMenuSetupPage(void) {
	m_NewSetup = StreamdevClientSetup;

	AddBoolEdit (tr("Start Client"),       m_NewSetup.StartClient);
	AddIpEdit   (tr("Remote IP"),          m_NewSetup.RemoteIp);
	AddShortEdit(tr("Remote Port"),        m_NewSetup.RemotePort);
#if VDRVERSNUM >= 10300
	AddBoolEdit (tr("Filter Streaming"),   m_NewSetup.StreamFilters);
#endif
	AddBoolEdit (tr("Synchronize EPG"),    m_NewSetup.SyncEPG);
	SetCurrent(Get(0));
}

cStreamdevClientMenuSetupPage::~cStreamdevClientMenuSetupPage() {
}

void cStreamdevClientMenuSetupPage::Store(void) {
	if (m_NewSetup.StartClient != StreamdevClientSetup.StartClient) {
		if (m_NewSetup.StartClient)
			cStreamdevDevice::Init();
		else
			INFO(tr("Please restart VDR to activate changes"));
	}

	SetupStore("StartClient", m_NewSetup.StartClient);
	if (strcmp(m_NewSetup.RemoteIp, "") == 0)
		SetupStore("RemoteIp", "-none-");
	else
		SetupStore("RemoteIp",    m_NewSetup.RemoteIp);
	SetupStore("RemotePort",    m_NewSetup.RemotePort);
#if VDRVERSNUM >= 10300
	SetupStore("StreamFilters", m_NewSetup.StreamFilters);
#endif
	SetupStore("SyncEPG",       m_NewSetup.SyncEPG);

	StreamdevClientSetup = m_NewSetup;

	cStreamdevDevice::ReInit();
}

