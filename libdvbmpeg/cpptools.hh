/*
 *  dvb-mpegtools for the Siemens Fujitsu DVB PCI card
 *
 * Copyright (C) 2000, 2001 Marcus Metzler 
 *            for convergence integrated media GmbH
 * Copyright (C) 2002 Marcus Metzler 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 * 

 * The author can be reached at mocm@metzlerbros.de, 
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;


#ifndef _CPPTOOLS_HH_
#define _CPPTOOLS_HH_

#include "ctools.h"


class PES_Packet{
	int info;
	pes_packet p;
public:
	PES_Packet(){
		info = 0;
		init_pes(&p);
	}

	~PES_Packet(){
		if (p.pack_header)
			delete [] p.pack_header;
		if (p.pes_ext)
			delete [] p.pes_ext;
		if (p.pes_pckt_data)
			delete [] p.pes_pckt_data;
		if (p.mpeg1_headr)
			delete [] p.mpeg1_headr;
	}
	
	inline void init(){
		if (p.pack_header)
			delete [] p.pack_header;
		if (p.pes_ext)
			delete [] p.pes_ext;
		if (p.pes_pckt_data)
			delete [] p.pes_pckt_data;
		if (p.mpeg1_headr)
			delete [] p.mpeg1_headr;

		info = 0;
		init_pes(&p);
	}

	inline pes_packet *P(){
		return &p;
	}

	inline void setlength(){
		setlength_pes(&p);
		if (p.length)
			p.pes_pckt_data = new uint8_t[p.length];
	}

	inline void Nlength(){
		nlength_pes(&p);
		p.pes_pckt_data = new uint8_t[p.length];
	}


	inline uint8_t &Stream_ID(){
		return p.stream_id;
	}

	inline uint8_t &Flags1(){
		return p.flags1;
	}

	inline uint8_t &Flags2(){
		return p.flags2;
	}

	inline uint32_t &Length(){
		return p.length;
	}
	
	inline uint8_t &HLength(){
		return p.pes_hlength;
	}

	inline uint8_t &Stuffing(){
		return p.stuffing;
	}

	inline uint8_t *Data(){
		return p.pes_pckt_data;
	}
	
	inline int has_pts(){
		return (p.flags2 & PTS_DTS);
	}

	inline int &MPEG(){
		return p.mpeg;
	}
	inline uint8_t *PTS(){
		return p.pts;
	}

	inline uint8_t *DTS(){
		return p.dts;
	}

	inline int &Info(){
		return info;
	}
  
  

	inline uint8_t high_pts(){
		if (has_pts())
			return ((p.pts[0] & 0x08)>>3);
		else
			return 0;
	}

	inline uint8_t high_dts(){
		return ((p.dts[0] & 0x08)>>3);
	}

	inline int WDTS(){
		int w_dts;
		w_dts = (int)trans_pts_dts(p.dts);
		return w_dts;
	}

	inline int WPTS(){
		int w_dts;
		w_dts = (int)trans_pts_dts(p.pts);
		return w_dts;
	}

	friend ostream & operator << (ostream & stream, PES_Packet & x);
	friend istream & operator >> (istream & stream, PES_Packet & x);

};


class TS_Packet{
	ts_packet p;
	int info;

public:
	TS_Packet(){
		init_ts(&p);
		info = 0;
	}

	~TS_Packet(){
		if (p.priv_dat)
			delete [] p.priv_dat;
	}
	
	inline void init(){
		if (p.priv_dat)
			delete [] p.priv_dat;

		init_ts(&p);
		info = 0;
	}

	inline ts_packet *P(){
		return &p;
	}
		
	inline int &Rest(){
		return p.rest;
	}

	inline uint8_t *Data(){
		return p.data;
	}

	inline short PID(){
		return pid_ts(&p);
	}

	inline uint8_t FLAG1(){
		return (p.pid[0] & ~PID_MASK_HI);
	}

	inline int &Info(){
		return info;
	}

	friend ostream & operator << (ostream & stream, TS_Packet & x);
	friend istream & operator >> (istream & stream, TS_Packet & x);
};


class PS_Packet{
	int info;
	ps_packet p;
public:

	PS_Packet(){
		init_ps(&p);
		info = 0;
	}
	
	~PS_Packet(){
		if (p.data)
			delete [] p.data;
	}

	inline void init(){
		if (p.data)
			delete [] p.data;

		init_ps(&p);
		info = 0;
	}

	inline ps_packet *P(){
		return &p;
	}

	inline int MUX(){
		return mux_ps(&p);
	}

	inline int Rate(){
		return rate_ps(&p);
	}

	inline void setlength(){
		setlength_ps(&p);
		p.data = new uint8_t[p.sheader_length];
	}

	inline int Stuffing(){
		return p.stuff_length & PACK_STUFF_MASK;
	}

	inline int NPES(){
		return p.npes;
	}

	inline int &MPEG(){
		return p.mpeg;
	}

	inline uint8_t &operator()(int l){
		return p.data[l];
	}

	inline char * Data() {
		return (char *)p.data+p.stuff_length;
	}

	inline int &SLENGTH(){
		return p.sheader_length;
	}
	
	inline int &Info(){
		return info;
	}
	
	uint32_t SCR_base(){
		return scr_base_ps(&p);
	}

	uint16_t SCR_ext(){
		return scr_ext_ps(&p);
	}

	friend ostream & operator << (ostream & stream, PS_Packet & x);
	friend istream & operator >> (istream & stream, PS_Packet & x);
};


typedef void (* FILTER)(istream &in, ostream &out);

typedef struct thread_args_{
	FILTER function;
	int *fd;
	int in;
	int out;
} thread_args;


void extract_audio_from_PES(istream &in, ostream &out);
void extract_video_from_PES(istream &in, ostream &out);
void extract_es_audio_from_PES(istream &in, ostream &out);
void extract_es_video_from_PES(istream &in, ostream &out);
int TS_PIDS(istream &in, ostream &out);
int ifilter (istream &in, FILTER function);
int ofilter (istream &in, FILTER function);
int itfilter (int in, FILTER function);
int otfilter (istream &in, FILTER function);
int stream_type(int fd);
int stream_type(istream &stream);
int tv_norm(istream &fin);

void analyze(istream &fin);


#endif //_CPPTOOLS_HH_

