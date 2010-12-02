/*
 *  $Id: suspend.c,v 1.1.1.1 2004/12/30 22:44:21 lordjaxom Exp $
 */
 
#include "server/suspend.h"
#include "server/suspend.dat"
#include "common.h"

cSuspendLive::cSuspendLive(void)
#if VDRVERSNUM >= 10300
		: cThread("Streamdev: server suspend")
#endif
{
}

cSuspendLive::~cSuspendLive() {
	Detach();
}

void cSuspendLive::Activate(bool On) {
	Dprintf("Activate cSuspendLive %d\n", On);
	if (On)
		Start();
	else
		Stop();
}

void cSuspendLive::Stop(void) {
	if (m_Active) {
		m_Active = false;
		Cancel(3);
	}
}

void cSuspendLive::Action(void) {
#if VDRVERSNUM < 10300
	isyslog("Streamdev: Suspend Live thread started (pid = %d)", getpid());
#endif

	m_Active = true;
	while (m_Active) {
		DeviceStillPicture(suspend_mpg, sizeof(suspend_mpg));
		usleep(100000);
	}

#if VDRVERSNUM < 10300
	isyslog("Streamdev: Suspend Live thread stopped");
#endif
}

bool cSuspendCtl::m_Active = false;

cSuspendCtl::cSuspendCtl(void):
		cControl(m_Suspend = new cSuspendLive) {
	m_Active = true;
}

cSuspendCtl::~cSuspendCtl() {
	m_Active = false;
	DELETENULL(m_Suspend);
}

eOSState cSuspendCtl::ProcessKey(eKeys Key) {
	if (!m_Suspend->IsActive() || Key == kBack) {
		DELETENULL(m_Suspend);
		return osEnd;
	}
	return osContinue;
}
