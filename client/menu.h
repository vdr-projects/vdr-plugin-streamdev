/*
 *  $Id: menu.h,v 1.2 2008/04/07 14:27:28 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_MENU_H
#define VDR_STREAMDEV_MENU_H

#include <vdr/osd.h>

#include "client/remote.h"

class cStreamdevMenuRecordingItem;

// --- cStreamdevMenu --------------------------------------------------------

class cStreamdevMenu: public cOsdMenu {
private:
	enum eSubmenus {
		sub_Start = os_User,
		subSchedule,
		subTimers,
		subRecordings,
		subSuspend,
		subSyncEPG
	};

protected:
	void SuspendServer(void);

public:
	cStreamdevMenu(void);
	virtual ~cStreamdevMenu(void);

	virtual eOSState ProcessKey(eKeys Key);
};

// --- cStreamdevMenuSchedule ------------------------------------------------

class cStreamdevMenuSchedule: public cOsdMenu {
private:
	bool              m_Now;
	bool              m_Next;
	int               m_OtherChannel;
	const cSchedules *m_Schedules;
	cSchedulesLock    m_Lock;

protected:
	void PrepareSchedule(cChannel *Channel);

	eOSState Switch(void);
	eOSState Record(void);

public:
	cStreamdevMenuSchedule(void);
	virtual ~cStreamdevMenuSchedule(void);

	virtual eOSState ProcessKey(eKeys Key);
};

// --- cStreamdevMenuWhatsOn -------------------------------------------------

class cStreamdevMenuWhatsOn: public cOsdMenu {
private:
	static int               m_CurrentChannel;
	static const cEvent     *m_ScheduleEventInfo;

protected:
	eOSState Switch(void);
	eOSState Record(void);

public:
	cStreamdevMenuWhatsOn(const cSchedules *Schedules, bool Now, 
			int CurrentChannel);

	static int CurrentChannel(void) { return m_CurrentChannel; }
	static void SetCurrentChannel(int Channel) { m_CurrentChannel = Channel; }
	static const cEvent *ScheduleEventInfo(void);

	virtual eOSState ProcessKey(eKeys Key);
};

// --- cStreamdevMenuRecordings ----------------------------------------------

class cStreamdevMenuRecordings: public cOsdMenu {
private:
	char *m_Base;
	int   m_Level;

	static int               HelpKeys;
	static cRemoteRecordings Recordings;

protected:
	bool Open(bool OpenSubMenus = false);
	void SetHelpKeys();
	cRemoteRecording *cStreamdevMenuRecordings::GetRecording(
		cStreamdevMenuRecordingItem *Item);

	eOSState Select(void);
	eOSState Delete(void);
	eOSState Summary(void);

public:
	cStreamdevMenuRecordings(const char *Base = NULL, int Level = 0, 
			bool OpenSubMenus = false);
	virtual ~cStreamdevMenuRecordings();

	virtual eOSState ProcessKey(eKeys Key);
};

// --- cStreamdevMenuTimers --------------------------------------------------

class cStreamdevMenuTimers: public cOsdMenu {
protected:
	eOSState Edit(void);
  eOSState New(void);
  eOSState Delete(void);
  eOSState OnOff(void);
  eOSState Summary(void);
  
	cRemoteTimer *CurrentTimer(void);

	void Refresh(void);

public:
	cStreamdevMenuTimers(void);
	virtual ~cStreamdevMenuTimers();
	
	virtual eOSState ProcessKey(eKeys Key);
};

#endif // VDR_STREAMDEV_MENU_H

