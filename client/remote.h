/*
 *  $Id: remote.h,v 1.2 2005/02/08 17:22:35 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_REMOTE_H
#define VDR_STREAMDEV_REMOTE_H

#include <vdr/config.h>
#include <string>

#if VDRVERSNUM < 10300
class cEventInfo;
#else
class cEvent;
#endif
class cChannel;

class cRemoteRecording: public cListObject {
private:
	bool        m_IsValid;
	int         m_Index;
	bool        m_IsNew;
	char       *m_TitleBuffer;
	std::string m_StartTime;
	std::string m_Name;
	std::string m_Summary;

public:
	cRemoteRecording(const char *Text);
	~cRemoteRecording();
	
	bool operator==(const cRemoteRecording &Recording);
	bool operator!=(const cRemoteRecording &Recording);

	void ParseInfo(const char *Text);

	bool IsValid(void) const { return m_IsValid; }
	int Index(void) const { return m_Index; }
	const char *StartTime(void) const { return m_StartTime.c_str(); }
	bool IsNew(void) const { return m_IsNew; }
	const char *Name(void) const { return m_Name.c_str(); }
	const char *Summary(void) const { return m_Summary.c_str(); }
	const char *Title(char Delimiter, bool NewIndicator, int Level);
	int HierarchyLevels(void);
};

inline bool cRemoteRecording::operator!=(const cRemoteRecording &Recording) { 
	return !operator==(Recording); 
}

class cRemoteRecordings: public cList<cRemoteRecording> {
public:
	bool Load(void);
	cRemoteRecording *GetByName(const char *Name);
};

class cRemoteTimer: public cListObject {
	friend class cStreamdevMenuEditTimer;

private:
	bool            m_IsValid;
	int             m_Index;
	int             m_Active;
	int             m_Day;
	int             m_Start;
	int             m_Stop;
	time_t          m_StartTime;
	time_t          m_StopTime;
	int             m_Priority;
	int             m_Lifetime;
	char            m_File[MaxFileName];
	time_t          m_FirstDay;
	std::string     m_Summary;
	char           *m_Buffer;
	const cChannel *m_Channel;

public:
	cRemoteTimer(const char *Text);
#if VDRVERSNUM < 10300
	cRemoteTimer(const cEventInfo *EventInfo);
#else
	cRemoteTimer(const cEvent *Event);
#endif
	cRemoteTimer(void);
	~cRemoteTimer();

	cRemoteTimer &operator=(const cRemoteTimer &Timer);
	bool operator==(const cRemoteTimer &Timer);
	bool operator!=(const cRemoteTimer &Timer) { return !operator==(Timer); }
	
	static int ParseDay(const char *s, time_t *FirstDay);
	static const char *PrintDay(int d, time_t FirstDay = 0);
	static time_t SetTime(time_t t, int SecondsFromMidnight);
	static time_t IncDay(time_t t, int Days);
	static int TimeToInt(int t) { return (t / 100 * 60 + t % 100) * 60; }

	const char *PrintFirstDay(void) const;
	void OnOff(void);
	bool IsSingleEvent(void) const { return (m_Day & 0x80000000) == 0; }
	void Skip(void) { m_FirstDay = IncDay(SetTime(StartTime(), 0), 1); }
	bool Matches(time_t t = 0);
	bool DayMatches(time_t t = 0);
	int GetMDay(time_t t);
	int GetWDay(time_t t);

	bool IsValid(void) const { return m_IsValid; }
	int Index(void) const { return m_Index; }
	int Active(void) const { return m_Active; }
	int Day(void) const { return m_Day; }
	int Start(void) const { return m_Start; }
	int Stop(void) const { return m_Stop; }
	time_t StartTime(void) { if (!m_StartTime) Matches(); return m_StartTime; }
	time_t StopTime(void) { if (!m_StopTime) Matches(); return m_StopTime; }
	int Priority(void) const { return m_Priority; }
	int Lifetime(void) const { return m_Lifetime; }
	const char *File(void) const { return m_File; }
	time_t FirstDay(void) const { return m_FirstDay; }
	const std::string &Summary(void) const { return m_Summary; }
	const cChannel *Channel(void) const { return m_Channel; }

	const char *ToText(void);
};

class cRemoteTimers: public cList<cRemoteTimer> {
public:
	bool Load(void);
};

extern cRemoteTimers RemoteTimers;

#endif // VDR_STREAMDEV_REMOTE_H
