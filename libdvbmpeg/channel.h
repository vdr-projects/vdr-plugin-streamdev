#ifndef _CHANNEL_H
#define _CHANNEL_H

#include <sys/types.h>

struct channel {
        int id;
        char name[81];
        int type;
        ushort pnr;
        ushort vpid;
        ushort apids[8];
        ushort apidnum;
        ushort ac3pid;
        ushort pcrpid;
        
        uint freq;
        int pol;
        int qam;
        uint srate;
        int fec;
};

#ifdef NEWSTRUCT

#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>

#define DVR_DEV   "/dev/dvb/adapter%d/dvr%d"     
#define VIDEO_DEV "/dev/dvb/adapter%d/video%d"
#define AUDIO_DEV "/dev/dvb/adapter%d/audio%d"
#define DEMUX_DEV "/dev/dvb/adapter%d/demux%d"
#define FRONT_DEV "/dev/dvb/adapter%d/frontend%d"
#define OSD_DEV   "/dev/dvb/adapter%d/osd%d"
#define CA_DEV    "/dev/dvb/adapter%d/ca%d"

#else

#include <ost/dmx.h>
#include <ost/frontend.h>
#include <ost/sec.h>
#include <ost/video.h>
#include <ost/audio.h>

#define DVR_DEV   "/dev/ost/dvr%d"
#define VIDEO_DEV "/dev/ost/video%d"
#define AUDIO_DEV "/dev/ost/audio%d"
#define DEMUX_DEV "/dev/ost/demux%d"
#define FRONT_DEV "/dev/ost/frontend%d"
#define OSD_DEV   "/dev/ost/osd%d"
#define CA_DEV   "/dev/ost/ca%d"

#endif


#endif
