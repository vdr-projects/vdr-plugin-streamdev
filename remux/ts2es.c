#include "remux/ts2es.h"
#include "common.h"

// from VDR's remux.c
#define MAXNONUSEFULDATA (10*1024*1024)

class cTS2ES: public ipack {
	friend void PutES(uint8_t *Buffer, int Size, void *Data);

private:
	uint8_t *m_ResultBuffer;
	int *m_ResultCount;

public:
	cTS2ES(uint8_t *ResultBuffer, int *ResultCount);
	~cTS2ES();

	void PutTSPacket(const uint8_t *Buffer);
};

void PutES(uint8_t *Buffer, int Size, void *Data) {
	cTS2ES *This = (cTS2ES*)Data;
	uint8_t payl = Buffer[8] + 9 + This->start - 1;
	int count = Size - payl;

	if (*This->m_ResultCount + count > RESULTBUFFERSIZE) {
		esyslog("ERROR: result buffer overflow (%d + %d > %d)", 
				*This->m_ResultCount, count, RESULTBUFFERSIZE);
		count = RESULTBUFFERSIZE - *This->m_ResultCount;
	}
	memcpy(This->m_ResultBuffer + *This->m_ResultCount, Buffer + payl, count);
	*This->m_ResultCount += count;
	This->start = 1;
}

cTS2ES::cTS2ES(uint8_t *ResultBuffer, int *ResultCount) {
	m_ResultBuffer = ResultBuffer;
	m_ResultCount = ResultCount;

	init_ipack(this, IPACKS, PutES, 0);
	data = (void*)this;
}

cTS2ES::~cTS2ES() {
}

void cTS2ES::PutTSPacket(const uint8_t *Buffer) {
  if (!Buffer)
     return;

  if (Buffer[1] & 0x80) { // ts error
		// TODO
	}

  if (Buffer[1] & 0x40) { // payload start
		if (plength == MMAX_PLENGTH - 6) {
    	plength = found - 6;
      found = 0;
      send_ipack(this);
      reset_ipack(this);
    }
  }

	uint8_t off = 0;

  if (Buffer[3] & 0x20) {  // adaptation field?
		off = Buffer[4] + 1;
    if (off + 4 > TS_SIZE - 1)
      return;
  }

  instant_repack((uint8_t*)(Buffer + 4 + off), TS_SIZE - 4 - off, this);
}

cTS2ESRemux::cTS2ESRemux(int Pid):
		cTSRemux(false) {
	m_Pid = Pid;
  m_Remux = new cTS2ES(m_ResultBuffer, &m_ResultCount);
}

cTS2ESRemux::~cTS2ESRemux() {
	delete m_Remux;
}

void cTS2ESRemux::PutTSPacket(int Pid, const uint8_t *Data) {
	if (Pid == m_Pid) m_Remux->PutTSPacket(Data);
}

