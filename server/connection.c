/*
 *  $Id: connection.c,v 1.16 2010/08/03 10:51:53 schmirl Exp $
 */
 
#include "server/connection.h"
#include "server/setup.h"
#include "server/suspend.h"
#include "common.h"

#include <vdr/tools.h>
#include <vdr/thread.h>
#include <vdr/transfer.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

class cSwitchLive {
private:
	cMutex         mutex;
	cCondWait      switched;
	cDevice        *device;
	const cChannel *channel;
public:
	cDevice* Switch(cDevice *Device, const cChannel *Channel);
	void Switch(void);
	cSwitchLive(void);
};

cSwitchLive::cSwitchLive(): device(NULL), channel(NULL)
{
}

cDevice* cSwitchLive::Switch(cDevice *Device, const cChannel *Channel)
{
	mutex.Lock();
	device = Device;
	channel = Channel;
	mutex.Unlock();
	switched.Wait();
	return device;
}

void cSwitchLive::Switch(void)
{
	mutex.Lock();
	if (channel && device) {
#if APIVERSNUM >= 10726
		cChannel *current = Channels.GetByNumber(cDevice::CurrentChannel());
		cDevice *newdev = cServerConnection::CheckDevice(current, 0, true, device);
		if (!newdev) { 
			if (StreamdevServerSetup.SuspendMode == smAlways) {
				Channels.SwitchTo(channel->Number());
				Skins.Message(mtInfo, tr("Streaming active"));
			}
			else {
				esyslog("streamdev: Can't receive channel %d (%s) from device %d. Moving live TV to other device failed (PrimaryDevice=%d, ActualDevice=%d)", channel->Number(), channel->Name(), device->CardIndex(), cDevice::PrimaryDevice()->CardIndex(), cDevice::ActualDevice()->CardIndex());
				device = NULL;
			}
		}
		else {
			newdev->SwitchChannel(current, true);
		}
#else
		cDevice::SetAvoidDevice(device);
		if (!Channels.SwitchTo(cDevice::CurrentChannel())) {
			if (StreamdevServerSetup.SuspendMode == smAlways) {
				Channels.SwitchTo(channel->Number());
				Skins.Message(mtInfo, tr("Streaming active"));
			}
			else {
				esyslog("streamdev: Can't receive channel %d (%s) from device %d. Moving live TV to other device failed (PrimaryDevice=%d, ActualDevice=%d)", channel->Number(), channel->Name(), device->CardIndex(), cDevice::PrimaryDevice()->CardIndex(), cDevice::ActualDevice()->CardIndex());
				device = NULL;
			}
		}
#endif
		// make sure we don't come in here next time
		channel = NULL;
		switched.Signal();
	}
	mutex.Unlock();
}

cServerConnection::cServerConnection(const char *Protocol, int Type):
		cTBSocket(Type),
		m_Protocol(Protocol),
		m_DeferClose(false),
		m_Pending(false),
		m_ReadBytes(0),
		m_WriteBytes(0),
		m_WriteIndex(0)
{
	m_SwitchLive = new cSwitchLive();
}

cServerConnection::~cServerConnection() 
{
	delete m_SwitchLive;
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

bool cServerConnection::Close()
{
	if (IsOpen())
		isyslog("streamdev-server: closing %s connection to %s:%d", Protocol(), RemoteIp().c_str(), RemotePort());
	return cTBSocket::Close();
}

#if APIVERSNUM >= 10700
static int GetClippedNumProvidedSystems(int AvailableBits, cDevice *Device)
{
  int MaxNumProvidedSystems = (1 << AvailableBits) - 1;
  int NumProvidedSystems = Device->NumProvidedSystems();
  if (NumProvidedSystems > MaxNumProvidedSystems) {
     esyslog("ERROR: device %d supports %d modulation systems but cDevice::GetDevice() currently only supports %d delivery systems which should be fixed", Device->CardIndex() + 1, NumProvidedSystems, MaxNumProvidedSystems);
     NumProvidedSystems = MaxNumProvidedSystems;
     }
  else if (NumProvidedSystems <= 0) {
     esyslog("ERROR: device %d reported an invalid number (%d) of supported delivery systems - assuming 1", Device->CardIndex() + 1, NumProvidedSystems);
     NumProvidedSystems = 1;
     }
  return NumProvidedSystems;
}
#endif

/*
 * copy of cDevice::GetDevice(...) but without side effects (not detaching receivers)
 */
cDevice* cServerConnection::CheckDevice(const cChannel *Channel, int Priority, bool LiveView, const cDevice *AvoidDevice)
{
  //cDevice *AvoidDevice = avoidDevice;
  //avoidDevice = NULL;
  // Collect the current priorities of all CAM slots that can decrypt the channel:
  int NumCamSlots = CamSlots.Count();
  int SlotPriority[NumCamSlots];
  int NumUsableSlots = 0;
  if (Channel->Ca() >= CA_ENCRYPTED_MIN) {
     for (cCamSlot *CamSlot = CamSlots.First(); CamSlot; CamSlot = CamSlots.Next(CamSlot)) {
         SlotPriority[CamSlot->Index()] = MAXPRIORITY + 1; // assumes it can't be used
         if (CamSlot->ModuleStatus() == msReady) {
            if (CamSlot->ProvidesCa(Channel->Caids())) {
               if (!ChannelCamRelations.CamChecked(Channel->GetChannelID(), CamSlot->SlotNumber())) {
                  SlotPriority[CamSlot->Index()] = CamSlot->Priority();
                  NumUsableSlots++;
                  }
               }
            }
         }
     if (!NumUsableSlots)
        return NULL; // no CAM is able to decrypt this channel
     }

  cDevice *d = NULL;
  //cCamSlot *s = NULL;

  uint32_t Impact = 0xFFFFFFFF; // we're looking for a device with the least impact
  for (int j = 0; j < NumCamSlots || !NumUsableSlots; j++) {
      if (NumUsableSlots && SlotPriority[j] > MAXPRIORITY)
         continue; // there is no CAM available in this slot
      for (int i = 0; i < cDevice::NumDevices(); i++) {
          cDevice *device = cDevice::GetDevice(i);
          if (device == AvoidDevice)
             continue; // we've been asked to skip this device
          if (Channel->Ca() && Channel->Ca() <= CA_DVB_MAX && Channel->Ca() != device->CardIndex() + 1)
             continue; // a specific card was requested, but not this one
          if (NumUsableSlots && !CamSlots.Get(j)->Assign(device, true))
             continue; // CAM slot can't be used with this device
          bool ndr;
          if (device->ProvidesChannel(Channel, Priority, &ndr)) { // this device is basicly able to do the job
             if (NumUsableSlots && device->CamSlot() && device->CamSlot() != CamSlots.Get(j))
                ndr = true; // using a different CAM slot requires detaching receivers
             // Put together an integer number that reflects the "impact" using
             // this device would have on the overall system. Each condition is represented
             // by one bit in the number (or several bits, if the condition is actually
             // a numeric value). The sequence in which the conditions are listed corresponds
             // to their individual severity, where the one listed first will make the most
             // difference, because it results in the most significant bit of the result.
             uint32_t imp = 0;
             imp <<= 1; imp |= LiveView ? !device->IsPrimaryDevice() || ndr : 0;                                  // prefer the primary device for live viewing if we don't need to detach existing receivers
             imp <<= 1; imp |= !device->Receiving() && (device != cTransferControl::ReceiverDevice() || device->IsPrimaryDevice()) || ndr; // use receiving devices if we don't need to detach existing receivers, but avoid primary device in local transfer mode
             imp <<= 1; imp |= device->Receiving();                                                               // avoid devices that are receiving
#if APIVERSNUM >= 10700
             imp <<= 4; imp |= GetClippedNumProvidedSystems(4, device) - 1;                                       // avoid cards which support multiple delivery systems
#endif
             imp <<= 1; imp |= device == cTransferControl::ReceiverDevice();                                      // avoid the Transfer Mode receiver device
             imp <<= 8; imp |= min(max(device->Priority() + MAXPRIORITY, 0), 0xFF);                               // use the device with the lowest priority (+MAXPRIORITY to assure that values -99..99 can be used)
             imp <<= 8; imp |= min(max((NumUsableSlots ? SlotPriority[j] : 0) + MAXPRIORITY, 0), 0xFF);              // use the CAM slot with the lowest priority (+MAXPRIORITY to assure that values -99..99 can be used)
             imp <<= 1; imp |= ndr;                                                                                  // avoid devices if we need to detach existing receivers
#if VDRVERSNUM < 10719
             imp <<= 1; imp |= device->IsPrimaryDevice();                                                         // avoid the primary device
#endif
             imp <<= 1; imp |= NumUsableSlots ? 0 : device->HasCi();                                              // avoid cards with Common Interface for FTA channels
#if VDRVERSNUM < 10719
             imp <<= 1; imp |= device->HasDecoder();                                                              // avoid full featured cards
#else
             imp <<= 1; imp |= device->AvoidRecording();                                                          // avoid SD full featured cards
#endif
             imp <<= 1; imp |= NumUsableSlots ? !ChannelCamRelations.CamDecrypt(Channel->GetChannelID(), j + 1) : 0; // prefer CAMs that are known to decrypt this channel
#if VDRVERSNUM >= 10719
             imp <<= 1; imp |= device->IsPrimaryDevice();                                                         // avoid the primary device
#endif
             if (imp < Impact) {
                // This device has less impact than any previous one, so we take it.
                Impact = imp;
                d = device;
                }
             }
          }
      if (!NumUsableSlots)
         break; // no CAM necessary, so just one loop over the devices
      }
  return d;
}

bool cServerConnection::UsedByLiveTV(cDevice *device)
{
	return device == cTransferControl::ReceiverDevice() ||
		(device->IsPrimaryDevice() && device->HasDecoder() && !device->Replaying());
}

cDevice *cServerConnection::GetDevice(const cChannel *Channel, int Priority) 
{
	// turn off the streams of this connection
	Detach();
	// This call may detach receivers of the device it returns
	cDevice *device = cDevice::GetDevice(Channel, Priority, false);

 	if (device && !device->IsTunedToTransponder(Channel)
			&& UsedByLiveTV(device)) {
		// now we would have to switch away live tv...let's see if live tv
		// can be handled by another device
		device = m_SwitchLive->Switch(device, Channel);
	}

	if (!device) {
		// can't switch - continue the current stream
		Attach();
		dsyslog("streamdev: GetDevice failed for channel %d (%s) at priority %d (PrimaryDevice=%d, ActualDevice=%d)", Channel->Number(), Channel->Name(), Priority, cDevice::PrimaryDevice()->CardIndex(), cDevice::ActualDevice()->CardIndex());
	}
	return device;
}

bool cServerConnection::ProvidesChannel(const cChannel *Channel, int Priority) 
{
	cDevice *device = CheckDevice(Channel, Priority, false);
	if (!device || (StreamdevServerSetup.SuspendMode != smAlways
			&& !device->IsTunedToTransponder(Channel)
			&& UsedByLiveTV(device))) {
		// no device available or the device is in use for live TV and suspend mode doesn't allow us to switch it:
		// maybe a device would be free if THIS connection did turn off its streams?
		Detach();
		device = CheckDevice(Channel, Priority, false);
		Attach();
		if (device && StreamdevServerSetup.SuspendMode != smAlways
				&& !device->IsTunedToTransponder(Channel)
				&& UsedByLiveTV(device)) {
			// now we would have to switch away live tv...let's see if live tv
			// can be handled by another device
			const cChannel *current = Channels.GetByNumber(cDevice::CurrentChannel());
			cDevice *newdev = current ? CheckDevice(current, 0, true, device) : NULL;
			if (newdev) {
				dsyslog("streamdev: Providing channel %d (%s) at priority %d requires moving live TV to device %d (PrimaryDevice=%d, ActualDevice=%d)", Channel->Number(), Channel->Name(), Priority, newdev->CardIndex(), cDevice::PrimaryDevice()->CardIndex(), cDevice::ActualDevice()->CardIndex());
			}
			else {
				device = NULL;
				dsyslog("streamdev: Not providing channel %d (%s) at priority %d - live TV not suspended", Channel->Number(), Channel->Name(), Priority);
			}
		}
		else if (!device)
			dsyslog("streamdev: No device provides channel %d (%s) at priority %d", Channel->Number(), Channel->Name(), Priority);
	}
	return device;
}

void cServerConnection::MainThreadHook()
{
	m_SwitchLive->Switch();
}

cString cServerConnection::ToText() const
{	return cString::sprintf("%s\t%s:%d", Protocol(), RemoteIp().c_str(), RemotePort()); }
