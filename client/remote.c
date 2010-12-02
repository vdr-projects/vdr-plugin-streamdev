/*
 *  $Id: remote.c,v 1.4 2005/04/24 16:26:14 lordjaxom Exp $
 */

#include <ctype.h>
 
#include "client/remote.h"
#include "client/device.h"
#include "common.h"

cRemoteTimers RemoteTimers;

// --- cRemoteRecording ------------------------------------------------------

cRemoteRecording::cRemoteRecording(const char *Text) {
	m_IsValid     = false;
	m_Index       = -1;
	m_IsNew       = false;
	m_TitleBuffer = NULL;

	char *ptr;
	char *timestr;
	int idx;

	Dprintf("text: %s\n", Text);

	m_Index = strtoul(Text, &ptr, 10);
	Dprintf("index: %d\n", m_Index);
	if (*ptr == '\0' || *++ptr == '\0' ) return;
	timestr = ptr;
	while (*ptr != '\0' && !isspace(*ptr)) ++ptr;
	if (*ptr == '\0' || *++ptr == '\0') return;
	while (*ptr != '\0' && *ptr != '*' && !isspace(*ptr)) ++ptr;
	if (*ptr == '*') m_IsNew = true;
	Dprintf("new: %d\n", m_IsNew);
	*(ptr++) = '\0';
	m_StartTime = timestr;
	idx = -1;
	while ((idx = m_StartTime.find(' ', idx + 1)) != -1) m_StartTime[idx] = '\t';
	Dprintf("m_Start: %s\n", m_StartTime.c_str());
	if (*ptr == 0) return;
	if (isspace(*ptr)) ++ptr;
	if (*ptr == 0) return;
	m_Name = ptr;
	Dprintf("file: %s\n", m_Name.c_str());
	m_IsValid = true;
}

cRemoteRecording::~cRemoteRecording(void) {
}

bool cRemoteRecording::operator==(const cRemoteRecording &Recording) {
	return m_IsValid   == Recording.m_IsValid
			&& m_Index     == Recording.m_Index
			&& m_StartTime == Recording.m_StartTime
			&& m_Name      == Recording.m_Name;
}

void cRemoteRecording::ParseInfo(const char *Text) {
	m_Summary = strreplace(strdup(Text), '|', '\n');
}

const char *cRemoteRecording::Title(char Delimiter, bool NewIndicator,
		int Level) {
  char New = NewIndicator && IsNew() ? '*' : ' ';

  if (m_TitleBuffer != NULL) {
		free(m_TitleBuffer);
		m_TitleBuffer = NULL;
	}

  if (Level < 0 || Level == HierarchyLevels()) {
    char *s;
	const char *t;
    if (Level > 0 && (t = strrchr(m_Name.c_str(), '~')) != NULL)
      t++;
    else
      t = m_Name.c_str();
					
	asprintf(&m_TitleBuffer, "%s%c%c%s", m_StartTime.c_str(), New, Delimiter, t);
    // let's not display a trailing '~':
    stripspace(m_TitleBuffer);
    s = &m_TitleBuffer[strlen(m_TitleBuffer) - 1];
    if (*s == '~')
      *s = 0;
  } else if (Level < HierarchyLevels()) {
    const char *s = m_Name.c_str();
    const char *p = s;
    while (*++s) {
      if (*s == '~') {
        if (Level--)
          p = s + 1;
        else
          break;
      }
    }
    m_TitleBuffer = MALLOC(char, s - p + 3);
    *m_TitleBuffer = Delimiter;
    *(m_TitleBuffer + 1) = Delimiter;
    strn0cpy(m_TitleBuffer + 2, p, s - p + 1);
  } else
    return "";
  return m_TitleBuffer;
}

int cRemoteRecording::HierarchyLevels(void)
{
  const char *s = m_Name.c_str();
  int level = 0;
  while (*++s) {
    if (*s == '~') ++level;
  }
  return level;
}

// --- cRemoteRecordings -----------------------------------------------------

bool cRemoteRecordings::Load(void) {
	Clear();
	return ClientSocket.LoadRecordings(*this);
}

cRemoteRecording *cRemoteRecordings::GetByName(const char *Name) {
	for (cRemoteRecording *r = First(); r; r = Next(r))
		if (strcmp(r->Name(), Name) == 0)
			return r;
	return NULL;
}

// --- cRemoteTimer ----------------------------------------------------------

cRemoteTimer::cRemoteTimer(const char *Text) {
	m_IsValid   = false;
	m_Index     = -1;
	m_Active    = -1;
	m_Day       = -1;
	m_Start     = -1;
	m_Stop      = -1;
	m_StartTime = 0;
	m_StopTime  = 0;
	m_Priority  = -1;
	m_Lifetime  = -1;
	m_File[0]   = '\0';
	m_FirstDay  = 0;
	m_Buffer    = NULL;
	m_Channel   = NULL;

	char *tmpbuf;
	char *ptr;

	Dprintf("text: %s\n", Text);

	m_Index = strtoul(Text, &ptr, 10);
	Dprintf("index: %d\n", m_Index);
	if (*ptr == '\0' || *++ptr == '\0') return;
	m_Active = strtoul(ptr, &ptr, 10);
	Dprintf("m_Active: %d\n", m_Active);
	if (*ptr == '\0' || *++ptr == '\0') return;

	tmpbuf = ptr;
	while (*ptr != '\0' && *ptr != ':') ++ptr;
	if (*ptr == '\0') return;
	*(ptr++)= '\0';
	if (isnumber(tmpbuf))
		m_Channel = Channels.GetByNumber(strtoul(tmpbuf, NULL, 10));
	else
		m_Channel = Channels.GetByChannelID(tChannelID::FromString(tmpbuf));
	Dprintf("channel no.: %d\n", m_Channel->Number());
	
	tmpbuf = ptr;
	while (*ptr != '\0' && *ptr != ':') ++ptr;
	if (*ptr == '\0') return;
	*(ptr++) = '\0';
	m_Day = ParseDay(tmpbuf, &m_FirstDay);
	Dprintf("Day: %d\n", m_Day);
	m_Start = strtoul(ptr, &ptr, 10);
	Dprintf("Start: %d\n", m_Start);
	if (*ptr == '\0' || *++ptr == '\0') return;
	m_Stop = strtoul(ptr, &ptr, 10);
	Dprintf("Stop: %d\n", m_Stop);
	if (*ptr == '\0' || *++ptr == '\0') return;
	m_Priority = strtoul(ptr, &ptr, 10);
	Dprintf("Prio: %d\n", m_Priority);
	if (*ptr == '\0' || *++ptr == '\0') return;
	m_Lifetime = strtoul(ptr, &ptr, 10);
	Dprintf("Lifetime: %d\n", m_Lifetime);
	if (*ptr == '\0' || *++ptr == '\0') return;
	tmpbuf = ptr;
	while (*ptr != '\0' && *ptr != ':') ++ptr;
	if (*ptr == '\0') return;
	*(ptr++) = '\0';
	strncpy(m_File, tmpbuf, MaxFileName);
	Dprintf("file: %s\n", m_File);
	if (*ptr != '\0') m_Summary = ptr;
	Dprintf("summary: %s\n", m_Summary.c_str());
	m_IsValid = true;
}

#if VDRVERSNUM < 10300
cRemoteTimer::cRemoteTimer(const cEventInfo *EventInfo) {
	time_t tstart = EventInfo->GetTime();
	time_t tstop = tstart + EventInfo->GetDuration() + Setup.MarginStop * 60;
	tstart -= Setup.MarginStart * 60;
	struct tm tm_r;
	struct tm *time = localtime_r(&tstart, &tm_r);
	const char *title = EventInfo->GetTitle();
	cChannel *channel = Channels.GetByChannelID(EventInfo->GetChannelID(), true);
#else
cRemoteTimer::cRemoteTimer(const cEvent *Event) {
	time_t tstart = Event->StartTime();
	time_t tstop = tstart + Event->Duration() + Setup.MarginStop * 60;
	tstart -= Setup.MarginStart * 60;
	struct tm tm_r;
	struct tm *time = localtime_r(&tstart, &tm_r);
	const char *title = Event->Title();
	cChannel *channel = Channels.GetByChannelID(Event->ChannelID(), true);
#endif

	m_IsValid   = true;
	m_Index     = -1;
	m_Active    = true;
	m_Day       = time->tm_mday;
	m_Start     = time->tm_hour * 100 + time->tm_min;
	time        = localtime_r(&tstop, &tm_r);
	m_Stop      = time->tm_hour * 100 + time->tm_min;
	m_StartTime = 0;
	m_StopTime  = 0;
	if (m_Stop >= 2400) m_Stop -= 2400;
	m_Priority  = Setup.DefaultPriority;
	m_Lifetime  = Setup.DefaultLifetime;
	m_File[0]   = '\0';
	if (!isempty(title))
		strn0cpy(m_File, title, sizeof(m_File));
	m_FirstDay  = 0;
	m_Channel   = channel;
}

cRemoteTimer::cRemoteTimer(void) {
	time_t t = time(NULL);
	struct tm tm_r;
	struct tm *now = localtime_r(&t, &tm_r);

	m_IsValid   = true;
	m_Index     = -1;
	m_Active    = -1;
	m_Day       = now->tm_mday;
	m_Start     = now->tm_hour * 100 + now->tm_min;
	m_Stop      = now->tm_hour * 60 + now->tm_min + Setup.InstantRecordTime;
	m_Stop      = (m_Stop / 60) * 100 + (m_Stop % 60);
	if (m_Stop >= 2400) m_Stop -= 2400;
	m_StartTime = 0;
	m_StopTime  = 0;
	m_Priority  = Setup.DefaultPriority;
	m_Lifetime  = Setup.DefaultLifetime;
	m_File[0]   = '\0';
	m_FirstDay  = 0;
	m_Buffer    = NULL;
	m_Channel   = Channels.GetByNumber(cDevice::CurrentChannel());
}

cRemoteTimer::~cRemoteTimer() {
	if (m_Buffer  != NULL) free(m_Buffer);
}

cRemoteTimer &cRemoteTimer::operator=(const cRemoteTimer &Timer) {
	Dprintf("\n\n\n\nOPÜERATHVBDÖLJVG\n\n\n");
	m_IsValid = Timer.m_IsValid;
	m_Index = Timer.m_Index;
	m_Active = Timer.m_Active;
	m_Day = Timer.m_Day;
	m_Start = Timer.m_Start;
	m_Stop = Timer.m_Stop;
	m_Priority = Timer.m_Priority;
	m_Lifetime = Timer.m_Lifetime;
	m_FirstDay = Timer.m_FirstDay;
	m_Channel = Timer.m_Channel;
	m_Summary = Timer.m_Summary;
	return *this;
}

bool cRemoteTimer::operator==(const cRemoteTimer &Timer) {
	return m_IsValid   == Timer.m_IsValid
			&& m_Index     == Timer.m_Index
			&& m_Active    == Timer.m_Active
			&& m_Day       == Timer.m_Day
			&& m_Start     == Timer.m_Start
			&& m_Stop      == Timer.m_Stop
			&& m_Priority  == Timer.m_Priority
			&& m_Lifetime  == Timer.m_Lifetime
			&& m_FirstDay  == Timer.m_FirstDay
			&& m_Channel   == Timer.m_Channel
			&& strcmp(m_File, Timer.m_File) == 0
			&& m_Summary   == Timer.m_Summary;
}

int cRemoteTimer::ParseDay(const char *s, time_t *FirstDay) {
  char *tail;
  int d = strtol(s, &tail, 10);
  if (FirstDay)
     *FirstDay = 0;
  if (tail && *tail) {
     d = 0;
     if (tail == s) {
        const char *first = strchr(s, '@');
        int l = first ? first - s : strlen(s);
        if (l == 7) {
           for (const char *p = s + 6; p >= s; p--) {
               d <<= 1;
               d |= (*p != '-');
               }
           d |= 0x80000000;
           }
        if (FirstDay && first) {
           ++first;
           if (strlen(first) == 10) {
              struct tm tm_r;
              if (3 == sscanf(first, "%d-%d-%d", &tm_r.tm_year, &tm_r.tm_mon, &tm_r.tm_mday)) {
                 tm_r.tm_year -= 1900;
                 tm_r.tm_mon--;
                 tm_r.tm_hour = tm_r.tm_min = tm_r.tm_sec = 0;
                 tm_r.tm_isdst = -1; // makes sure mktime() will determine the correct DST setting
                 *FirstDay = mktime(&tm_r);
                 }
              }
           else
              d = 0;
           }
        }
     }
  else if (d < 1 || d > 31)
     d = 0;
  return d;
}

const char *cRemoteTimer::PrintDay(int d, time_t FirstDay) {
#define DAYBUFFERSIZE 32
  static char buffer[DAYBUFFERSIZE];
  if ((d & 0x80000000) != 0) {
     char *b = buffer;
     const char *w = tr("MTWTFSS");
     while (*w) {
           *b++ = (d & 1) ? *w : '-';
           d >>= 1;
           w++;
           }
     if (FirstDay) {
        struct tm tm_r;
        localtime_r(&FirstDay, &tm_r);
        b += strftime(b, DAYBUFFERSIZE - (b - buffer), "@%Y-%m-%d", &tm_r);
        }
     *b = 0;
     }
  else
     sprintf(buffer, "%d", d);
  return buffer;
}

const char *cRemoteTimer::PrintFirstDay(void) const {
  if (m_FirstDay) {
     const char *s = PrintDay(m_Day, m_FirstDay);
     if (strlen(s) == 18)
        return s + 8;
     }
  return ""; // not NULL, so the caller can always use the result
}

void cRemoteTimer::OnOff(void) {
  if (IsSingleEvent())
     m_Active = !m_Active;
  else if (m_FirstDay) {
     m_FirstDay = 0;
     m_Active = false;
     }
  else if (m_Active)
     Skip();
  else
     m_Active = true;
  Matches(); // refresh m_Start and end time
}

time_t cRemoteTimer::SetTime(time_t t, int SecondsFromMidnight) {
  struct tm tm_r;
  tm tm = *localtime_r(&t, &tm_r);
  tm.tm_hour = SecondsFromMidnight / 3600;
  tm.tm_min = (SecondsFromMidnight % 3600) / 60;
  tm.tm_sec =  SecondsFromMidnight % 60;
  tm.tm_isdst = -1; // makes sure mktime() will determine the correct DST setting
  return mktime(&tm);
}

bool cRemoteTimer::Matches(time_t t) {
  m_StartTime = m_StopTime = 0;
  if (t == 0)
     t = time(NULL);

  int begin  = TimeToInt(m_Start); // seconds from midnight
  int length = TimeToInt(m_Stop) - begin;
  if (length < 0)
     length += SECSINDAY;

  int DaysToCheck = IsSingleEvent() ? 61 : 7; // 61 to handle months with 31/30/31
  for (int i = -1; i <= DaysToCheck; i++) {
      time_t t0 = IncDay(t, i);
      if (DayMatches(t0)) {
         time_t a = SetTime(t0, begin);
         time_t b = a + length;
         if ((!m_FirstDay || a >= m_FirstDay) && t <= b) {
            m_StartTime = a;
            m_StopTime = b;
            break;
            }
         }
      }
  if (!m_StartTime)
     m_StartTime = m_FirstDay; // just to have something that's more than a week in the future
  else if (t > m_StartTime || t > m_FirstDay + SECSINDAY + 3600) // +3600 in case of DST change
     m_FirstDay = 0;
  return m_Active && m_StartTime <= t && t < m_StopTime; // must m_Stop *before* m_StopTime to allow adjacent timers
}

bool cRemoteTimer::DayMatches(time_t t) {
  return IsSingleEvent()
			? GetMDay(t) == m_Day
			: (m_Day & (1 << GetWDay(t))) != 0;
}

int cRemoteTimer::GetMDay(time_t t)
{
  struct tm tm_r;
  return localtime_r(&t, &tm_r)->tm_mday;
}

int cRemoteTimer::GetWDay(time_t t)
{
  struct tm tm_r;
  int weekday = localtime_r(&t, &tm_r)->tm_wday;
  return weekday == 0 ? 6 : weekday - 1; // we start with monday==0!
}

time_t cRemoteTimer::IncDay(time_t t, int Days) {
  struct tm tm_r;
  tm tm = *localtime_r(&t, &tm_r);
  tm.tm_mday += Days; // now tm_mday may be out of its valid range
  int h = tm.tm_hour; // save original hour to compensate for DST change
  tm.tm_isdst = -1;   // makes sure mktime() will determine the correct DST setting
  t = mktime(&tm);    // normalize all values
  tm.tm_hour = h;     // compensate for DST change
  return mktime(&tm); // calculate final result
}

const char *cRemoteTimer::ToText(void) {
	char *summary = NULL;

	if (m_Buffer != NULL) free(m_Buffer);

	strreplace(m_File, ':', '|');
	if (m_Summary != "")
		summary = strreplace(strdup(m_Summary.c_str()), ':', '|');

	asprintf(&m_Buffer, "%d:%s:%s:%04d:%04d:%d:%d:%s:%s", m_Active, 
			(const char*)Channel()->GetChannelID().ToString(), PrintDay(m_Day, m_FirstDay), 
			m_Start, m_Stop, m_Priority, m_Lifetime, m_File, summary ? summary : "");

	if (summary != NULL)
		free(summary);
	strreplace(m_File, '|', ':');
	return m_Buffer;
}

// --- cRemoteTimers ---------------------------------------------------------

bool cRemoteTimers::Load(void) {
	Clear();
	return ClientSocket.LoadTimers(*this);
}

