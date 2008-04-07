/*
 *  $Id: menu.c,v 1.5 2008/04/07 14:27:28 schmirl Exp $
 */
 
#include <vdr/menuitems.h>
#include <vdr/interface.h>

#include "client/menu.h"
#include "client/socket.h"
#include "i18n.h"

#define CHNUMWIDTH  (numdigits(Channels.MaxNumber()) + 1)

// --- cMenuText -------------------------------------------------------------

class cMenuText : public cOsdMenu {
public:
  cMenuText(const char *Title, const char *Text, eDvbFont Font = fontOsd);
  virtual eOSState ProcessKey(eKeys Key);
  };

// --- cStreamdevMenu --------------------------------------------------------

cStreamdevMenu::cStreamdevMenu(void):
		cOsdMenu(tr("Streaming Control")) {
	SetHasHotkeys();
	Add(new cOsdItem(hk(tr("Remote Schedule")),   (eOSState)subSchedule));
	Add(new cOsdItem(hk(tr("Remote Timers")),     (eOSState)subTimers));
	Add(new cOsdItem(hk(tr("Remote Recordings")), (eOSState)subRecordings));
	Add(new cOsdItem(hk(tr("Suspend Server")),    (eOSState)subSuspend));
	Add(new cOsdItem(hk(tr("Synchronize EPG")),   (eOSState)subSyncEPG));
}

cStreamdevMenu::~cStreamdevMenu() {
}

eOSState cStreamdevMenu::ProcessKey(eKeys Key) {
	eOSState state = cOsdMenu::ProcessKey(Key);
	switch (state) {
	case subSchedule:   return AddSubMenu(new cStreamdevMenuSchedule);
	case subTimers:     return AddSubMenu(new cStreamdevMenuTimers);
	case subRecordings: return AddSubMenu(new cStreamdevMenuRecordings);
	case subSuspend:    SuspendServer(); return osEnd;
	case subSyncEPG:		ClientSocket.SynchronizeEPG(); return osEnd;
	default:            return state;
	}
}

void cStreamdevMenu::SuspendServer(void) {
	if (ClientSocket.SuspendServer())
		INFO(tr("Server is suspended"));
	else
		ERROR(tr("Couldn't suspend Server!"));
}

// --- cStreamdevMenuEditTimer -----------------------------------------------

class cStreamdevMenuEditTimer : public cOsdMenu {
private:
  int                m_Channel;
  bool               m_AddIfConfirmed;
  cRemoteTimer      *m_Timer;
  cRemoteTimer       m_Data;
  cMenuEditDateItem *m_FirstDay;

protected:
  void SetFirstDayItem(void);

public:
  cStreamdevMenuEditTimer(cRemoteTimer *Timer, bool New = false);
  virtual ~cStreamdevMenuEditTimer();

  virtual eOSState ProcessKey(eKeys Key);
};

cStreamdevMenuEditTimer::cStreamdevMenuEditTimer(cRemoteTimer *Timer, bool New):
		cOsdMenu(tr("Edit remote timer"), 12) {
  m_FirstDay       = NULL;
  m_Timer          = Timer;
  m_AddIfConfirmed = New;

  if (m_Timer) {
    m_Data = *m_Timer;
    if (New)
      m_Data.m_Active = 1;
    m_Channel = m_Data.Channel()->Number();
		Add(new cMenuEditBitItem( tr("Active"),       &m_Data.m_Active, tfActive));
    Add(new cMenuEditChanItem(tr("Channel"),      &m_Channel));
    Add(new cMenuEditDayItem( tr("Day"),          &m_Data.m_Day));
    Add(new cMenuEditTimeItem(tr("Start"),        &m_Data.m_Start));
    Add(new cMenuEditTimeItem(tr("Stop"),         &m_Data.m_Stop));
		Add(new cMenuEditBitItem( tr("VPS"),          &m_Data.m_Active, tfVps));
    Add(new cMenuEditIntItem( tr("Priority"),     &m_Data.m_Priority, 0, 
				MAXPRIORITY));
    Add(new cMenuEditIntItem( tr("Lifetime"),     &m_Data.m_Lifetime, 0, 
				MAXLIFETIME));
    Add(new cMenuEditStrItem( tr("File"),          m_Data.m_File, 
				sizeof(m_Data.m_File), tr(FileNameChars)));
    SetFirstDayItem();
	}
}

cStreamdevMenuEditTimer::~cStreamdevMenuEditTimer() {
  if (m_Timer && m_AddIfConfirmed) {
		Dprintf("SOMETHING GETS DELETED\n");
    delete m_Timer; // apparently it wasn't confirmed
	}
}

void cStreamdevMenuEditTimer::SetFirstDayItem(void) {
  if (!m_FirstDay && !m_Data.IsSingleEvent()) {
    Add(m_FirstDay = new cMenuEditDateItem(tr("First day"),&m_Data.m_FirstDay));
    Display();
  } else if (m_FirstDay && m_Data.IsSingleEvent()) {
    Del(m_FirstDay->Index());
    m_FirstDay = NULL;
    m_Data.m_FirstDay = 0;
    Display();
  }
}

eOSState cStreamdevMenuEditTimer::ProcessKey(eKeys Key) {
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (state == osUnknown) {
    switch (Key) {
    case kOk:     
			{  
				cChannel *ch = Channels.GetByNumber(m_Channel);
				if (ch)
					m_Data.m_Channel = ch;
				else {
					ERROR(tr("*** Invalid Channel ***"));
					break;
				}
				if (!*m_Data.m_File)
					strcpy(m_Data.m_File, m_Data.Channel()->Name());
				if (m_Timer) {
					bool success = true;
					if (m_Data != *m_Timer) {
						// Timer has changed
						if ((success = ClientSocket.SaveTimer(m_Timer, m_Data))) {
							*m_Timer = m_Data;
							if (m_Timer->m_Active)
								m_Timer->m_Active = 1; 
								// allows external programs to mark active timers with 
								// values > 1 and recognize if the user has modified them
						}
					}
					if (success) {
						if (m_AddIfConfirmed)
							RemoteTimers.Add(m_Timer);
						isyslog("timer %d %s (%s)", m_Timer->Index() + 1, 
								m_AddIfConfirmed ? "added" : "modified", 
								m_Timer->m_Active ? "active" : "inactive");
						m_AddIfConfirmed = false;
					}
				}
			}
			return osBack;

		case kRed:
    case kGreen:
    case kYellow:
    case kBlue:   return osContinue;
    default: break;
		}
  }
  if (Key != kNone)
     SetFirstDayItem();
  return state;
}

// --- cMenuWhatsOnItem ------------------------------------------------------

class cMenuWhatsOnItem : public cOsdItem {
public:
  const cEvent *event;
  const cChannel *channel;
  cMenuWhatsOnItem(const cEvent *Event, cChannel *Channel); //, bool Now = false);
};

// --- cMenuEvent ------------------------------------------------------------

class cMenuEvent : public cOsdMenu {
private:
  const cEvent *event;
public:
  cMenuEvent(const cEvent *Event, bool CanSwitch = false);
  virtual void Display(void);
  virtual eOSState ProcessKey(eKeys Key);
};

// --- cStreamdevMenuWhatsOn -------------------------------------------------

int cStreamdevMenuWhatsOn::m_CurrentChannel = 0;
const cEvent *cStreamdevMenuWhatsOn::m_ScheduleEventInfo = NULL;

cStreamdevMenuWhatsOn::cStreamdevMenuWhatsOn(const cSchedules *Schedules,
		bool Now, int CurrentChannel):
		cOsdMenu(Now ? tr("What's on now?") : tr("What's on next?"), CHNUMWIDTH, 
				7, 6) {
  for (cChannel *Channel = Channels.First(); Channel; 
			Channel = Channels.Next(Channel)) {
  	if (!Channel->GroupSep()) {
      const cSchedule *Schedule 
					= Schedules->GetSchedule(Channel->GetChannelID());
      if (Schedule) {
        const cEvent *Event = Now ? Schedule->GetPresentEvent() 
						: Schedule->GetFollowingEvent();
        if (Event)
          Add(new cMenuWhatsOnItem(Event, Channel), 
							Channel->Number() == CurrentChannel);
      }
    }
  }
	m_CurrentChannel = CurrentChannel;
	SetHelp(Count() ? tr("Record") : NULL, Now ? tr("Next") : tr("Now"), 
			tr("Schedule"), tr("Switch"));
}

const cEvent *cStreamdevMenuWhatsOn::ScheduleEventInfo(void) {
	const cEvent *ei = m_ScheduleEventInfo;
	m_ScheduleEventInfo = NULL;
	return ei;
}

eOSState cStreamdevMenuWhatsOn::Switch(void) {
	cMenuWhatsOnItem *item = (cMenuWhatsOnItem*)Get(Current());
	if (item) {
		cChannel *channel 
				= Channels.GetByChannelID(item->event->ChannelID(), true);
		if (channel && cDevice::PrimaryDevice()->SwitchChannel(channel, true))
			return osEnd;
	}
	ERROR(tr("Can't switch channel!"));
	return osContinue;
}

eOSState cStreamdevMenuWhatsOn::Record(void) {
	cMenuWhatsOnItem *item = (cMenuWhatsOnItem*)Get(Current());
	if (item) {
		cRemoteTimer *timer 
				= new cRemoteTimer(item->event);
		return AddSubMenu(new cStreamdevMenuEditTimer(timer));
		// Load remote timers and see if timer exists before editing
	}
	return osContinue;
}

eOSState cStreamdevMenuWhatsOn::ProcessKey(eKeys Key) {
	eOSState state = cOsdMenu::ProcessKey(Key);
	if (state == osUnknown) {
		switch (Key) {
		case kRecord:
		case kRed:
			return Record();

		case kYellow:
			state = osBack;
		case kGreen: 
			{
				cMenuWhatsOnItem *mi = (cMenuWhatsOnItem*)Get(Current());
				if (mi) {
					m_ScheduleEventInfo = mi->event;
					m_CurrentChannel = mi->channel->Number();
				}
			}
			break;

		case kBlue:
			return Switch();

		case kOk:
			if (Count())
				return AddSubMenu(new cMenuEvent(
						((cMenuWhatsOnItem*)Get(Current()))->event, true));
			break;

		default:
			break;
		}
	}
	return state;
}

// --- cMenuScheduleItem -----------------------------------------------------

class cMenuScheduleItem : public cOsdItem {
public:
  const cEvent *event;
  cMenuScheduleItem(const cEvent *Event);
};

// --- cStreamdevMenuSchedule ------------------------------------------------

cStreamdevMenuSchedule::cStreamdevMenuSchedule(void):
		cOsdMenu("", 7, 6, 4)
{
	m_Now          = false;
	m_Next         = false;
	m_OtherChannel = -1;
	m_Schedules    = NULL;

	cChannel *channel = Channels.GetByNumber(cDevice::CurrentChannel());
	if (channel) {
		m_Schedules = cSchedules::Schedules(m_Lock);
		PrepareSchedule(channel);
		SetHelp(Count() ? tr("Record") : NULL, tr("Now"), tr("Next"));
	}
}

cStreamdevMenuSchedule::~cStreamdevMenuSchedule() {
}

void cStreamdevMenuSchedule::PrepareSchedule(cChannel *Channel) {
	Clear();
  char *buffer = NULL;
  asprintf(&buffer, tr("Schedule - %s"), Channel->Name());
  SetTitle(buffer);
  free(buffer);
  if (m_Schedules) {
     const cSchedule *Schedule = m_Schedules->GetSchedule(Channel->GetChannelID());
     if (Schedule) {
        const cEvent *PresentEvent = Schedule->GetPresentEvent(Channel->Number() == cDevice::CurrentChannel());
        time_t now = time(NULL) - Setup.EPGLinger * 60;
        for (const cEvent *Event = Schedule->Events()->First(); Event; Event = Schedule->Events()->Next(Event)) {
            if (Event->EndTime() > now || Event == PresentEvent)
               Add(new cMenuScheduleItem(Event), Event == PresentEvent);
            }
        }
     }
}

eOSState cStreamdevMenuSchedule::Switch(void) {
	if (m_OtherChannel) {
		if (Channels.SwitchTo(m_OtherChannel))
			return osEnd;
	}
	ERROR(tr("Can't switch channel!"));
	return osContinue;
}

eOSState cStreamdevMenuSchedule::Record(void) {
	cMenuScheduleItem *item = (cMenuScheduleItem*)Get(Current());
	if (item) {
		cRemoteTimer *timer 
				= new cRemoteTimer(item->event);
		return AddSubMenu(new cStreamdevMenuEditTimer(timer));
		// Load remote timers and see if timer exists before editing
	}
	return osContinue;
}

eOSState cStreamdevMenuSchedule::ProcessKey(eKeys Key) {
	eOSState state = cOsdMenu::ProcessKey(Key);
	if (state == osUnknown) {
		switch (Key) {
		case kRecord:
		case kRed:    
			return Record();

		case kGreen:
			if (m_Schedules) {
				if (!m_Now && !m_Next) {
					int channelnr = 0;
					if (Count()) {
						cChannel *channel 
								= Channels.GetByChannelID(
								((cMenuScheduleItem*)Get(Current()))->event->ChannelID(), true);
						if (channel)
							channelnr = channel->Number();
					}
					m_Now = true;
					return AddSubMenu(new cStreamdevMenuWhatsOn(m_Schedules, true, 
							channelnr));
				}
				m_Now = !m_Now;
				m_Next = !m_Next;
				return AddSubMenu(new cStreamdevMenuWhatsOn(m_Schedules, m_Now, 
						cStreamdevMenuWhatsOn::CurrentChannel()));
			}

		case kYellow:
			if (m_Schedules)
				return AddSubMenu(new cStreamdevMenuWhatsOn(m_Schedules, false, 
						cStreamdevMenuWhatsOn::CurrentChannel()));
			break;

		case kBlue:
			if (Count())
				return Switch();
			break;

		case kOk:
			if (Count())
				return AddSubMenu(new cMenuEvent(
						((cMenuScheduleItem*)Get(Current()))->event, m_OtherChannel));
			break;

		default:
			break;
		}
	} else if (!HasSubMenu()) {
		m_Now = false;
		m_Next = false;
		const cEvent *ei
				= cStreamdevMenuWhatsOn::ScheduleEventInfo();
		if (ei) {
			cChannel *channel 
					= Channels.GetByChannelID(ei->ChannelID(), true);
			if (channel) {
				PrepareSchedule(channel);
				if (channel->Number() != cDevice::CurrentChannel()) {
					m_OtherChannel = channel->Number();
					SetHelp(Count() ? tr("Record") : NULL, tr("Now"), tr("Next"), 
							tr("Switch"));
				}
				Display();
			}
		}
	}
	return state;
}

// --- cStreamdevMenuRecordingItem -------------------------------------------

class cStreamdevMenuRecordingItem: public cOsdItem {
private:
	int   m_Total;
	int   m_New;
	char *m_FileName;
	char *m_Name;

public:
	cStreamdevMenuRecordingItem(cRemoteRecording *Recording, int Level);
	virtual ~cStreamdevMenuRecordingItem();

	void IncrementCounter(bool New);
	const char *Name(void) const { return m_Name; }
	const char *FileName(void) const { return m_FileName; }
	bool IsDirectory(void) const { return m_Name != NULL; }
};

cStreamdevMenuRecordingItem::cStreamdevMenuRecordingItem(
		cRemoteRecording *Recording, int Level) {
	m_FileName = strdup(Recording->Name());
	m_Name = NULL;
	m_Total = m_New = 0;
	SetText(Recording->Title('\t', true, Level));
	if (*Text() == '\t')
		m_Name = strdup(Text() + 2);
}

cStreamdevMenuRecordingItem::~cStreamdevMenuRecordingItem() {
}

void cStreamdevMenuRecordingItem::IncrementCounter(bool New) {
	++m_Total;
	if (New) ++m_New;
	char *buffer = NULL;
	asprintf(&buffer, "%d\t%d\t%s", m_Total, m_New, m_Name);
	SetText(buffer, false);
}

// --- cStreamdevMenuRecordings ----------------------------------------------

cRemoteRecordings cStreamdevMenuRecordings::Recordings;
int cStreamdevMenuRecordings::HelpKeys = -1;

cStreamdevMenuRecordings::cStreamdevMenuRecordings(const char *Base, int Level,
		bool OpenSubMenus):
		cOsdMenu(Base ? Base : tr("Remote Recordings"), 6, 6) {
	m_Base = Base ? strdup(Base) : NULL;
	m_Level = Setup.RecordingDirs ? Level : -1;

  Display(); // this keeps the higher level menus from showing up briefly when 
	           // pressing 'Back' during replay

	if (!Base) {
		STATUS(tr("Fetching recordings..."));
		FLUSH();
	}

	if (Base || Recordings.Load()) {
		cStreamdevMenuRecordingItem *LastItem = NULL;
		char *LastItemText = NULL;
		for (cRemoteRecording *r = Recordings.First(); r; r = Recordings.Next(r)) {
			if (!Base || (strstr(r->Name(), Base) == r->Name()
						&& r->Name()[strlen(Base)] == '~')) {
				cStreamdevMenuRecordingItem *Item = new cStreamdevMenuRecordingItem(r, 
						m_Level);
				if (*Item->Text() && (!LastItem || strcmp(Item->Text(), LastItemText) 
						!= 0)) {
					Add(Item);
					LastItem = Item;
					free(LastItemText);
					LastItemText = strdup(LastItem->Text());
				} else
					delete Item;

				if (LastItem) {
					if (LastItem->IsDirectory())
						LastItem->IncrementCounter(r->IsNew());
				}
			}
		}
		free(LastItemText);
		if (Current() < 0)
			SetCurrent(First());
		else if (OpenSubMenus && Open(true))
			return;
	}

	STATUS(NULL);
	FLUSH();

	SetHelpKeys();
}

cStreamdevMenuRecordings::~cStreamdevMenuRecordings() {
	if (m_Base != NULL) free(m_Base);
	HelpKeys = -1;
}

void cStreamdevMenuRecordings::SetHelpKeys(void) {
  cStreamdevMenuRecordingItem *ri =(cStreamdevMenuRecordingItem*)Get(Current());
  int NewHelpKeys = HelpKeys;
  if (ri) {
     if (ri->IsDirectory())
        NewHelpKeys = 1;
     else {
        NewHelpKeys = 2;
        cRemoteRecording *recording = GetRecording(ri);
        if (recording && recording->Summary())
           NewHelpKeys = 3;
        }
     }
  if (NewHelpKeys != HelpKeys) {
     switch (NewHelpKeys) {
       case 0: SetHelp(NULL); break;
       case 1: SetHelp(tr("Open")); break;
       case 2:
       case 3: SetHelp(NULL, NULL, tr("Delete"), NewHelpKeys == 3 ? tr("Summary") : NULL);
							 //SetHelp(tr("Play"), tr("Rewind"), tr("Delete"), NewHelpKeys == 3 ? tr("Summary") : NULL); XXX
       }
     HelpKeys = NewHelpKeys;
     }
}

cRemoteRecording *cStreamdevMenuRecordings::GetRecording(
		cStreamdevMenuRecordingItem *Item) {
	Dprintf("looking for %s\n", Item->FileName());
  cRemoteRecording *recording = Recordings.GetByName(Item->FileName());
  if (!recording)
     ERROR(tr("Error while accessing recording!"));
  return recording;
}

bool cStreamdevMenuRecordings::Open(bool OpenSubMenus) {
	cStreamdevMenuRecordingItem *ri 
			= (cStreamdevMenuRecordingItem*)Get(Current());

	if (ri && ri->IsDirectory()) {
		const char *t = ri->Name();
		char *buffer = NULL;
		if (m_Base) {
			asprintf(&buffer, "%s~%s", m_Base, t);
			t = buffer;
		}
		AddSubMenu(new cStreamdevMenuRecordings(t, m_Level + 1, OpenSubMenus));
		if (buffer != NULL) free(buffer);
		return true;
	}
	return false;
}

eOSState cStreamdevMenuRecordings::Select(void) {	
	cStreamdevMenuRecordingItem *ri 
			= (cStreamdevMenuRecordingItem*)Get(Current());

	if (ri) {
		if (ri->IsDirectory())
			Open();
		/*else {
			cControl::Launch(new cStreamdevPlayerControl(ri->FileName()));
			return osEnd;
		} XXX */
	}
	return osContinue;
}

eOSState cStreamdevMenuRecordings::Delete(void) {
  if (HasSubMenu() || Count() == 0)
    return osContinue;
  cStreamdevMenuRecordingItem *ri 
			= (cStreamdevMenuRecordingItem*)Get(Current());
  if (ri && !ri->IsDirectory()) {
    if (Interface->Confirm(tr("Delete recording?"))) {
      cRemoteRecording *recording = GetRecording(ri);
      if (recording) {
        if (ClientSocket.DeleteRecording(recording)) {
          cOsdMenu::Del(Current());
          Recordings.Del(recording);
          Display();
          if (!Count())
            return osBack;
        }
      }
    }
  }
  return osContinue;
}

eOSState cStreamdevMenuRecordings::Summary(void) {
  if (HasSubMenu() || Count() == 0)
    return osContinue;
  cStreamdevMenuRecordingItem *ri=(cStreamdevMenuRecordingItem *)Get(Current());
  if (ri && !ri->IsDirectory()) {
    cRemoteRecording *recording = GetRecording(ri);
    if (recording && recording->Summary() && *recording->Summary())
       return AddSubMenu(new cMenuText(tr("Summary"), recording->Summary()));
  }
  return osContinue;
}

eOSState cStreamdevMenuRecordings::ProcessKey(eKeys Key) {
	bool HadSubMenu = HasSubMenu();
	eOSState state = cOsdMenu::ProcessKey(Key);

	if (state == osUnknown) {
		switch (Key) {
		case kOk:     
		case kRed:    return Select();
		case kYellow: return Delete();
		case kBlue:   return Summary();
		default:      break;
		}
	}

	if (Key == kYellow && HadSubMenu && !HasSubMenu()) {
		cOsdMenu::Del(Current());
		if (!Count())
			return osBack;
		Display();
	}

	if (!HasSubMenu() && Key != kNone)
		SetHelpKeys();
	return state;
}

// --- cStreamdevMenuTimerItem -----------------------------------------------

class cStreamdevMenuTimerItem: public cOsdItem {
private:
	cRemoteTimer *m_Timer;

public:
	cStreamdevMenuTimerItem(cRemoteTimer *Timer);
	virtual ~cStreamdevMenuTimerItem();

	virtual void Set(void);

	cRemoteTimer *Timer(void) const { return m_Timer; }
};

cStreamdevMenuTimerItem::cStreamdevMenuTimerItem(cRemoteTimer *Timer) {
	m_Timer = Timer;
	Set();
}

cStreamdevMenuTimerItem::~cStreamdevMenuTimerItem() {
}

void cStreamdevMenuTimerItem::Set(void) {
  char *buffer = NULL;
  asprintf(&buffer, "%c\t%d\t%s\t%02d:%02d\t%02d:%02d\t%s",
                    !m_Timer->Active() ? ' ' : 
										m_Timer->FirstDay() ? '!' : 
										/*m_Timer->Recording() ? '#' :*/ '>',
                    m_Timer->Channel()->Number(),
                    m_Timer->PrintDay(m_Timer->Day()),
                    m_Timer->Start() / 100,
                    m_Timer->Start() % 100,
                    m_Timer->Stop() / 100,
                    m_Timer->Stop() % 100,
                    m_Timer->File());
  SetText(buffer, false);
}

// --- cStreamdevMenuTimers --------------------------------------------------

cStreamdevMenuTimers::cStreamdevMenuTimers(void):
		cOsdMenu(tr("Remote Timers"), 2, CHNUMWIDTH, 10, 6, 6) {
	Refresh();
  SetHelp(tr("Edit"), tr("New"), tr("Delete"), tr("On/Off"));
}

cStreamdevMenuTimers::~cStreamdevMenuTimers() {
}

eOSState cStreamdevMenuTimers::ProcessKey(eKeys Key) {
	int timerNum = HasSubMenu() ? Count() : -1;
	eOSState state = cOsdMenu::ProcessKey(Key);

	if (state == osUnknown) {
		switch (Key) {
		case kOk:     return Summary();
		case kRed:    return Edit();
		case kGreen:  return New();
		case kYellow: return Delete();
		case kBlue:   OnOff(); break;
		default:      break;
		}
	}

	if (timerNum >= 0 && !HasSubMenu()) {
		Refresh();
		Display();
	}
	return state;
}

eOSState cStreamdevMenuTimers::Edit(void) {
  if (HasSubMenu() || Count() == 0)
    return osContinue;
  isyslog("Streamdev: Editing remote timer %d", CurrentTimer()->Index() + 1);
  return AddSubMenu(new cStreamdevMenuEditTimer(CurrentTimer()));
}

eOSState cStreamdevMenuTimers::New(void) {
  if (HasSubMenu())
    return osContinue;
  return AddSubMenu(new cStreamdevMenuEditTimer(new cRemoteTimer, true));
}

eOSState cStreamdevMenuTimers::Delete(void) {
	cRemoteTimer *ti = CurrentTimer();
	if (ti) {
		if (Interface->Confirm(tr("Delete timer?"))) {
			int idx = ti->Index();
			if (ClientSocket.DeleteTimer(ti)) {
				RemoteTimers.Del(ti);
				cOsdMenu::Del(Current());
				isyslog("Streamdev: Remote timer %d deleted", idx + 1);
			}
			Refresh();
			Display();
		}
	}
	return osContinue;
}

eOSState cStreamdevMenuTimers::OnOff(void) {
	cRemoteTimer *timer = CurrentTimer();
	if (timer) {
		cRemoteTimer data = *timer;
		data.OnOff();
		if (data.FirstDay())
			isyslog("Streamdev: Remote timer %d first day set to %s", 
					data.Index() + 1, data.PrintFirstDay());
    else
      isyslog("Streamdev: Remote timer %d %sactivated", data.Index() + 1, 
					data.Active() ? "" : "de");

		if (ClientSocket.SaveTimer(timer, data)) {
			*timer = data;
			RefreshCurrent();
			DisplayCurrent(true);
		} else {
			Refresh();
			Display();
		}
	}
	return osContinue;
}

eOSState cStreamdevMenuTimers::Summary(void) {
	if (HasSubMenu() || Count() == 0)
		return osContinue;

	cRemoteTimer *ti = CurrentTimer();
	if (ti && ti->Summary() != "")
		return AddSubMenu(new cMenuText(tr("Summary"), ti->Summary().c_str()));

	return osContinue;
}

cRemoteTimer *cStreamdevMenuTimers::CurrentTimer(void) {
	cStreamdevMenuTimerItem *item = (cStreamdevMenuTimerItem*)Get(Current());
	return item ? item->Timer() : NULL;
}

void cStreamdevMenuTimers::Refresh(void) {
	Clear();
	if (RemoteTimers.Load()) {
		for (cRemoteTimer *t = RemoteTimers.First(); t; t = RemoteTimers.Next(t)) {
			Add(new cStreamdevMenuTimerItem(t));
		}
	}
}
