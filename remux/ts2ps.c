#include "remux/ts2ps.h"

class cTS2PS {
	friend void PutPES(uint8_t *Buffer, int Size, void *Data);

private:
	ipack m_Ipack;
	uint8_t *m_ResultBuffer;
	int *m_ResultCount;

public:
	cTS2PS(uint8_t *ResultBuffer, int *ResultCount, uint8_t AudioCid = 0x00, 
			bool PS = false);
	~cTS2PS();

	void PutTSPacket(const uint8_t *Buffer);
};

void PutPES(uint8_t *Buffer, int Size, void *Data) {
	cTS2PS *This = (cTS2PS*)Data;
	if (*This->m_ResultCount + Size > RESULTBUFFERSIZE) {
		esyslog("ERROR: result buffer overflow (%d + %d > %d)", 
				*This->m_ResultCount, Size, RESULTBUFFERSIZE);
		Size = RESULTBUFFERSIZE - *This->m_ResultCount;
	}
	memcpy(This->m_ResultBuffer + *This->m_ResultCount, Buffer, Size);
	*This->m_ResultCount += Size;
}

cTS2PS::cTS2PS(uint8_t *ResultBuffer, int *ResultCount, uint8_t AudioCid, 
		bool PS) {
	m_ResultBuffer = ResultBuffer;
	m_ResultCount = ResultCount;

	init_ipack(&m_Ipack, IPACKS, PutPES, PS);
	m_Ipack.cid = AudioCid;
	m_Ipack.data = (void*)this;
}

cTS2PS::~cTS2PS() {
}

void cTS2PS::PutTSPacket(const uint8_t *Buffer) {
  if (!Buffer)
     return;

  if (Buffer[1] & 0x80) { // ts error
		// TODO
	}

  if (Buffer[1] & 0x40) { // payload start
		if (m_Ipack.plength == MMAX_PLENGTH - 6 && m_Ipack.found > 6) {
    	m_Ipack.plength = m_Ipack.found - 6;
      m_Ipack.found = 0;
      send_ipack(&m_Ipack);
      reset_ipack(&m_Ipack);
    }
  }

	uint8_t off = 0;

  if (Buffer[3] & 0x20) {  // adaptation field?
		off = Buffer[4] + 1;
    if (off + 4 > TS_SIZE - 1)
      return;
  }

  instant_repack((uint8_t*)(Buffer + 4 + off), TS_SIZE - 4 - off, &m_Ipack);
}

cTS2PSRemux::cTS2PSRemux(int VPid, int APid1, int APid2, int DPid1, 
		int DPid2, bool PS) {
	m_VPid  = VPid;
	m_APid1 = APid1;
	m_APid2 = APid2;
	m_DPid1 = DPid1;
	m_DPid2 = DPid2;
  m_VRemux  =         new cTS2PS(m_ResultBuffer, &m_ResultCount, 0x00, PS);
  m_ARemux1 =         new cTS2PS(m_ResultBuffer, &m_ResultCount, 0xC0, PS);
  m_ARemux2 = APid2 ? new cTS2PS(m_ResultBuffer, &m_ResultCount, 0xC1, PS) 
	                  : NULL;
  m_DRemux1 = DPid1 ? new cTS2PS(m_ResultBuffer, &m_ResultCount, 0x00, PS) 
	                  : NULL;
  //XXX don't yet know how to tell apart primary and secondary DD data...
  m_DRemux2 = /*XXX m_DPid2 ? new cTS2PS(m_ResultBuffer, &m_ResultCount, 
			0x00, PS) : XXX*/ NULL;
}

cTS2PSRemux::~cTS2PSRemux() {
	if (m_DRemux2) delete m_DRemux2;
	if (m_DRemux1) delete m_DRemux1;
	if (m_ARemux2) delete m_ARemux2;
	delete m_ARemux1;
	delete m_VRemux;
}

void cTS2PSRemux::PutTSPacket(int Pid, const uint8_t *Data) {
	if      (Pid == m_VPid)               m_VRemux->PutTSPacket(Data);
	else if (Pid == m_APid1)              m_ARemux1->PutTSPacket(Data);
	else if (Pid == m_APid2 && m_ARemux2) m_ARemux2->PutTSPacket(Data);
	else if (Pid == m_DPid1 && m_DRemux1) m_DRemux1->PutTSPacket(Data);
	else if (Pid == m_DPid2 && m_DRemux2) m_DRemux2->PutTSPacket(Data);
}

