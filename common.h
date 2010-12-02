/*
 *  $Id: common.h,v 1.15.2.1 2010/06/11 06:06:01 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_COMMON_H
#define VDR_STREAMDEV_COMMON_H

/* FreeBSD has it's own version of isnumber(),
   but VDR's version is incompatible */
#ifdef __FreeBSD__
#undef isnumber
#endif

#include <vdr/tools.h>
#include <vdr/plugin.h>

#include "tools/socket.h"

#ifdef DEBUG
#	include <stdio.h>
#	define Dprintf(x...) fprintf(stderr, x)
#else
#	define Dprintf(x...)
#endif

#define TRANSPONDER(c1, c2) (c1->Transponder() == c2->Transponder())

#define MAXPARSEBUFFER KILOBYTE(16)

/* Check if a channel is a radio station. */
#define ISRADIO(x) ((x)->Vpid()==0||(x)->Vpid()==1||(x)->Vpid()==0x1fff)

class cChannel;

enum eStreamType {
	stTS,
	stPES,
	stPS,
	stES,
	stEXT,
	stTSPIDS,
	st_Count
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
	siLiveFilter,
	siDataRespond,
	si_Count
};

extern const char *VERSION;

class cMenuEditIpItem: public cMenuEditItem {
private:
	static const char IpCharacters[];
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
