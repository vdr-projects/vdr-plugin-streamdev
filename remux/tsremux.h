#ifndef VDR_STREAMDEV_TSREMUX_H
#define VDR_STREAMDEV_TSREMUX_H

#include "libdvbmpeg/transform.h"
#include <vdr/remux.h>

#define RESULTBUFFERSIZE KILOBYTE(256)

class cTSRemux {
protected:
  /*uchar m_ResultBuffer[RESULTBUFFERSIZE];
	int m_ResultCount;
	int m_ResultDelivered;
	int m_Synced;
	int m_Skipped;
	int m_Sync;


	virtual void PutTSPacket(int Pid, const uint8_t *Data) = 0;

public:
	cTSRemux(bool Sync = true);
	virtual ~cTSRemux();
	
	virtual uchar *Process(const uchar *Data, int &Count, int &Result);*/

	static void SetBrokenLink(uchar *Data, int Length);
	static int GetPid(const uchar *Data);
 	static int GetPacketLength(const uchar *Data, int Count, int Offset);
	static int ScanVideoPacket(const uchar *Data, int Count, int Offset, uchar &PictureType);
};

#endif // VDR_STREAMDEV_TSREMUX_H
