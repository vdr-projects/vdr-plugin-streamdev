/*
 *  $Id: setup.c,v 1.10 2010/07/19 13:49:31 schmirl Exp $
 */
 
#include <vdr/menuitems.h>

#include "server/setup.h"
#include "server/server.h"

cStreamdevServerSetup StreamdevServerSetup;

cStreamdevServerSetup::cStreamdevServerSetup(void) {
	HideMenuEntry   = false;
	MaxClients      = 5;
	StartVTPServer  = true;
	VTPServerPort   = 2004;
	LoopPrevention  = false;
	StartHTTPServer = true;
	HTTPServerPort  = 3000;
	HTTPStreamType  = stTS;
	StartIGMPServer = false;
	IGMPClientPort  = 1234;
	IGMPStreamType  = stTS;
	SuspendMode     = smAlways;
	AllowSuspend    = false;
	strcpy(VTPBindIP, "0.0.0.0");
	strcpy(HTTPBindIP, "0.0.0.0");
	strcpy(IGMPBindIP, "0.0.0.0");
}

bool cStreamdevServerSetup::SetupParse(const char *Name, const char *Value) {
	if      (strcmp(Name, "HideMenuEntry") == 0)   HideMenuEntry   = atoi(Value);
	else if (strcmp(Name, "MaxClients") == 0)      MaxClients      = atoi(Value);
	else if (strcmp(Name, "StartServer") == 0)     StartVTPServer  = atoi(Value);
	else if (strcmp(Name, "ServerPort") == 0)      VTPServerPort   = atoi(Value);
	else if (strcmp(Name, "VTPBindIP") == 0)       strcpy(VTPBindIP, Value);
	else if (strcmp(Name, "LoopPrevention") == 0)  LoopPrevention  = atoi(Value);
	else if (strcmp(Name, "StartHTTPServer") == 0) StartHTTPServer = atoi(Value);
	else if (strcmp(Name, "HTTPServerPort") == 0)  HTTPServerPort  = atoi(Value);
	else if (strcmp(Name, "HTTPStreamType") == 0)  HTTPStreamType  = atoi(Value);
	else if (strcmp(Name, "HTTPBindIP") == 0)      strcpy(HTTPBindIP, Value);
	else if (strcmp(Name, "StartIGMPServer") == 0) StartIGMPServer = atoi(Value);
	else if (strcmp(Name, "IGMPClientPort") == 0)  IGMPClientPort  = atoi(Value);
	else if (strcmp(Name, "IGMPStreamType") == 0)  IGMPStreamType  = atoi(Value);
	else if (strcmp(Name, "IGMPBindIP") == 0)      strcpy(IGMPBindIP, Value);
	else if (strcmp(Name, "SuspendMode") == 0)     SuspendMode     = atoi(Value);
	else if (strcmp(Name, "AllowSuspend") == 0)    AllowSuspend    = atoi(Value);
	else return false;
	return true;
}

const char* cStreamdevServerMenuSetupPage::StreamTypes[st_Count - 1] = {
	"TS",
	"PES",
	"PS",
	"ES",
	"EXT"
};

const char* cStreamdevServerMenuSetupPage::SuspendModes[sm_Count] = {
	trNOOP("Offer suspend mode"),
	trNOOP("Always suspended"),
	trNOOP("Never suspended")
};

cStreamdevServerMenuSetupPage::cStreamdevServerMenuSetupPage(void) {
	m_NewSetup = StreamdevServerSetup;

	Set();
}

cStreamdevServerMenuSetupPage::~cStreamdevServerMenuSetupPage() {
}

void cStreamdevServerMenuSetupPage::Set(void) {
	static const char* modes[sm_Count];
	for (int i = 0; i < sm_Count; i++)
		modes[i] = tr(SuspendModes[i]);

	int current = Current();
	Clear();
	AddCategory (tr("Common Settings"));
	Add(new cMenuEditBoolItem(tr("Hide Mainmenu Entry"),       &m_NewSetup.HideMenuEntry));
	Add(new cMenuEditIntItem (tr("Maximum Number of Clients"), &m_NewSetup.MaxClients, 0, 100));

	Add(new cMenuEditStraItem(tr("Suspend behaviour"),         &m_NewSetup.SuspendMode, sm_Count, modes));
	if (m_NewSetup.SuspendMode == smOffer)
		Add(new cMenuEditBoolItem(tr("Client may suspend"),        &m_NewSetup.AllowSuspend));
	
	AddCategory (tr("VDR-to-VDR Server"));
	Add(new cMenuEditBoolItem(tr("Start VDR-to-VDR Server"),   &m_NewSetup.StartVTPServer));
	Add(new cMenuEditIntItem (tr("VDR-to-VDR Server Port"),    &m_NewSetup.VTPServerPort, 0, 65535));
	Add(new cMenuEditIpItem  (tr("Bind to IP"),                 m_NewSetup.VTPBindIP));
	if (cPluginManager::CallFirstService(LOOP_PREVENTION_SERVICE))
		Add(new cMenuEditBoolItem(tr("Loop Prevention"),           &m_NewSetup.LoopPrevention));

	AddCategory (tr("HTTP Server"));
	Add(new cMenuEditBoolItem(tr("Start HTTP Server"),         &m_NewSetup.StartHTTPServer));
	Add(new cMenuEditIntItem (tr("HTTP Server Port"),          &m_NewSetup.HTTPServerPort, 0, 65535));
	Add(new cMenuEditStraItem(tr("HTTP Streamtype"),           &m_NewSetup.HTTPStreamType, st_Count - 1, StreamTypes));
	Add(new cMenuEditIpItem  (tr("Bind to IP"),                 m_NewSetup.HTTPBindIP));
	AddCategory (tr("Multicast Streaming Server"));
	Add(new cMenuEditBoolItem(tr("Start IGMP Server"),         &m_NewSetup.StartIGMPServer));
	Add(new cMenuEditIntItem (tr("Multicast Client Port"),     &m_NewSetup.IGMPClientPort, 0, 65535));
	Add(new cMenuEditStraItem(tr("Multicast Streamtype"),      &m_NewSetup.IGMPStreamType, st_Count - 1, StreamTypes));
	Add(new cMenuEditIpItem  (tr("Bind to IP"),                 m_NewSetup.IGMPBindIP));
	SetCurrent(Get(current));
	Display();
}

void cStreamdevServerMenuSetupPage::AddCategory(const char *Title) {

  cString str = cString::sprintf("--- %s -------------------------------------------------"
   		"---------------", Title );

  cOsdItem *item = new cOsdItem(*str);
  item->SetSelectable(false);
  Add(item);
}
	
void cStreamdevServerMenuSetupPage::Store(void) {
	bool restart = false;
	if (m_NewSetup.StartVTPServer != StreamdevServerSetup.StartVTPServer
			|| m_NewSetup.VTPServerPort != StreamdevServerSetup.VTPServerPort
			|| strcmp(m_NewSetup.VTPBindIP, StreamdevServerSetup.VTPBindIP) != 0
			|| m_NewSetup.StartHTTPServer != StreamdevServerSetup.StartHTTPServer
			|| m_NewSetup.HTTPServerPort != StreamdevServerSetup.HTTPServerPort
			|| strcmp(m_NewSetup.HTTPBindIP, StreamdevServerSetup.HTTPBindIP) != 0
			|| m_NewSetup.StartIGMPServer != StreamdevServerSetup.StartIGMPServer
			|| m_NewSetup.IGMPClientPort != StreamdevServerSetup.IGMPClientPort
			|| strcmp(m_NewSetup.IGMPBindIP, StreamdevServerSetup.IGMPBindIP) != 0) {
		restart = true;
		cStreamdevServer::Destruct();
	}
	
	SetupStore("HideMenuEntry",   m_NewSetup.HideMenuEntry);
	SetupStore("MaxClients",      m_NewSetup.MaxClients);
	SetupStore("StartServer",     m_NewSetup.StartVTPServer);
	SetupStore("ServerPort",      m_NewSetup.VTPServerPort);
	SetupStore("VTPBindIP",       m_NewSetup.VTPBindIP);
	SetupStore("LoopPrevention",  m_NewSetup.LoopPrevention);
	SetupStore("StartHTTPServer", m_NewSetup.StartHTTPServer);
	SetupStore("HTTPServerPort",  m_NewSetup.HTTPServerPort);
	SetupStore("HTTPStreamType",  m_NewSetup.HTTPStreamType);
	SetupStore("HTTPBindIP",      m_NewSetup.HTTPBindIP);
	SetupStore("StartIGMPServer", m_NewSetup.StartIGMPServer);
	SetupStore("IGMPClientPort",  m_NewSetup.IGMPClientPort);
	SetupStore("IGMPStreamType",  m_NewSetup.IGMPStreamType);
	SetupStore("IGMPBindIP",      m_NewSetup.IGMPBindIP);
	SetupStore("SuspendMode",     m_NewSetup.SuspendMode);
	SetupStore("AllowSuspend",    m_NewSetup.AllowSuspend);

	StreamdevServerSetup = m_NewSetup;

	if (restart) 
		cStreamdevServer::Initialize();
}

eOSState cStreamdevServerMenuSetupPage::ProcessKey(eKeys Key) {
	int oldMode = m_NewSetup.SuspendMode;
	eOSState state = cMenuSetupPage::ProcessKey(Key);
	if (oldMode != m_NewSetup.SuspendMode)
		Set();
	return state;
}
