/*
 *  $Id: connection.c,v 1.2 2005/02/08 17:22:35 lordjaxom Exp $
 */
 
#include "server/connection.h"
#include "server/setup.h"
#include "server/suspend.h"
#include "common.h"

#include <vdr/tools.h>
#include <string.h>
#include <errno.h>

cServerConnection::cServerConnection(const char *Protocol) {
	m_RdBytes = 0;
	m_WrBytes = 0;
	m_WrOffs = 0;
	m_DeferClose = false;
	m_Protocol = Protocol;
}

cServerConnection::~cServerConnection() {
}

bool cServerConnection::CanAct(const cTBSelect &Select) {
	if (Select.CanRead(*this)) {
		int b;
		if ((b = Read(m_RdBuf + m_RdBytes, sizeof(m_RdBuf) - m_RdBytes - 1)) < 0) {
			esyslog("Streamdev: Read from client (%s) %s:%d failed: %s", m_Protocol,
					RemoteIp().c_str(), RemotePort(), strerror(errno));
			return false;
		}

		if (b == 0) {
			isyslog("Streamdev: Client (%s) %s:%d closed connection", m_Protocol,
					RemoteIp().c_str(), RemotePort());
			return false;
		}

		m_RdBytes += b;
		m_RdBuf[m_RdBytes] = '\0';
		return ParseBuffer();
	}

	if (Select.CanWrite(*this)) {
		int b;
		if ((b = Write(m_WrBuf + m_WrOffs, m_WrBytes - m_WrOffs)) < 0) {
			esyslog("Streamdev: Write to client (%s) %s:%d failed: %s", m_Protocol,
					RemoteIp().c_str(), RemotePort(), strerror(errno));
			return false;
		}

		m_WrOffs += b;
		if (m_WrOffs == m_WrBytes) {
			m_WrBytes = 0;
			m_WrOffs = 0;
		}
	}

	if (m_WrBytes == 0) {
		if (m_DeferClose)
			return false;
		Flushed();
	}
	return true;
}

bool cServerConnection::ParseBuffer(void) {
	char *ep;
	bool res;

	while ((ep = strchr(m_RdBuf, '\012')) != NULL) {
		*ep = '\0';
		if (ep > m_RdBuf && *(ep-1) == '\015')
			*(ep-1) = '\0';

		Dprintf("IN: |%s|\n", m_RdBuf);
		res = Command(m_RdBuf);
		m_RdBytes -= ++ep - m_RdBuf;
		if (m_RdBytes > 0)
			memmove(m_RdBuf, ep, m_RdBytes);
		if (res == false)
			return false;
	}
	return true;
}

bool cServerConnection::Respond(const std::string &Message) {
	if (m_WrBytes + Message.size() + 2 > sizeof(m_WrBuf)) {
		esyslog("Streamdev: Output buffer overflow (%s) for %s:%d", m_Protocol,
		        RemoteIp().c_str(), RemotePort());
		return false;
	}
	Dprintf("OUT: |%s|\n", Message.c_str());
	memcpy(m_WrBuf + m_WrBytes, Message.c_str(), Message.size());

	m_WrBytes += Message.size();
	m_WrBuf[m_WrBytes++] = '\015';
	m_WrBuf[m_WrBytes++] = '\012';
	return true;
}
	
cDevice *cServerConnection::GetDevice(const cChannel *Channel, int Priority) {
	cDevice *device = NULL;

	/*Dprintf("+ Statistics:\n");
	Dprintf("+ Current Channel: %d\n", cDevice::CurrentChannel());
	Dprintf("+ Current Device: %d\n", cDevice::ActualDevice()->CardIndex());
	Dprintf("+ Transfer Mode: %s\n", cDevice::ActualDevice() 
			== cDevice::PrimaryDevice() ? "false" : "true");
	Dprintf("+ Replaying: %s\n", cDevice::PrimaryDevice()->Replaying() ? "true"
			: "false");*/

	Dprintf(" * GetDevice(const cChannel*, int)\n");
	Dprintf(" * -------------------------------\n");

	device = cDevice::GetDevice(Channel, Priority);

	Dprintf(" * Found following device: %p (%d)\n", device, 
			device ? device->CardIndex() + 1 : 0);
	if (device == cDevice::ActualDevice())
		Dprintf(" * is actual device\n");
	if (!cSuspendCtl::IsActive() && StreamdevServerSetup.SuspendMode != smAlways)
		Dprintf(" * NOT suspended\n");
	
	if (!device || (device == cDevice::ActualDevice() 
			&& !cSuspendCtl::IsActive() 
			&& StreamdevServerSetup.SuspendMode != smAlways)) {
		// mustn't switch actual device
		// maybe a device would be free if THIS connection did turn off its streams?
		Dprintf(" * trying again...\n");
		const cChannel *current = Channels.GetByNumber(cDevice::CurrentChannel());
		isyslog("streamdev-server: Detaching current receiver");
		Detach();
		device = cDevice::GetDevice(Channel, Priority);
		Attach();
		Dprintf(" * Found following device: %p (%d)\n", device, 
				device ? device->CardIndex() + 1 : 0);
		if (device == cDevice::ActualDevice())
			Dprintf(" * is actual device\n");
		if (!cSuspendCtl::IsActive() 
				&& StreamdevServerSetup.SuspendMode != smAlways)
			Dprintf(" * NOT suspended\n");
		if (current && !TRANSPONDER(Channel, current))
			Dprintf(" * NOT same transponder\n");
		if (device && (device == cDevice::ActualDevice()
				&& !cSuspendCtl::IsActive()
				&& StreamdevServerSetup.SuspendMode != smAlways
				&& current != NULL
				&& !TRANSPONDER(Channel, current))) {
			// now we would have to switch away live tv...let's see if live tv
			// can be handled by another device
			cDevice *newdev = NULL;
			for (int i = 0; i < cDevice::NumDevices(); ++i) {
				cDevice *dev = cDevice::GetDevice(i);
				if (dev->ProvidesChannel(current, 0) && dev != device) {
					newdev = dev;
					break;
				}
			}
			Dprintf(" * Found device for live tv: %p (%d)\n", newdev,
					newdev ? newdev->CardIndex() + 1 : 0);
			if (newdev == NULL || newdev == device)
				// no suitable device to continue live TV, giving up...
				device = NULL;
			else
				newdev->SwitchChannel(current, true);
		}
	}

	return device;
}
