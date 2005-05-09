/*
 *  $Id: common.h,v 1.6 2005/05/09 20:22:29 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_COMMON_H
#define VDR_STREAMDEV_COMMON_H

#include <vdr/tools.h>
#include <vdr/plugin.h>

#include "tools/socket.h"

#ifdef DEBUG
#	include <stdio.h>
#	define Dprintf(x...) fprintf(stderr, x)
#else
#	define Dprintf(x...)
#endif

#if VDRVERSNUM < 10300
#	define TRANSPONDER(c1, c2) (ISTRANSPONDER(c1->Frequency(), c2->Frequency()))
#else
#	define TRANSPONDER(c1, c2) (c1->Transponder() == c2->Transponder())
#endif

#if VDRVERSNUM < 10307
#	define INFO(s) Interface->Info(s)
#	define STATUS(s) Interface->Status(s)
#	define ERROR(s) Interface->Status(s)
#	define FLUSH() Interface->Flush()
#else
#	define INFO(s) Skins.Message(mtInfo, s)
#	define STATUS(s) Skins.Message(mtInfo, s)
#	define ERROR(s) Skins.Message(mtStatus, s)
#	define FLUSH() Skins.Flush()
#endif

/* Check if a channel is a radio station. */
#define ISRADIO(x) ((x)->Vpid()==0||(x)->Vpid()==1||(x)->Vpid()==0x1fff)

class cChannel;

char *GetNextLine(char *String, uint Length, uint &Offset);

const cChannel *ChannelFromString(const char *String, int *Apid = NULL);

/* Disable logging if BUFCOUNT buffer overflows occur within BUFOVERTIME
   milliseconds. Enable logging again if there is no error within BUFOVERTIME
   milliseconds. */
#define BUFOVERTIME  5000
#define BUFOVERCOUNT 100

#define POLLFAIL esyslog("Streamdev: Polling failed: %s", strerror(errno))
#define WRITEFAIL esyslog("Streamdev: Writing failed: %s", strerror(errno))
#define READFAIL esyslog("Streamdev: Reading failed: %s", strerror(errno))
#define CHECKPOLL(x) if ((x)<0){POLLFAIL; return false;}
#define CHECKWRITE(x) if ((x)<0) { WRITEFAIL; return false; }
#define CHECKREAD(x) if ((x)<0) { READFAIL; return false; }

enum eStreamType {
	stTS,
	stPES,
	stPS,
	stES,
	stExtern,
	stTSPIDS,

#define st_CountSetup (stExtern+1)
#define st_Count (stTSPIDS+1)
};

enum eSuspendMode {
	smOffer,
	smAlways,
	smNever,
	sm_Count
};
	
enum eSocketId {
	siLive,
	siReplay,
	si_Count
};

extern const char *VERSION;
extern const char *StreamTypes[st_Count];
extern const char *SuspendModes[sm_Count];
extern const char  IpCharacters[];

class cStreamdevMenuSetupPage: public cMenuSetupPage {
protected:
	void AddCategory(const char *Title);
	virtual void Store(void) = 0;

	void AddBoolEdit(const char *Title, int &Value);
	void AddIpEdit(const char *Title, char *Value);
	void AddShortEdit(const char *Title, int &Value);
	void AddRangeEdit(const char *Title, int &Value, int Min, int Max);
	void AddSuspEdit(const char *Title, int &Value);
	void AddTypeEdit(const char *Title, int &Value);
};

class cMenuEditIpItem: public cMenuEditItem {
private:
	char *value;
	int curNum;
	int pos;
	bool step;

protected:
	virtual void Set(void);

public:
	cMenuEditIpItem(const char *Name, char *Value); // Value must be 16 bytes
	~cMenuEditIpItem();

	virtual eOSState ProcessKey(eKeys Key);
};

#endif // VDR_STREAMDEV_COMMON_H
