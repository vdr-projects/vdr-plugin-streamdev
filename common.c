/*
 *  $Id: common.c,v 1.4 2005/02/11 16:44:14 lordjaxom Exp $
 */
 
#include <vdr/channels.h>
#include <iostream>

#include "common.h"
#include "tools/select.h"
#include "i18n.h"

using namespace std;

const char *VERSION = "0.3.3-pre3-geni";

const char *StreamTypes[st_Count] = {
	"TS",
	"PES",
	"PS",
	"ES",
	"Extern",
	"", // used internally only
};

const char *SuspendModes[sm_Count] = {
	"Offer suspend mode",
	"Always suspended",
	"Never suspended"
};

const char IpCharacters[] = "0123456789.";

char *GetNextLine(char *String, uint Length, uint &Offset) {
	char *last, *first;

	first = String + Offset;
	for (last = first; last < String + Length; ++last) {
		if (*last == '\012') {
			if (*(last - 1) == '\015')
				*(last - 1) = '\0';

			*last++ = '\0';
			Dprintf("IN: |%s|\n", first);
			Offset = last - String;
			return first;
		}
	}
	return NULL;
}

const cChannel *ChannelFromString(const char *String, int *Apid) {
	const cChannel *channel = NULL;
	char *string = strdup(String);
	char *ptr, *end;
	int apididx = 0;
	
	if ((ptr = strrchr(string, '+')) != NULL) {
		*(ptr++) = '\0';
		apididx = strtoul(ptr, &end, 10);
		Dprintf("found apididx: %d\n", apididx);
	}

	if (isnumber(string)) {
		int temp = strtol(String, NULL, 10);
		if (temp >= 1 && temp <= Channels.MaxNumber())
			channel = Channels.GetByNumber(temp);
	} else {
		channel = Channels.GetByChannelID(tChannelID::FromString(string));

		if (channel == NULL) {
			int i = 1;
			while ((channel = Channels.GetByNumber(i, 1)) != NULL) {
				if (String == channel->Name())
					break;

				i = channel->Number() + 1;
			}
		}
	}

	if (channel != NULL && apididx > 0) {
		int apid = 0, index = 1;

		for (int i = 0; channel->Apid(i) != 0; ++i, ++index) {
			if (index == apididx) {
				apid = channel->Apid(i);
				break;
			}
		}

		if (apid == 0) {
			for (int i = 0; channel->Dpid(i) != 0; ++i, ++index) {
				if (index == apididx) {
					apid = channel->Dpid(i);
					break;
				}
			}
		}

		if (Apid != NULL) 
			*Apid = apid;
	}

	free(string);
	return channel;
}

void cStreamdevMenuSetupPage::AddCategory(const char *Title) {
  char *buffer = NULL;

  asprintf(&buffer, "--- %s -------------------------------------------------"
   		"---------------", Title );

  cOsdItem *item = new cOsdItem(buffer);
  free(buffer);

#if VDRVERSNUM < 10307
#	ifdef HAVE_BEAUTYPATCH
  item->SetColor(clrScrolLine, clrBackground);
#	else
  item->SetColor(clrCyan, clrBackground);
#	endif
#else
	item->SetSelectable(false);
#endif
  Add(item);
}
	
void cStreamdevMenuSetupPage::AddBoolEdit(const char *Title, int &Value) {
	Add(new cMenuEditBoolItem(Title, &Value));
}

void cStreamdevMenuSetupPage::AddIpEdit(const char *Title, char *Value) {
	Add(new cMenuEditIpItem(Title, Value));
}

void cStreamdevMenuSetupPage::AddShortEdit(const char *Title, int &Value) {
	AddRangeEdit(Title, Value, 0, 65535);
}

void cStreamdevMenuSetupPage::AddRangeEdit(const char *Title, int &Value, 
		int Min, int Max) {
	Add(new cMenuEditIntItem(Title, &Value, Min, Max));
}

void cStreamdevMenuSetupPage::AddSuspEdit(const char *Title, int &Value) {
	static const char *SuspendModesTR[sm_Count] = { NULL };

	if (SuspendModesTR[0] == NULL) {
		for (int i = 0; i < sm_Count; ++i)
			SuspendModesTR[i] = tr(SuspendModes[i]);
	}

	Add(new cMenuEditStraItem(Title, &Value, sm_Count, SuspendModesTR));
}
void cStreamdevMenuSetupPage::AddTypeEdit(const char *Title, int &Value) {
	Add(new cMenuEditStraItem(Title, &Value, st_CountSetup, StreamTypes));
}

cMenuEditIpItem::cMenuEditIpItem(const char *Name, char *Value):
		cMenuEditItem(Name) {
	value = Value;
	curNum = -1;
	pos = -1;
	step = false;
	Set();
}

cMenuEditIpItem::~cMenuEditIpItem() {
}

void cMenuEditIpItem::Set(void) {
	char buf[1000];
	if (pos >= 0) {
		in_addr_t addr = inet_addr(value);
		if ((int)addr == -1)
			addr = 0;
		int p = 0;
		for (int i = 0; i < 4; ++i) {
			p += snprintf(buf + p, sizeof(buf) - p, pos == i ? "[%d]" : "%d", 
					pos == i ? curNum : (addr >> (i * 8)) & 0xff);
			if (i < 3)
				buf[p++] = '.';
		}
		SetValue(buf);
	} else
		SetValue(value);
}

eOSState cMenuEditIpItem::ProcessKey(eKeys Key) {
	in_addr addr;
	addr.s_addr = inet_addr(value);
	if ((int)addr.s_addr == -1)
		addr.s_addr = 0;

	switch (Key) {
	case kUp:
		if (pos >= 0) {
			if (curNum < 255) ++curNum;
		} else
			return cMenuEditItem::ProcessKey(Key);
		break;

	case kDown:
		if (pos >= 0) {
			if (curNum > 0) --curNum;
		} else
			return cMenuEditItem::ProcessKey(Key);
		break;

	case kOk:
		if (pos >= 0) {
			addr.s_addr = inet_addr(value);
			if ((int)addr.s_addr == -1)
				addr.s_addr = 0;
			addr.s_addr &= ~(0xff << (pos * 8));
			addr.s_addr |= curNum << (pos * 8);
			strcpy(value, inet_ntoa(addr));
		} else
			return cMenuEditItem::ProcessKey(Key);
		curNum = -1;
		pos = -1;
		break;
		
	case kRight:
		if (pos >= 0) {
			addr.s_addr = inet_addr(value);
			if ((int)addr.s_addr == -1)
				addr.s_addr = 0;
			addr.s_addr &= ~(0xff << (pos * 8));
			addr.s_addr |= curNum << (pos * 8);
			strcpy(value, inet_ntoa(addr));
		}

		if (pos == -1 || pos == 3)
			pos = 0;
		else
			++pos;

		curNum = (addr.s_addr >> (pos * 8)) & 0xff;
		step = true;
		break;

	case kLeft:
		if (pos >= 0) {
			addr.s_addr = inet_addr(value);
			if ((int)addr.s_addr == -1)
				addr.s_addr = 0;
			addr.s_addr &= ~(0xff << (pos * 8));
			addr.s_addr |= curNum << (pos * 8);
			strcpy(value, inet_ntoa(addr));
		}

		if (pos <= 0)
			pos = 3;
		else
			--pos;

		curNum = (addr.s_addr >> (pos * 8)) & 0xff;
		step = true;
		break;

	case k0 ... k9:
		if (pos == -1)
			pos = 0;

		if (curNum == -1 || step) {
			curNum = Key - k0;
			step = false;
		} else
			curNum = curNum * 10 + (Key - k0);

		if (!step && (curNum * 10 > 255) || (curNum == 0)) {
			in_addr addr;
			addr.s_addr = inet_addr(value);
			if ((int)addr.s_addr == -1)
				addr.s_addr = 0;
			addr.s_addr &= ~(0xff << (pos * 8));
			addr.s_addr |= curNum << (pos * 8);
			strcpy(value, inet_ntoa(addr));
			if (++pos == 4)
				pos = 0;
			curNum = (addr.s_addr >> (pos * 8)) & 0xff;
			step = true;
		}
		break;

	default:
		return cMenuEditItem::ProcessKey(Key);
	}

	Set();
	return osContinue;
}

