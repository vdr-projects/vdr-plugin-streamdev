#include "remux/tsremux.h"

// from VDR's remux.c
#define MAXNONUSEFULDATA (10*1024*1024)
#define SC_PICTURE 0x00  // "picture header"
#define VIDEO_STREAM_S   0xE0

cTSRemux::cTSRemux(bool Sync) {
	m_ResultCount = 0;
	m_ResultDelivered = 0;
	m_Synced = false;
	m_Skipped = 0;
	m_Sync = Sync;
}

cTSRemux::~cTSRemux(void) {
}

uchar *cTSRemux::Process(const uchar *Data, int &Count, int &Result) {
  // Remove any previously delivered data from the result buffer:
  if (m_ResultDelivered) {
    if (m_ResultDelivered < m_ResultCount)
      memmove(m_ResultBuffer, m_ResultBuffer + m_ResultDelivered, m_ResultCount 
					- m_ResultDelivered);
    m_ResultCount -= m_ResultDelivered;
    m_ResultDelivered = 0;
	}

  int used = 0;
  
	// Make sure we are looking at a TS packet:
  while (Count > TS_SIZE) {
		if (Data[0] == 0x47 && Data[TS_SIZE] == 0x47)
			break;
    Data++;
    Count--;
    used++;
  }
  if (used)
    esyslog("ERROR: skipped %d byte to sync on TS packet", used);
  
	// Convert incoming TS data
  for (int i = 0; i < Count; i += TS_SIZE) {
    if (Count - i < TS_SIZE)
      break;
    if (Data[i] != 0x47)
      break;
    int pid = get_pid((uint8_t*)(Data + i + 1));
    if (Data[i + 3] & 0x10) // got payload
			PutTSPacket(pid, Data + i);
    /*if      (pid == m_VPid)               m_VRemux->ConvertTSPacket(Data + i);
         else if (pid == m_APid1)              m_ARemux1->ConvertTSPacket(Data + i);
         else if (pid == m_APid2 && m_ARemux2) m_ARemux2->ConvertTSPacket(Data + i);
         else if (pid == m_DPid1 && m_DRemux1) m_DRemux1->ConvertTSPacket(Data + i);
         else if (pid == m_DPid2 && m_DRemux2) m_DRemux2->ConvertTSPacket(Data + i);*/
    used += TS_SIZE;
    if (m_ResultCount > (int)sizeof(m_ResultBuffer) / 2)
      break;
  }
  Count = used;
  
	// When we don't need to sync, we don't need to sync :-)
  if (!m_Sync) {
    Result = m_ResultDelivered = m_ResultCount;
    return Result ? m_ResultBuffer : NULL;
  }
  
	// Check if we're getting anywhere here:

  if (!m_Synced && m_Skipped >= 0) {
     if (m_Skipped > MAXNONUSEFULDATA) {
        esyslog("ERROR: no useful data seen within %d byte of video stream", m_Skipped);
        m_Skipped = -1;
        //if (exitOnFailure)
           //cThread::EmergencyExit(true);
        }
     else
        m_Skipped += Count;
     }

  // Check for frame borders:

  if (m_ResultCount > 0) {
     for (int i = 0; i < m_ResultCount; i++) {
         if (m_ResultBuffer[i] == 0 && m_ResultBuffer[i + 1] == 0 && m_ResultBuffer[i + 2] == 1) {
            switch (m_ResultBuffer[i + 3]) {
              case VIDEO_STREAM_S ... VIDEO_STREAM_E:
                   {
                     uchar pt = NO_PICTURE;
                     int l = ScanVideoPacket(m_ResultBuffer, m_ResultCount, i, pt);
                     if (l < 0)
                        return NULL; // no useful data found, wait for more
                     if (pt != NO_PICTURE) {
                        if (pt < I_FRAME || B_FRAME < pt)
                           esyslog("ERROR: unknown picture type '%d'", pt);
                        else if (!m_Synced) {
                           if (pt == I_FRAME) {
                              m_ResultDelivered = i; // will drop everything before this position
                              SetBrokenLink(m_ResultBuffer + i, l);
                              m_Synced = true;
                              }
                           else {
                              m_ResultDelivered = i + l; // will drop everything before and including this packet
                              return NULL;
                              }
                           }
                        }
                     if (m_Synced) {
                        Result = l;
                        uchar *p = m_ResultBuffer + m_ResultDelivered;
                        m_ResultDelivered += l;
                        return p;
                        }
                     else {
                        m_ResultDelivered = i + l; // will drop everything before and including this packet
                        return NULL;
                        }
                   }
                   break;
              case PRIVATE_STREAM1:
              case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                   {
                     int l = GetPacketLength(m_ResultBuffer, m_ResultCount, i);
                     if (l < 0)
                        return NULL; // no useful data found, wait for more
                     if (m_Synced) {
                        Result = l;
                        uchar *p = m_ResultBuffer + m_ResultDelivered;
                        m_ResultDelivered += l;
                        return p;
                        }
                     else {
                        m_ResultDelivered = i + l; // will drop everything before and including this packet
                        return NULL;
                        }
                   }
                   break;
              }
            }
         }
     }
  return NULL; // no useful data found, wait for more
}

int cTSRemux::ScanVideoPacket(const uchar *Data, int Count, int Offset, uchar &PictureType) {
  // Scans the video packet starting at Offset and returns its length.
  // If the return value is -1 the packet was not completely in the buffer.

  int Length = GetPacketLength(Data, Count, Offset);
  if (Length > 0 && Offset + Length <= Count) {
     int i = Offset + 8; // the minimum length of the video packet header
     i += Data[i] + 1;   // possible additional header bytes
     for (; i < Offset + Length; i++) {
         if (Data[i] == 0 && Data[i + 1] == 0 && Data[i + 2] == 1) {
            switch (Data[i + 3]) {
              case SC_PICTURE: PictureType = (Data[i + 5] >> 3) & 0x07;
                               return Length;
              }
            }
         }
     PictureType = NO_PICTURE;
     return Length;
     }
  return -1;
}

int cTSRemux::GetPacketLength(const uchar *Data, int Count, int Offset) {
  // Returns the entire length of the packet starting at offset, or -1 in case of error.
  return (Offset + 5 < Count) ? (Data[Offset + 4] << 8) + Data[Offset + 5] + 6 : -1;
}

void cTSRemux::SetBrokenLink(uchar *Data, int Length) {
  if (Length > 9 && Data[0] == 0 && Data[1] == 0 && Data[2] == 1 && (Data[3] & VIDEO_STREAM_S) == VIDEO_STREAM_S) {
     for (int i = Data[8] + 9; i < Length - 7; i++) { // +9 to skip video packet header
         if (Data[i] == 0 && Data[i + 1] == 0 && Data[i + 2] == 1 && Data[i + 3] == 0xB8) {
            if (!(Data[i + 7] & 0x40)) // set flag only if GOP is not closed
               Data[i + 7] |= 0x20;
            return;
            }
         }
     dsyslog("SetBrokenLink: no GOP header found in video packet");
     }
  else
     dsyslog("SetBrokenLink: no video packet in frame");
}
