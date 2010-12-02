/*
 *  $Id: setup.c,v 1.2 2005/05/09 20:22:29 lordjaxom Exp $
 */
 
#include <vdr/menuitems.h>

#include "server/setup.h"
#include "server/server.h"
#include "i18n.h"

cStreamdevServerSetup StreamdevServerSetup;

cStreamdevServerSetup::cStreamdevServerSetup(void) {
	MaxClients      = 5;
	StartVTPServer  = true;
	VTPServerPort   = 2004;
	StartHTTPServer = true;
	HTTPServerPort  = 3000;
	HTTPStreamType  = stPES;
	SuspendMode     = smOffer;
	AllowSuspend    = false;
	strcpy(VTPBindIP, "0.0.0.0");
	strcpy(HTTPBindIP, "0.0.0.0");
}

bool cStreamdevServerSetup::SetupParse(const char *Name, const char *Value) {
	if      (strcmp(Name, "MaxClients") == 0)      MaxClients      = atoi(Value);
	else if (strcmp(Name, "StartServer") == 0)     StartVTPServer  = atoi(Value);
	else if (strcmp(Name, "ServerPort") == 0)      VTPServerPort   = atoi(Value);
	else if (strcmp(Name, "VTPBindIP") == 0)       strcpy(VTPBindIP, Value);
	else if (strcmp(Name, "StartHTTPServer") == 0) StartHTTPServer = atoi(Value);
	else if (strcmp(Name, "HTTPServerPort") == 0)  HTTPServerPort  = atoi(Value);
	else if (strcmp(Name, "HTTPStreamType") == 0)  HTTPStreamType  = atoi(Value);
	else if (strcmp(Name, "HTTPBindIP") == 0)      strcpy(HTTPBindIP, Value);
	else if (strcmp(Name, "SuspendMode") == 0)     SuspendMode     = atoi(Value);
	else if (strcmp(Name, "AllowSuspend") == 0)    AllowSuspend    = atoi(Value);
	else return false;
	return true;
}

cStreamdevServerMenuSetupPage::cStreamdevServerMenuSetupPage(void) {
	m_NewSetup = StreamdevServerSetup;

	AddCategory (tr("Common Settings"));
	AddRangeEdit(tr("Maximum Number of Clients"), m_NewSetup.MaxClients, 0, 100);
	AddSuspEdit (tr("Suspend behaviour"),         m_NewSetup.SuspendMode);
	AddBoolEdit (tr("Client may suspend"),        m_NewSetup.AllowSuspend);
	
	AddCategory (tr("VDR-to-VDR Server"));
	AddBoolEdit (tr("Start VDR-to-VDR Server"),   m_NewSetup.StartVTPServer);
	AddShortEdit(tr("VDR-to-VDR Server Port"),    m_NewSetup.VTPServerPort);
	AddIpEdit   (tr("Bind to IP"),                m_NewSetup.VTPBindIP);

	AddCategory (tr("HTTP Server"));
	AddBoolEdit (tr("Start HTTP Server"),         m_NewSetup.StartHTTPServer);
	AddShortEdit(tr("HTTP Server Port"),          m_NewSetup.HTTPServerPort);
	AddTypeEdit (tr("HTTP Streamtype"),           m_NewSetup.HTTPStreamType);
	AddIpEdit   (tr("Bind to IP"),                m_NewSetup.HTTPBindIP);
	
	SetCurrent(Get(1));
}

cStreamdevServerMenuSetupPage::~cStreamdevServerMenuSetupPage() {
}

void cStreamdevServerMenuSetupPage::Store(void) {
	bool restart = false;
	if (m_NewSetup.StartVTPServer != StreamdevServerSetup.StartVTPServer
			|| m_NewSetup.VTPServerPort != StreamdevServerSetup.VTPServerPort
			|| strcmp(m_NewSetup.VTPBindIP, StreamdevServerSetup.VTPBindIP) != 0
			|| m_NewSetup.StartHTTPServer != StreamdevServerSetup.StartHTTPServer
			|| m_NewSetup.HTTPServerPort != StreamdevServerSetup.HTTPServerPort
			|| strcmp(m_NewSetup.HTTPBindIP, StreamdevServerSetup.HTTPBindIP) != 0) {
		restart = true;
		cStreamdevServer::Destruct();
	}
	
	SetupStore("MaxClients",      m_NewSetup.MaxClients);
	SetupStore("StartServer",     m_NewSetup.StartVTPServer);
	SetupStore("ServerPort",      m_NewSetup.VTPServerPort);
	SetupStore("VTPBindIP",       m_NewSetup.VTPBindIP);
	SetupStore("StartHTTPServer", m_NewSetup.StartHTTPServer);
	SetupStore("HTTPServerPort",  m_NewSetup.HTTPServerPort);
	SetupStore("HTTPStreamType",  m_NewSetup.HTTPStreamType);
	SetupStore("HTTPBindIP",      m_NewSetup.HTTPBindIP);
	SetupStore("SuspendMode",     m_NewSetup.SuspendMode);
	SetupStore("AllowSuspend",    m_NewSetup.AllowSuspend);

	StreamdevServerSetup = m_NewSetup;

	if (restart) 
		cStreamdevServer::Initialize();
}

