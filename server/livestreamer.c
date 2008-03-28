#include <assert.h>

#include <libsi/section.h>
#include <libsi/descriptor.h>

#include <vdr/ringbuffer.h>

#include "server/livestreamer.h"
#include "server/livefilter.h"
#include "remux/ts2ps.h"
#include "remux/ts2es.h"
#include "remux/extern.h"
#include "common.h"

#define TSPATREPACKER

// --- cStreamdevLiveReceiver -------------------------------------------------

class cStreamdevLiveReceiver: public cReceiver {
	friend class cStreamdevStreamer;

private:
	cStreamdevStreamer *m_Streamer;

protected:
	virtual void Activate(bool On);
	virtual void Receive(uchar *Data, int Length);

public:
#if VDRVERSNUM < 10500
	cStreamdevLiveReceiver(cStreamdevStreamer *Streamer, int Ca, int Priority, const int *Pids);
#else
	cStreamdevLiveReceiver(cStreamdevStreamer *Streamer, tChannelID ChannelID, int Priority, const int *Pids);
#endif
	virtual ~cStreamdevLiveReceiver();
};

#if VDRVERSNUM < 10500
cStreamdevLiveReceiver::cStreamdevLiveReceiver(cStreamdevStreamer *Streamer, int Ca, 
                                               int Priority, const int *Pids):
		cReceiver(Ca, Priority, 0, Pids),
#else
cStreamdevLiveReceiver::cStreamdevLiveReceiver(cStreamdevStreamer *Streamer, tChannelID ChannelID, 
                                               int Priority, const int *Pids):
		cReceiver(ChannelID, Priority, 0, Pids),
#endif
		m_Streamer(Streamer)
{
}

cStreamdevLiveReceiver::~cStreamdevLiveReceiver() 
{
	Dprintf("Killing live receiver\n");
	Detach();
}

void cStreamdevLiveReceiver::Receive(uchar *Data, int Length) {
	int p = m_Streamer->Receive(Data, Length);
	if (p != Length)
		m_Streamer->ReportOverflow(Length - p);
}

inline void cStreamdevLiveReceiver::Activate(bool On) 
{ 
	Dprintf("LiveReceiver->Activate(%d)\n", On);
	m_Streamer->Activate(On); 
}

// --- cStreamdevPatFilter ----------------------------------------------------

class cStreamdevPatFilter : public cFilter {
private:
	int    pmtPid;
	int    pmtSid;
	int    pmtVersion;

	const cChannel *m_Channel;
	cStreamdevLiveStreamer *m_Streamer;

	virtual void Process(u_short Pid, u_char Tid, const u_char *Data, int Length);

	int GetPid(SI::PMT::Stream& stream);
public:
	cStreamdevPatFilter(cStreamdevLiveStreamer *Streamer, const cChannel *Channel);
};

cStreamdevPatFilter::cStreamdevPatFilter(cStreamdevLiveStreamer *Streamer, const cChannel *Channel)
{
	Dprintf("cStreamdevPatFilter(\"%s\")", Channel->Name());
	assert(Streamer);
	m_Channel = Channel;
	m_Streamer = Streamer;
	pmtPid = 0;
	pmtSid = 0;
	pmtVersion = -1;
	Set(0x00, 0x00);  // PAT
}

static const char * const psStreamTypes[] = {
	"UNKNOWN", 
	"ISO/IEC 11172 Video", 
	"ISO/IEC 13818-2 Video", 
	"ISO/IEC 11172 Audio", 
	"ISO/IEC 13818-3 Audio",
	"ISO/IEC 13818-1 Privete sections", 
	"ISO/IEC 13818-1 Private PES data",
	"ISO/IEC 13512 MHEG", 
	"ISO/IEC 13818-1 Annex A DSM CC",
	"0x09",
	"ISO/IEC 13818-6 Multiprotocol encapsulation",
	"ISO/IEC 13818-6 DSM-CC U-N Messages",
	"ISO/IEC 13818-6 Stream Descriptors",
	"ISO/IEC 13818-6 Sections (any type, including private data)",
	"ISO/IEC 13818-1 auxiliary",
	"ISO/IEC 13818-7 Audio with ADTS transport sytax",
	"ISO/IEC 14496-2 Visual (MPEG-4)",
	"ISO/IEC 14496-3 Audio with LATM transport syntax",
	"0x12", "0x13", "0x14", "0x15", "0x16", "0x17", "0x18", "0x19", "0x1a",
	"ISO/IEC 14496-10 Video (MPEG-4 part 10/AVC, aka H.264)",
	"",
};

int cStreamdevPatFilter::GetPid(SI::PMT::Stream& stream)
{
	SI::Descriptor *d;

	if (!stream.getPid()) 
		return 0;

	switch (stream.getStreamType()) {
	case 0x01: // ISO/IEC 11172 Video
	case 0x02: // ISO/IEC 13818-2 Video
	case 0x03: // ISO/IEC 11172 Audio
	case 0x04: // ISO/IEC 13818-3 Audio
#if 0
	case 0x07: // ISO/IEC 13512 MHEG 
	case 0x08: // ISO/IEC 13818-1 Annex A  DSM CC
	case 0x0a: // ISO/IEC 13818-6 Multiprotocol encapsulation
	case 0x0b: // ISO/IEC 13818-6 DSM-CC U-N Messages
	case 0x0c: // ISO/IEC 13818-6 Stream Descriptors
	case 0x0d: // ISO/IEC 13818-6 Sections (any type, including private data)
	case 0x0e: // ISO/IEC 13818-1 auxiliary
#endif
	case 0x0f: // ISO/IEC 13818-7 Audio with ADTS transport syntax
	case 0x10: // ISO/IEC 14496-2 Visual (MPEG-4)
	case 0x11: // ISO/IEC 14496-3 Audio with LATM transport syntax
	case 0x1b: // ISO/IEC 14496-10 Video (MPEG-4 part 10/AVC, aka H.264)
		Dprintf("cStreamdevPatFilter PMT scanner adding PID %d (%s)",
			stream.getPid(), psStreamTypes[stream.getStreamType()]);
		return stream.getPid();
	case 0x05: // ISO/IEC 13818-1 private sections
	case 0x06: // ISO/IEC 13818-1 PES packets containing private data
		for (SI::Loop::Iterator it; (d = stream.streamDescriptors.getNext(it)); ) {
			switch (d->getDescriptorTag()) {
			case SI::AC3DescriptorTag:
				Dprintf("cStreamdevPatFilter PMT scanner: adding PID %d (%s) %s", 
					stream.getPid(), psStreamTypes[stream.getStreamType()], "AC3");
				return stream.getPid();
			case SI::TeletextDescriptorTag:
				Dprintf("cStreamdevPatFilter PMT scanner: adding PID %d (%s) %s", 
					stream.getPid(), psStreamTypes[stream.getStreamType()], "Teletext");
				return stream.getPid();
			case SI::SubtitlingDescriptorTag:
				Dprintf("cStreamdevPatFilter PMT scanner: adding PID %d (%s) %s", 
					stream.getPid(), psStreamTypes[stream.getStreamType()], "DVBSUB");
				return stream.getPid();
			default:
				Dprintf("cStreamdevPatFilter PMT scanner: NOT adding PID %d (%s) %s", 
					stream.getPid(), psStreamTypes[stream.getStreamType()], "UNKNOWN");
				break;
			}
			delete d;
		}
		break;
	default:
		/* This following section handles all the cases where the audio track 
		 * info is stored in PMT user info with stream id >= 0x80
		 * we check the registration format identifier to see if it 
		 * holds "AC-3"
		 */
		if (stream.getStreamType() >= 0x80) {
			bool found = false;
			for (SI::Loop::Iterator it; (d = stream.streamDescriptors.getNext(it)); ) {
				switch (d->getDescriptorTag()) {
				case SI::RegistrationDescriptorTag:
					/* unfortunately libsi does not implement RegistrationDescriptor */
					if (d->getLength() >= 4) {
						found = true;
						SI::CharArray rawdata = d->getData();
						if (/*rawdata[0] == 5 && rawdata[1] >= 4 && */
						    rawdata[2] == 'A' && rawdata[3] == 'C' &&
						    rawdata[4] == '-' && rawdata[5] == '3') {
							isyslog("cStreamdevPatFilter PMT scanner:"
								"Adding pid %d (type 0x%x) RegDesc len %d (%c%c%c%c)", 
								stream.getPid(), stream.getStreamType(), 
								d->getLength(), rawdata[2], rawdata[3], 
								rawdata[4], rawdata[5]);
							return stream.getPid();
						}
					}
					break;
				default: 
					break;
				}
				delete d;
			}
			if(!found) {
				isyslog("Adding pid %d (type 0x%x) RegDesc not found -> assume AC-3", 
					stream.getPid(), stream.getStreamType());
				return stream.getPid();
			}
		}
		Dprintf("cStreamdevPatFilter PMT scanner: NOT adding PID %d (%s) %s",
			stream.getPid(), psStreamTypes[stream.getStreamType()<0x1c?stream.getStreamType():0], "UNKNOWN");
		break;
	}
	return 0;
}

void cStreamdevPatFilter::Process(u_short Pid, u_char Tid, const u_char *Data, int Length)
{
	if (Pid == 0x00) {
		if (Tid == 0x00 && !pmtPid) {
			SI::PAT pat(Data, false);
			if (!pat.CheckCRCAndParse())
				return;
			SI::PAT::Association assoc;
			for (SI::Loop::Iterator it; pat.associationLoop.getNext(assoc, it); ) {
				if (!assoc.isNITPid()) {
					const cChannel *Channel =  Channels.GetByServiceID(Source(), Transponder(), assoc.getServiceId());
					if (Channel && (Channel == m_Channel)) {
						if (0 != (pmtPid = assoc.getPid())) {
							Dprintf("cStreamdevPatFilter: PMT pid for channel %s: %d", Channel->Name(), pmtPid);
							pmtSid = assoc.getServiceId();
							if (Length < TS_SIZE-5) {
								// repack PAT to TS frame and send to client
#ifndef TSPATREPACKER
								uint8_t pat_ts[TS_SIZE] = {TS_SYNC_BYTE, 0x40 /* pusi=1 */, 0 /* pid=0 */, 0x10 /* adaption=1 */, 0 /* pointer */};
								memcpy(pat_ts + 5, Data, Length);
								m_Streamer->Put(pat_ts, TS_SIZE);
#else
								int ts_id;
								unsigned int crc, i, len;
								uint8_t *tmp, tspat_buf[TS_SIZE];
								memset(tspat_buf, 0xff, TS_SIZE);
								memset(tspat_buf, 0x0, 4 + 12 + 5);   // TS_HDR_LEN + PAT_TABLE_LEN + 5
								ts_id = Channel->Tid();               // Get transport stream id of the channel
								tspat_buf[0] = TS_SYNC_BYTE;          // Transport packet header sunchronization byte (1000011 = 0x47h)
								tspat_buf[1] = 0x40;                  // Set payload unit start indicator bit
								tspat_buf[2] = 0x0;                   // PID
								tspat_buf[3] = 0x10;                  // Set payload flag to indicate precence of payload data
								tspat_buf[4] = 0x0;                   // PSI
								tspat_buf[5] = 0x0;                   // PAT table id
								tspat_buf[6] = 0xb0;                  // Section syntax indicator bit and reserved bits set
								tspat_buf[7] = 12 + 1;                // Section length (12 bit): PAT_TABLE_LEN + 1
								tspat_buf[8] = (ts_id >> 8) & 0xff;   // Transport stream ID (bits 8-15)
								tspat_buf[9] = (ts_id & 0xff);        // Transport stream ID (bits 0-7)
								tspat_buf[10] = 0x01;                 // Version number 0, Current next indicator bit set  
								tspat_buf[11] = 0x0;                  // Section number
								tspat_buf[12] = 0x0;                  // Last section number
								tspat_buf[13] = (pmtSid >> 8) & 0xff; // Program number (bits 8-15)
								tspat_buf[14] = (pmtSid & 0xff);      // Program number (bits 0-7)
								tspat_buf[15] = (pmtPid >> 8) & 0xff; // Network ID (bits 8-12)
								tspat_buf[16] = (pmtPid & 0xff);      // Network ID (bits 0-7)
								crc = 0xffffffff;
								len = 12;                             // PAT_TABLE_LEN
								tmp = &tspat_buf[4 + 1];              // TS_HDR_LEN + 1
								while (len--) {
									crc ^= *tmp++ << 24;
									for (i = 0; i < 8; i++)
										crc = (crc << 1) ^ ((crc & 0x80000000) ? 0x04c11db7 : 0); // CRC32POLY
								}
								tspat_buf[17] = crc >> 24 & 0xff;     // Checksum
								tspat_buf[18] = crc >> 16 & 0xff;     // Checksum
								tspat_buf[19] = crc >>  8 & 0xff;     // Checksum
								tspat_buf[20] = crc & 0xff;           // Checksum
								m_Streamer->Put(tspat_buf, TS_SIZE);
#endif
							} else 
								isyslog("cStreamdevPatFilter: PAT size %d too large to fit in one TS", Length);
							m_Streamer->SetPids(pmtPid);
							Add(pmtPid, 0x02);
							pmtVersion = -1;
							return;
						}
					}
				}
			}
		}
	} else if (Pid == pmtPid && Tid == SI::TableIdPMT && Source() && Transponder()) {
		SI::PMT pmt(Data, false);
		if (!pmt.CheckCRCAndParse())
			return;
		if (pmt.getServiceId() != pmtSid)
			return; // skip broken PMT records
		if (pmtVersion != -1) {
			if (pmtVersion != pmt.getVersionNumber()) {
				Dprintf("cStreamdevPatFilter: PMT version changed, detaching all pids");
				Del(pmtPid, 0x02);
				pmtPid = 0; // this triggers PAT scan
			}
			return;
		}
		pmtVersion = pmt.getVersionNumber();

		SI::PMT::Stream stream;
		int pids[MAXRECEIVEPIDS + 1], npids = 0;
		pids[npids++] = pmtPid;
#if 0
		pids[npids++] = 0x10;  // pid 0x10, tid 0x40: NIT 
		pids[npids++] = 0x11;  // pid 0x11, tid 0x42: SDT 
		pids[npids++] = 0x14;  // pid 0x14, tid 0x70: TDT 
#endif
		pids[npids++] = 0x12;  // pid 0x12, tid 0x4E...0x6F: EIT 
		for (SI::Loop::Iterator it; pmt.streamLoop.getNext(stream, it); )
			if (0 != (pids[npids] = GetPid(stream)) && npids < MAXRECEIVEPIDS)
				npids++;

		pids[npids] = 0;
		m_Streamer->SetPids(pmt.getPCRPid(), pids);
	}
}

// --- cStreamdevLiveStreamer -------------------------------------------------

cStreamdevLiveStreamer::cStreamdevLiveStreamer(int Priority, std::string Parameter):
		cStreamdevStreamer("streamdev-livestreaming"),
		m_Priority(Priority),
		m_Parameter(Parameter),
		m_NumPids(0),
		m_StreamType(stTSPIDS),
		m_Channel(NULL),
		m_Device(NULL),
		m_Receiver(NULL),
		m_PatFilter(NULL),
		m_PESRemux(NULL),
		m_ESRemux(NULL),
		m_PSRemux(NULL),
		m_ExtRemux(NULL)
{
}

cStreamdevLiveStreamer::~cStreamdevLiveStreamer() 
{
	Dprintf("Desctructing Live streamer\n");
	Stop();
	if(m_PatFilter) {
		Detach();
		DELETENULL(m_PatFilter);
	}
	DELETENULL(m_Receiver);
	delete m_PESRemux;
	delete m_ESRemux;
	delete m_PSRemux;
	delete m_ExtRemux;
}

bool cStreamdevLiveStreamer::HasPid(int Pid) 
{
	int idx;
	for (idx = 0; idx < m_NumPids; ++idx)
		if (m_Pids[idx] == Pid)
			return true;
	return false;
}

bool cStreamdevLiveStreamer::SetPid(int Pid, bool On) 
{
	int idx;

	if (Pid == 0)
		return true;
	
	if (On) {
		for (idx = 0; idx < m_NumPids; ++idx) {
			if (m_Pids[idx] == Pid)
				return true; // No change needed
		}

		if (m_NumPids == MAXRECEIVEPIDS) {
			esyslog("ERROR: Streamdev: No free slot to receive pid %d\n", Pid);
			return false;
		}

		m_Pids[m_NumPids++] = Pid;
		m_Pids[m_NumPids] = 0;
	} else {
		for (idx = 0; idx < m_NumPids; ++idx) {
			if (m_Pids[idx] == Pid) {
				--m_NumPids;
				memmove(&m_Pids[idx], &m_Pids[idx + 1], sizeof(int) * (m_NumPids - idx));
			}
		}
	}

	StartReceiver();
	return true;
}

bool cStreamdevLiveStreamer::SetPids(int Pid, const int *Pids1, const int *Pids2, const int *Pids3)
{
	m_NumPids = 0;

	if (Pid)
		m_Pids[m_NumPids++] = Pid;

	if (Pids1)
		for ( ; *Pids1 && m_NumPids < MAXRECEIVEPIDS; Pids1++)
			if (!HasPid(*Pids1))
				m_Pids[m_NumPids++] = *Pids1;

	if (Pids2)
		for ( ; *Pids2 && m_NumPids < MAXRECEIVEPIDS; Pids2++)
			if (!HasPid(*Pids2))
				m_Pids[m_NumPids++] = *Pids2;

	if (Pids3)
		for ( ; *Pids3 && m_NumPids < MAXRECEIVEPIDS; Pids3++)
			if (!HasPid(*Pids3))
				m_Pids[m_NumPids++] = *Pids3;

	if (m_NumPids >= MAXRECEIVEPIDS) {
		esyslog("ERROR: Streamdev: No free slot to receive pid %d\n", Pid);
		return false;
	}

	m_Pids[m_NumPids] = 0;
	StartReceiver();
	return true;
}

void cStreamdevLiveStreamer::StartReceiver(void)
{
	DELETENULL(m_Receiver);
	if (m_NumPids > 0) {
		Dprintf("Creating Receiver to respect changed pids\n");
#if VDRVERSNUM < 10500
		m_Receiver = new cStreamdevLiveReceiver(this, m_Channel->Ca(), m_Priority, m_Pids);
#else
		m_Receiver = new cStreamdevLiveReceiver(this, m_Channel->GetChannelID(), m_Priority, m_Pids);
#endif
		if (IsRunning() && m_Device != NULL) {
			Dprintf("Attaching new receiver\n");
			Attach();
		}
	}
}

bool cStreamdevLiveStreamer::SetChannel(const cChannel *Channel, eStreamType StreamType, int Apid) 
{
	Dprintf("Initializing Remuxer for full channel transfer\n");
	//printf("ca pid: %d\n", Channel->Ca());
	m_Channel = Channel;
	m_StreamType = StreamType;

	int apid[2] = { Apid, 0 };
	const int *Apids = Apid ? apid : m_Channel->Apids();
	const int *Dpids = Apid ? NULL : m_Channel->Dpids();

	switch (m_StreamType) {
	case stES: 
		{
			int pid = ISRADIO(m_Channel) ? m_Channel->Apid(0) : m_Channel->Vpid();
			if (Apid != 0)
				pid = Apid;
			m_ESRemux = new cTS2ESRemux(pid);
			return SetPids(pid);
		}

	case stPES: 
		m_PESRemux = new cRemux(m_Channel->Vpid(), m_Channel->Apids(), m_Channel->Dpids(), 
								m_Channel->Spids(), false);
		return SetPids(m_Channel->Vpid(), Apids, Dpids, m_Channel->Spids());

	case stPS:  
		m_PSRemux = new cTS2PSRemux(m_Channel->Vpid(), m_Channel->Apids(), m_Channel->Dpids(),
		                            m_Channel->Spids());
		return SetPids(m_Channel->Vpid(), Apids, Dpids, m_Channel->Spids());

	case stTS:
		// This should never happen, but ...
		if (m_PatFilter) {
			Detach();
			DELETENULL(m_PatFilter);
		}
		// Set pids from PMT
		m_PatFilter = new cStreamdevPatFilter(this, m_Channel);
		return true;

	case stExtern:
		m_ExtRemux = new cExternRemux(m_Channel->Vpid(), m_Channel->Apids(), m_Channel->Dpids(),
		                              m_Channel->Spids(), m_Parameter);
		return SetPids(m_Channel->Vpid(), Apids, Dpids, m_Channel->Spids());

	case stTSPIDS:
		Dprintf("pid streaming mode\n");
		return true;
	}
	return false;
}

int cStreamdevLiveStreamer::Put(const uchar *Data, int Count) 
{
	switch (m_StreamType) {
	case stTS:
	case stTSPIDS:
		return cStreamdevStreamer::Put(Data, Count);

	case stPES:
		return m_PESRemux->Put(Data, Count);

	case stES:
		return m_ESRemux->Put(Data, Count);

	case stPS:
		return m_PSRemux->Put(Data, Count);

	case stExtern:
		return m_ExtRemux->Put(Data, Count);

	default: // shouldn't happen???
		return 0;
	}
}

uchar *cStreamdevLiveStreamer::Get(int &Count)
{
	switch (m_StreamType) {
	case stTS:
	case stTSPIDS:
		return cStreamdevStreamer::Get(Count);

	case stPES:
		return m_PESRemux->Get(Count);
	
	case stES:
		return m_ESRemux->Get(Count);

	case stPS:
		return m_PSRemux->Get(Count);

	case stExtern:
		return m_ExtRemux->Get(Count);

	default: // shouldn't happen???
		return 0;
	}
}

void cStreamdevLiveStreamer::Del(int Count)
{
	switch (m_StreamType) {
	case stTS:
	case stTSPIDS:
		cStreamdevStreamer::Del(Count);
		break;

	case stPES:
		m_PESRemux->Del(Count);
		break;
	
	case stES:
		m_ESRemux->Del(Count);
		break;

	case stPS:
		m_PSRemux->Del(Count);
		break;

	case stExtern:
		m_ExtRemux->Del(Count);
		break;
	}
}
	
void cStreamdevLiveStreamer::Attach(void) 
{ 
	Dprintf("cStreamdevLiveStreamer::Attach()\n");
	if (m_Device) {
		if (m_Receiver) {
			m_Device->Detach(m_Receiver); 
			m_Device->AttachReceiver(m_Receiver); 
		}
		if (m_PatFilter) {
			m_Device->Detach(m_PatFilter); 
			m_Device->AttachFilter(m_PatFilter); 
		}
	}
}

void cStreamdevLiveStreamer::Detach(void) 
{ 
	Dprintf("cStreamdevLiveStreamer::Detach()\n");
	if (m_Device) {
		if (m_Receiver)
			m_Device->Detach(m_Receiver); 
		if (m_PatFilter)
			m_Device->Detach(m_PatFilter); 
	}
}

std::string cStreamdevLiveStreamer::Report(void) 
{
	std::string result;

	if (m_Device != NULL)
		result += (std::string)"+- Device is " + (const char*)itoa(m_Device->CardIndex()) + "\n";
	if (m_Receiver != NULL)
		result += "+- Receiver is allocated\n";
		
	result += "+- Pids are ";
	for (int i = 0; i < MAXRECEIVEPIDS; ++i) 
		if (m_Pids[i] != 0)
			result += (std::string)(const char*)itoa(m_Pids[i]) + ", ";
	result += "\n";
	return result;
}

// --- cStreamdevFilterStreamer -------------------------------------------------

#if VDRVERSNUM >= 10300
cStreamdevFilterStreamer::cStreamdevFilterStreamer():
		cStreamdevStreamer("streamdev-filterstreaming"),
		m_Device(NULL),
		m_Filter(NULL)/*,
		m_Channel(NULL)*/
{
}

cStreamdevFilterStreamer::~cStreamdevFilterStreamer() 
{
	Dprintf("Desctructing Filter streamer\n");
	Detach();
	m_Device = NULL;
	DELETENULL(m_Filter);
	Stop();
}

void cStreamdevFilterStreamer::Attach(void) 
{ 
	Dprintf("cStreamdevFilterStreamer::Attach()\n");
	LOCK_THREAD;
	if(m_Device && m_Filter)
		m_Device->AttachFilter(m_Filter);
}

void cStreamdevFilterStreamer::Detach(void) 
{ 
	Dprintf("cStreamdevFilterStreamer::Detach()\n");
	LOCK_THREAD;
	if(m_Device && m_Filter)
		m_Device->Detach(m_Filter); 
}

#if 0
void cStreamdevFilterStreamer::SetChannel(const cChannel *Channel)
{
	LOCK_THREAD;
	Dprintf("cStreamdevFilterStreamer::SetChannel(%s : %s)", Channel?Channel->Name():"<null>",
		Channel ? *Channel->GetChannelID().ToString() : "");
	m_Channel = Channel;
}
#endif

void cStreamdevFilterStreamer::SetDevice(cDevice *Device)
{
	Dprintf("cStreamdevFilterStreamer::SetDevice()\n");
	LOCK_THREAD;
	if(Device != m_Device) {
		Detach();
		m_Device = Device;
		//m_Channel = NULL;
		Attach();
	}
}

bool cStreamdevFilterStreamer::SetFilter(u_short Pid, u_char Tid, u_char Mask, bool On) 
{	
	Dprintf("cStreamdevFilterStreamer::SetFilter(%u,0x%x,0x%x,%s)\n", Pid, Tid, Mask, On?"On":"Off");

	if(!m_Device)
		return false;

	if (On) {
		if (m_Filter == NULL) {
			m_Filter = new cStreamdevLiveFilter(this);
			Dprintf("attaching filter to device\n");
			Attach();
		}
		m_Filter->Set(Pid, Tid, Mask);
	} else if (m_Filter != NULL) 
		m_Filter->Del(Pid, Tid, Mask);

	return true;
}

#if 0
void cStreamdevFilterStreamer::ChannelSwitch(const cDevice *Device, int ChannelNumber) {
	LOCK_THREAD;
	if(Device == m_Device) {
		if(ChannelNumber > 0) {
			cChannel *ch = Channels.GetByNumber(ChannelNumber);
			if(ch != NULL) {
				if(m_Filter != NULL &&
						m_Channel != NULL &&
						(! TRANSPONDER(ch, m_Channel))) {

					isyslog("***** LiveFilterStreamer: transponder changed ! %s",
						*ch->GetChannelID().ToString());

					uchar buffer[TS_SIZE] = {TS_SYNC_BYTE, 0xff, 0xff, 0xff, 0x7f, 0};
					strcpy((char*)(buffer + 5), ch->GetChannelID().ToString());
					int p = Put(buffer, TS_SIZE);
					if (p != TS_SIZE)
						ReportOverflow(TS_SIZE - p);
				}
				m_Channel = ch;
			}
		}
	}
}
#endif

#endif // if VDRVERSNUM >= 10300
