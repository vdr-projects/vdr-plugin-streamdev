/*
 *  $Id: connection.c,v 1.13.2.1 2010/06/11 06:06:02 schmirl Exp $
 */
 
#include "server/connection.h"
#include "server/setup.h"
#include "server/suspend.h"
#include "common.h"

#include <vdr/tools.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

cServerConnection::cServerConnection(const char *Protocol, int Type):
		cTBSocket(Type),
		m_Protocol(Protocol),
		m_DeferClose(false),
		m_Pending(false),
		m_ReadBytes(0),
		m_WriteBytes(0),
		m_WriteIndex(0)
{
}

cServerConnection::~cServerConnection() 
{
}

const cChannel* cServerConnection::ChannelFromString(const char *String, int *Apid, int *Dpid) {
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
		int apid = 0, dpid = 0;
		int index = 1;

		for (int i = 0; channel->Apid(i) != 0; ++i, ++index) {
			if (index == apididx) {
				apid = channel->Apid(i);
				break;
			}
		}

		if (apid == 0) {
			for (int i = 0; channel->Dpid(i) != 0; ++i, ++index) {
				if (index == apididx) {
					dpid = channel->Dpid(i);
					break;
				}
			}
		}

		if (Apid != NULL) 
			*Apid = apid;
		if (Dpid != NULL) 
			*Dpid = dpid;
	}

	free(string);
	return channel;
}

bool cServerConnection::Read(void) 
{
	int b;
	if ((b = cTBSocket::Read(m_ReadBuffer + m_ReadBytes,
	                         sizeof(m_ReadBuffer) - m_ReadBytes - 1)) < 0) {
		esyslog("ERROR: read from client (%s) %s:%d failed: %m",
		        m_Protocol, RemoteIp().c_str(), RemotePort());
		return false;
	}

	if (b == 0) {
		isyslog("client (%s) %s:%d has closed connection",
		        m_Protocol, RemoteIp().c_str(), RemotePort());
		return false;
	}

	m_ReadBytes += b;
	m_ReadBuffer[m_ReadBytes] = '\0';

	char *end;
	bool result = true;
	while ((end = strchr(m_ReadBuffer, '\012')) != NULL) {
		*end = '\0';
		if (end > m_ReadBuffer && *(end - 1) == '\015')
			*(end - 1) = '\0';

		if (!Command(m_ReadBuffer))
			return false;

		m_ReadBytes -= ++end - m_ReadBuffer;
		if (m_ReadBytes > 0)
			memmove(m_ReadBuffer, end, m_ReadBytes);
	}

	if (m_ReadBytes == sizeof(m_ReadBuffer) - 1) {
		esyslog("ERROR: streamdev: input buffer overflow (%s) for %s:%d",
		        m_Protocol, RemoteIp().c_str(), RemotePort());
		return false;
	}
	
	return result;
}

bool cServerConnection::Write(void) 
{
	int b;
	if ((b = cTBSocket::Write(m_WriteBuffer + m_WriteIndex, 
	                          m_WriteBytes - m_WriteIndex)) < 0) {
		esyslog("ERROR: streamdev: write to client (%s) %s:%d failed: %m",
		        m_Protocol, RemoteIp().c_str(), RemotePort());
		return false;
	}

	m_WriteIndex += b;
	if (m_WriteIndex == m_WriteBytes) {
		m_WriteIndex = 0;
		m_WriteBytes = 0;
		if (m_Pending)
			Command(NULL);
		if (m_DeferClose)
			return false;
		Flushed();
	}
	return true;
}

bool cServerConnection::Respond(const char *Message, bool Last, ...) 
{
	char *buffer;
	int length;
	va_list ap;
	va_start(ap, Last);
	length = vasprintf(&buffer, Message, ap);
	va_end(ap);

	if (length < 0) {
		esyslog("ERROR: streamdev: buffer allocation failed (%s) for %s:%d",
				m_Protocol, RemoteIp().c_str(), RemotePort());
		return false;
	}

	if (m_WriteBytes + length + 2 > sizeof(m_WriteBuffer)) {
		esyslog("ERROR: streamdev: output buffer overflow (%s) for %s:%d", 
		        m_Protocol, RemoteIp().c_str(), RemotePort());
		free(buffer);
		return false;
	}
	Dprintf("OUT: |%s|\n", buffer);
	memcpy(m_WriteBuffer + m_WriteBytes, buffer, length);
	free(buffer);

	m_WriteBytes += length;
	m_WriteBuffer[m_WriteBytes++] = '\015';
	m_WriteBuffer[m_WriteBytes++] = '\012';
	m_Pending = !Last;
	return true;
}
	
cDevice *cServerConnection::GetDevice(const cChannel *Channel, int Priority) 
{
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

	device = cDevice::GetDevice(Channel, Priority, false);

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
		device = cDevice::GetDevice(Channel, Priority, false);
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
