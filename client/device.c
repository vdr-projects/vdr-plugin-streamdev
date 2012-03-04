/*
 *  $Id: device.c,v 1.27 2010/08/18 10:26:55 schmirl Exp $
 */
 
#include "client/device.h"
#include "client/setup.h"
#include "client/filter.h"

#include "tools/select.h"

#include <vdr/config.h>
#include <vdr/channels.h>
#include <vdr/ringbuffer.h>
#include <vdr/eit.h>
#include <vdr/timers.h>

#include <time.h>
#include <iostream>

using namespace std;

#define VIDEOBUFSIZE MEGABYTE(3)

cStreamdevDevice *cStreamdevDevice::m_Device = NULL;
const cChannel *cStreamdevDevice::m_DenyChannel = NULL;

cStreamdevDevice::cStreamdevDevice(void) {
	m_Channel    = NULL;
	m_TSBuffer   = NULL;

	m_Filters    = new cStreamdevFilters;
	StartSectionHandler();
	isyslog("streamdev-client: got device number %d", CardIndex() + 1);

	m_Device = this;
	m_Pids = 0;
	m_Priority = -100;
	m_DvrClosed = true;
}

cStreamdevDevice::~cStreamdevDevice() {
	Dprintf("Device gets destructed\n");

	Lock();
	m_Device = NULL;
	m_Filters->SetConnection(-1);
	ClientSocket.Quit();
	ClientSocket.Reset();
	Unlock();

	Cancel(3);

	StopSectionHandler();
	DELETENULL(m_Filters);
	DELETENULL(m_TSBuffer);
}

#if APIVERSNUM >= 10700
int cStreamdevDevice::NumProvidedSystems(void) const
{	return StreamdevClientSetup.NumProvidedSystems; }
#endif

bool cStreamdevDevice::ProvidesSource(int Source) const {
	Dprintf("ProvidesSource, Source=%d\n", Source);
	return true;
}

bool cStreamdevDevice::ProvidesTransponder(const cChannel *Channel) const
{
	Dprintf("ProvidesTransponder\n");
	return true;
}

#if APIVERSNUM >= 10722
bool cStreamdevDevice::IsTunedToTransponder(const cChannel *Channel) const
#else
bool cStreamdevDevice::IsTunedToTransponder(const cChannel *Channel)
#endif
{
	bool res = false;
	if (ClientSocket.DataSocket(siLive) != NULL
			&& TRANSPONDER(Channel, m_Channel)
			&& Channel->Ca() == CA_FTA
			&& m_Channel->Ca() == CA_FTA)
		res = true;
	return res;
}

bool cStreamdevDevice::ProvidesChannel(const cChannel *Channel, int Priority, 
		bool *NeedsDetachReceivers) const {
	bool res = false;
	bool prio = Priority < 0 || Priority > this->Priority();
	bool ndr = false;

	if (!StreamdevClientSetup.StartClient || Channel == m_DenyChannel)
		return false;

	Dprintf("ProvidesChannel, Channel=%s, Prio=%d\n", Channel->Name(), Priority);

	if (StreamdevClientSetup.MinPriority <= StreamdevClientSetup.MaxPriority)
	{
		if (Priority < StreamdevClientSetup.MinPriority ||
				Priority > StreamdevClientSetup.MaxPriority)
			return false;
	}
	else
	{
		if (Priority < StreamdevClientSetup.MinPriority &&
				Priority > StreamdevClientSetup.MaxPriority)
			return false;
	}

	if (ClientSocket.DataSocket(siLive) != NULL 
			&& TRANSPONDER(Channel, m_Channel))
		res = true;
	else {
		res = prio && ClientSocket.ProvidesChannel(Channel, Priority);
		ndr = true;
	}

	if (NeedsDetachReceivers)
		*NeedsDetachReceivers = ndr;
	Dprintf("prov res = %d, ndr = %d\n", res, ndr);
	return res;
}

bool cStreamdevDevice::SetChannelDevice(const cChannel *Channel, 
		bool LiveView) {
	Dprintf("SetChannelDevice Channel: %s, LiveView: %s\n", Channel->Name(),
			LiveView ? "true" : "false");
	LOCK_THREAD;

	m_UpdatePriority = ClientSocket.SupportsPrio();

	if (LiveView)
		return false;

	if (ClientSocket.DataSocket(siLive) != NULL 
			&& TRANSPONDER(Channel, m_Channel)
			&& Channel->Ca() == CA_FTA
			&& m_Channel->Ca() == CA_FTA)
		return true;

	DetachAllReceivers();
	m_Channel = Channel;
	bool r = ClientSocket.SetChannelDevice(m_Channel);
	Dprintf("setchanneldevice r=%d\n", r);
	return r;
}

bool cStreamdevDevice::SetPid(cPidHandle *Handle, int Type, bool On) {
	Dprintf("SetPid, Pid=%d, Type=%d, On=%d, used=%d\n", Handle->pid, Type, On,
			Handle->used);
	LOCK_THREAD;

	m_UpdatePriority = ClientSocket.SupportsPrio();

	if (On && !m_TSBuffer) {
		Dprintf("SetPid: no data connection -> OpenDvr()");
		OpenDvrInt();
	}

	bool res = true; 
	if (Handle->pid && (On || !Handle->used)) {
		res = ClientSocket.SetPid(Handle->pid, On);

		m_Pids += (!res) ? 0 : On ? 1 : -1;
		if (m_Pids < 0) 
			m_Pids = 0;

		if(m_Pids < 1 && m_DvrClosed) { 
			Dprintf("SetPid: 0 pids left -> CloseDvr()"); 
			CloseDvrInt(); 
		}
	}

	return res;
}

bool cStreamdevDevice::OpenDvrInt(void) {
	Dprintf("OpenDvrInt\n");
	LOCK_THREAD;

	CloseDvrInt();
	if (m_TSBuffer) {
		Dprintf("cStreamdevDevice::OpenDvrInt(): DVR connection already open\n");
		return true;
	}

	Dprintf("cStreamdevDevice::OpenDvrInt(): Connecting ...\n");
	if (ClientSocket.CreateDataConnection(siLive)) {
		m_TSBuffer = new cTSBuffer(*ClientSocket.DataSocket(siLive), MEGABYTE(2), CardIndex() + 1);
		return true;
	}
	esyslog("cStreamdevDevice::OpenDvrInt(): DVR connection FAILED");
	return false;
}

bool cStreamdevDevice::OpenDvr(void) {
	Dprintf("OpenDvr\n");
	LOCK_THREAD;

	m_DvrClosed = false;
	return OpenDvrInt();
}

void cStreamdevDevice::CloseDvrInt(void) {
	Dprintf("CloseDvrInt\n");
	LOCK_THREAD;

	if (ClientSocket.CheckConnection()) {
		if (!m_DvrClosed) {
			Dprintf("cStreamdevDevice::CloseDvrInt(): m_DvrClosed=false -> not closing yet\n");
			return;
		}
		if (m_Pids > 0) {
			Dprintf("cStreamdevDevice::CloseDvrInt(): %d active pids -> not closing yet\n", m_Pids);
			return;
		}
	} else {
		Dprintf("cStreamdevDevice::CloseDvrInt(): Control connection gone !\n");
	}

	Dprintf("cStreamdevDevice::CloseDvrInt(): Closing DVR connection\n");
	// Hack for VDR 1.5.x clients (sometimes sending ABRT after TUNE)
	// TODO: Find a clean solution to fix this
	ClientSocket.SetChannelDevice(m_Channel);
	ClientSocket.CloseDvr();
	DELETENULL(m_TSBuffer);
}

void cStreamdevDevice::CloseDvr(void) {
	Dprintf("CloseDvr\n");
	LOCK_THREAD;

	m_DvrClosed = true;
	CloseDvrInt();
}

bool cStreamdevDevice::GetTSPacket(uchar *&Data) {
	if (m_TSBuffer && m_Device) {
		Data = m_TSBuffer->Get();
#if 1 // TODO: this should be fixed in vdr cTSBuffer
		// simple disconnect detection
		static int m_TSFails = 0;
		if (!Data) {
			LOCK_THREAD;
			if(!ClientSocket.DataSocket(siLive)) {
				return false; // triggers CloseDvr() + OpenDvr() in cDevice
                        }
			cPoller Poller(*ClientSocket.DataSocket(siLive));
			errno = 0;
			if (Poller.Poll() && !errno) {
				char tmp[1];
				if (recv(*ClientSocket.DataSocket(siLive), tmp, 1, MSG_PEEK) == 0 && !errno) {
esyslog("cStreamDevice::GetTSPacket: GetChecked: NOTHING (%d)", m_TSFails);
					m_TSFails++; 
					if (m_TSFails > 10) {
						isyslog("cStreamdevDevice::GetTSPacket(): disconnected");
						m_Pids = 0;
						CloseDvrInt();
						m_TSFails = 0;
						return false;
					}
					return true;
				}
			}
			m_TSFails = 0;
		}
#endif
		return true;
	}
	return false;
}

int cStreamdevDevice::OpenFilter(u_short Pid, u_char Tid, u_char Mask) {
	Dprintf("OpenFilter\n");

	if (!StreamdevClientSetup.StreamFilters)
		return -1;


	if (!ClientSocket.DataSocket(siLiveFilter)) {
		if (ClientSocket.CreateDataConnection(siLiveFilter)) {
			m_Filters->SetConnection(*ClientSocket.DataSocket(siLiveFilter));
		} else {
			isyslog("cStreamdevDevice::OpenFilter: connect failed: %m");
			return -1;
		}
	}

	if (ClientSocket.SetFilter(Pid, Tid, Mask, true))
		return m_Filters->OpenFilter(Pid, Tid, Mask);

	return -1;
}

bool cStreamdevDevice::Init(void) {
	if (m_Device == NULL && StreamdevClientSetup.StartClient)
		new cStreamdevDevice;
	return true;
}

bool cStreamdevDevice::ReInit(void) {
	if(m_Device) {
		m_Device->Lock();
		m_Device->m_Filters->SetConnection(-1);
		m_Device->m_Pids = 0;
	}
	ClientSocket.Quit();
	ClientSocket.Reset();
	if (m_Device != NULL) {
		//DELETENULL(m_Device->m_TSBuffer);
		m_Device->Unlock();
	}
	return StreamdevClientSetup.StartClient ? Init() : true;
}

void cStreamdevDevice::UpdatePriority(void) {
	if (m_Device) {
		m_Device->Lock();
		if (m_Device->m_UpdatePriority && ClientSocket.DataSocket(siLive)) {
			int Priority = m_Device->Priority();
			// override TRANSFERPRIORITY (-1) with live TV priority from setup
			if (m_Device == cDevice::ActualDevice() && Priority == -1)
				Priority = StreamdevClientSetup.LivePriority;
			if (m_Device->m_Priority != Priority && ClientSocket.SetPriority(Priority))
				m_Device->m_Priority = Priority;
		}
		m_Device->Unlock();
	}
}

int cStreamdevDevice::SignalStrength(void) const {
	int strength = -1;
	if (ClientSocket.DataSocket(siLive) != NULL)
		ClientSocket.GetSignal(&strength, NULL);
	return strength;
}

int cStreamdevDevice::SignalQuality(void) const {
	int quality = -1;
	if (ClientSocket.DataSocket(siLive) != NULL)
		ClientSocket.GetSignal(NULL, &quality);
	return quality;
}

