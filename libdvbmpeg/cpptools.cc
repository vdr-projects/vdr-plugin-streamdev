/*
 *  dvb-mpegtools for the Siemens Fujitsu DVB PCI card
 *
 * Copyright (C) 2000, 2001 Marcus Metzler 
 *            for convergence integrated media GmbH
 * Copyright (C) 2002  Marcus Metzler 
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

 * The author can be reached at mocm@metzlerbros.de
 */

#include "cpptools.hh"

#define HEX(N) "0x" << hex << setw(2) << setfill('0') \
<< int(N) << " " << dec
#define HHEX(N,M) "0x" << hex << setw(M) << setfill('0') \
<< int(N) << " " << dec
#define LHEX(N,M) "0x" << hex << setw(M) << setfill('0') \
<< long(N) << " " << dec

#define MAX_SEARCH 1024 * 1024

ostream & operator << (ostream & stream, PES_Packet & x){
  
	if (x.info){
		cerr << "PES Packet: " ;
		switch ( x.p.stream_id ) {
				
		case PROG_STREAM_MAP:
			cerr << "Program Stream Map";
			break;
		case PRIVATE_STREAM2:
			cerr << "Private Stream 2";
			break;
		case PROG_STREAM_DIR:
			cerr << "Program Stream Directory";
			break;
		case ECM_STREAM     :
			cerr << "ECM Stream";
			break;
		case EMM_STREAM     :
			cerr << "EMM Stream";
			break;
		case PADDING_STREAM :
			cerr << "Padding Stream";
			break;
		case DSM_CC_STREAM  :
			cerr << "DSM Stream";
			break;
		case ISO13522_STREAM:
			cerr << "ISO13522 Stream";
			break;
		case PRIVATE_STREAM1:
			cerr << "Private Stream 1";
			break;
		case AUDIO_STREAM_S ... AUDIO_STREAM_E:
			cerr << "Audio Stream " << HEX(x.p.stream_id);
			break;
		case VIDEO_STREAM_S ... VIDEO_STREAM_E:
			cerr << "Video Stream " << HEX(x.p.stream_id);
			break;
			
		}
		cerr << " MPEG" << x.p.mpeg << endl;
		if ( x.p.mpeg == 2 ){
			cerr << "    FLAGS: ";

			if (x.p.flags1 & SCRAMBLE_FLAGS){
				cerr << " SCRAMBLE(";
				cerr << ((x.p.flags1 & SCRAMBLE_FLAGS)>>4);
				cerr << ")";
			}
			if (x.p.flags1 & PRIORITY_FLAG) 
				cerr << " PRIORITY";    
			if (x.p.flags1 & DATA_ALIGN_FLAG)
				cerr << " DATA_ALIGN";  
			if (x.p.flags1 & COPYRIGHT_FLAG) 
				cerr << " COPYRIGHT";   
			if (x.p.flags1 & ORIGINAL_FLAG) 
				cerr << " ORIGINAL";   

			if (x.p.flags2 & PTS_DTS_FLAGS){
				cerr << " PTS_DTS(";
				cerr << ((x.p.flags2 & PTS_DTS_FLAGS)>>6);
				cerr << ")";
			}
			if (x.p.flags2 & ESCR_FLAG) 
				cerr << " ESCR";
			if (x.p.flags2 & ES_RATE_FLAG)
				cerr << " ES_RATE";   
			if (x.p.flags2 & DSM_TRICK_FLAG)
				cerr << " DSM_TRICK";   
			if (x.p.flags2 & ADD_CPY_FLAG)
				cerr << " ADD_CPY";     
			if (x.p.flags2 & PES_CRC_FLAG) 
				cerr << " CRC";     
			if (x.p.flags2 & PES_EXT_FLAG)
				cerr << " EXT";     

			cerr << endl;

			if ((x.p.flags2 & PTS_DTS_FLAGS) == PTS_ONLY)
				cerr << "   PTS: " 
				     << LHEX(ntohl(x.WPTS()),8)
				     << "(h" << int(x.high_pts()) << ")"
				     << endl; 
			else if ((x.p.flags2 & PTS_DTS_FLAGS) == PTS_DTS){
				cerr << "   PTS: " 
				     << LHEX(ntohl(x.WPTS()),8)
				     << "(h" << int(x.high_pts()) << ")";
				cerr << "   DTS: " 
				     << LHEX(ntohl(x.WDTS()),8)
				     << "(h" << int(x.high_dts()) << ")"
				     << endl; 
			}
/*			
			if (x.p.flags2 & ESCR_FLAG)

			
			if (x.p.flags2 & ES_RATE_FLAG)

			
			if (x.p.flags2 & DSM_TRICK_FLAG)

			
			if (x.p.flags2 & ADD_CPY_FLAG)

			
			if (x.p.flags2 & PES_CRC_FLAG)

			
			if (x.p.flags2 & PES_EXT_FLAG){
				
				if (x.p.priv_flags & PRIVATE_DATA)
					stream.write(x.p.pes_priv_data,16);
				
				if (x.p.priv_flags & HEADER_FIELD){
					stream.write(&x.p.pack_field_length,1);
					x.p.pack_header = new 
						uint8_t[x.p.pack_field_length];
					stream.write(x.p.pack_header,
						     x.p.pack_field_length);
				}
				
				if ( x.p.priv_flags & PACK_SEQ_CTR){
					stream.write(&x.p.pck_sqnc_cntr,1);
					stream.write(&x.p.org_stuff_length,1);
				}
				
				if ( x.p.priv_flags & P_STD_BUFFER)
					stream.write(x.p.p_std,2);
				
				if ( x.p.priv_flags & PES_EXT_FLAG2){
					stream.write(&x.p.pes_ext_lngth,1);
					x.p.pes_ext = new 
						uint8_t[x.p.pes_ext_lngth];
					stream.write(x.p.pes_ext,
						     x.p.pes_ext_lngth);
				}
			}
		} else {
			if ((x.p.flags2 & PTS_DTS_FLAGS) == PTS_ONLY)
				stream.write(x.p.pts,5);
			else if ((x.p.flags2 & PTS_DTS_FLAGS) == 
				 PTS_DTS){
				stream.write(x.p.pts,5);
				stream.write(x.p.dts,5);
			}
*/	
		}			
		cerr << endl << endl;
		return stream;
	}

	int l = x.p.length+x.p.pes_hlength+9;
	uint8_t buf[l];
	int length = cwrite_pes(buf,&(x.p),l);
	stream.write((char *)buf,length);
	
	return stream;
}

static unsigned int find_length(istream & stream){
	streampos p = 0;
	streampos start = 0;
	streampos q = 0;
	int found = 0;
	uint8_t sync4[4];

	start = stream.tellg();
	start -=2;
	stream.seekg(start);
	while ( !stream.eof() && !found ){
		p = stream.tellg();
		stream.read((char *)&sync4,4);
		if (sync4[0] == 0x00 && sync4[1] == 0x00 && sync4[2] == 0x01) {
			switch ( sync4[3] ) {
				
			case PROG_STREAM_MAP:
			case PRIVATE_STREAM2:
			case PROG_STREAM_DIR:
			case ECM_STREAM     :
			case EMM_STREAM     :
			case PADDING_STREAM :
			case DSM_CC_STREAM  :
			case ISO13522_STREAM:
			case PRIVATE_STREAM1:
			case AUDIO_STREAM_S ... AUDIO_STREAM_E:
			case VIDEO_STREAM_S ... VIDEO_STREAM_E:
				found = 1;
				break;
			default:
				q = stream.tellg();
				break;
			}	
		} 
	}
	q = stream.tellg();
	stream.seekg(streampos(2)+start);
	if (found) return (unsigned int)(q-start)-4-2;
	else return (unsigned int)(q-start)-2;
	
}

istream & operator >> (istream & stream, PES_Packet & x){
	
	uint8_t sync4[4];
	int found=0;
	int done=0;
	streampos p = 0;
	
	while (!stream.eof() && !found) {
	        p = stream.tellg();
		stream.read((char *)&sync4,4);
		if (sync4[0] == 0x00 && sync4[1] == 0x00 && sync4[2] == 0x01) {
			x.p.stream_id = sync4[3];

			switch ( sync4[3] ) {
				
			case PROG_STREAM_MAP:
			case PRIVATE_STREAM2:
			case PROG_STREAM_DIR:
			case ECM_STREAM     :
			case EMM_STREAM     :
				found = 1;
				stream.read((char *)x.p.llength,2);
				x.setlength();
				if (!x.p.length){ 
					x.p.length = find_length(stream);
					x.Nlength();
				}
				stream.read((char *)x.p.pes_pckt_data,x.p.length);
				done = 1;
				break;
			case PADDING_STREAM :
				found = 1;
				stream.read((char *)x.p.llength,2);
				x.setlength();
				if (!x.p.length){ 
					x.p.length = find_length(stream);
					x.Nlength();
				}
				x.p.padding = x.p.length;
				stream.read((char *)x.p.pes_pckt_data,x.p.length);
				done = 1;
				break;
				
			case DSM_CC_STREAM  :
			case ISO13522_STREAM:
			case PRIVATE_STREAM1:
			case AUDIO_STREAM_S ... AUDIO_STREAM_E:
			case VIDEO_STREAM_S ... VIDEO_STREAM_E:
				stream.read((char *)x.p.llength,2);
				x.setlength();
				if (!x.p.length){ 
					x.p.length = find_length(stream);
					x.Nlength();
				}
				found = 1;
				break;
				
			default:
				stream.seekg(p+streampos(1));
			break;
			}	
		} else stream.seekg(p+streampos(1));
	}
	
	if ( found && !done) {
		p = stream.tellg();
		stream.read((char *)&x.p.flags1,1);
		if ( (x.p.flags1 & 0xC0) == 0x80 )
			x.p.mpeg = 2;
		else
			x.p.mpeg = 1;
		if ( x.p.mpeg == 2 ){
			stream.read((char *)&x.p.flags2,1);
			stream.read((char *)&x.p.pes_hlength,1);
			
			if ((int)x.p.length > x.p.pes_hlength+3)
				x.p.length -=x.p.pes_hlength+3;
			else 
				return stream;

			uint8_t count = x.p.pes_hlength;

			if ((x.p.flags2 & PTS_DTS_FLAGS) == PTS_ONLY){
				stream.read((char *)x.p.pts,5);
				count -=5;
			} else 
				if ((x.p.flags2 & PTS_DTS_FLAGS) == PTS_DTS){
					stream.read((char *)x.p.pts,5);
					stream.read((char *)x.p.dts,5);
					count -= 10;
				}

			if (x.p.flags2 & ESCR_FLAG){
				stream.read((char *)x.p.escr,6);
				count -= 6;
			}

			if (x.p.flags2 & ES_RATE_FLAG){
				stream.read((char *)x.p.es_rate,3);
				count -= 6;
			}

			if (x.p.flags2 & DSM_TRICK_FLAG){
				stream.read((char *)&x.p.trick,1);
				count -= 1;
			}

			if (x.p.flags2 & ADD_CPY_FLAG){
				stream.read((char *)&x.p.add_cpy,1);
				count -= 1;
			}

			if (x.p.flags2 & PES_CRC_FLAG){
				stream.read((char *)x.p.prev_pes_crc,2);
				count -= 2;
			}			

			if (x.p.flags2 & PES_EXT_FLAG){
				stream.read((char *)&x.p.priv_flags,1);
				count -= 1;
				
				if (x.p.priv_flags & PRIVATE_DATA){
					stream.read((char *)x.p.pes_priv_data,16);
					count -= 16;
				}

				if (x.p.priv_flags & HEADER_FIELD){
					stream.read((char *)&x.p.pack_field_length,1);
					x.p.pack_header = new 
						uint8_t[x.p.pack_field_length];
					stream.read((char *)x.p.pack_header,
						    x.p.pack_field_length);
					count -= 1+x.p.pack_field_length;
				}
				
				if ( x.p.priv_flags & PACK_SEQ_CTR){
					stream.read((char *)&x.p.pck_sqnc_cntr,1);
					stream.read((char *)&x.p.org_stuff_length,1);
					count -= 2;
				}

				if ( x.p.priv_flags & P_STD_BUFFER){
					stream.read((char *)x.p.p_std,2);
					count -= 2;
				}

				if ( x.p.priv_flags & PES_EXT_FLAG2){
					stream.read((char *)&x.p.pes_ext_lngth,1);
					x.p.pes_ext = new 
						uint8_t[x.p.pes_ext_lngth];
					stream.read((char *)x.p.pes_ext,
						    x.p.pes_ext_lngth);
					count -= 1+x.p.pes_ext_lngth;
				}
			}
			x.p.stuffing = count;
			uint8_t dummy;
			for(int i = 0; i< count ;i++) 
				stream.read((char *)&dummy,1);
			
		} else {
			uint8_t check;
			x.p.mpeg1_pad = 1;
			check = x.p.flags1;
			while (check == 0xFF){
				stream.read((char *)&check,1);
				x.p.mpeg1_pad++;
			}
		    
			if ( (check & 0xC0) == 0x40){
				stream.read((char *)&check,1);
				x.p.mpeg1_pad++;
				stream.read((char *)&check,1);
				x.p.mpeg1_pad++;
			}
			x.p.flags2 = 0;
			x.p.length -= x.p.mpeg1_pad;

			stream.seekg(p);
			if ( (check & 0x30)){
				x.p.length ++;
				x.p.mpeg1_pad --;

				if (check == x.p.flags1){
					x.p.pes_hlength = 0;
				} else {
					x.p.mpeg1_headr = 
						new uint8_t[x.p.mpeg1_pad];
					x.p.pes_hlength = x.p.mpeg1_pad;
					stream.read((char *)x.p.mpeg1_headr,
						    x.p.mpeg1_pad);
				}

				x.p.flags2 = (check & 0xF0) << 2;
				if ((x.p.flags2 & PTS_DTS_FLAGS) == PTS_ONLY){
					stream.read((char *)x.p.pts,5);
					x.p.length -= 5;
					x.p.pes_hlength += 5;
				}
				else if ((x.p.flags2 & PTS_DTS_FLAGS) == 
					 PTS_DTS){
					stream.read((char *)x.p.pts,5);
					stream.read((char *)x.p.dts,5);
					x.p.length -= 10;
					x.p.pes_hlength += 10;
				}
			} else {
				x.p.mpeg1_headr = new uint8_t[x.p.mpeg1_pad];
				x.p.pes_hlength = x.p.mpeg1_pad;
				stream.read((char *)x.p.mpeg1_headr,x.p.mpeg1_pad);
			}
		}
		stream.read((char *)x.p.pes_pckt_data,x.p.length);
	}
	return stream;
}

ostream & operator << (ostream & stream, TS_Packet & x){

	uint8_t buf[TS_SIZE];
	int length = cwrite_ts(buf,&(x.p),TS_SIZE);
	stream.write((char *)buf,length);

	return stream;
}

istream & operator >> (istream & stream, TS_Packet & x){
	uint8_t sync;
	int found=0;
	streampos p,q;

	sync=0;
	while (!stream.eof() && !found) {
		stream.read((char *)&sync,1);
		if (sync == 0x47) 
			found = 1;
	}
	stream.read((char *)x.p.pid,2);
	stream.read((char *)&x.p.flags,1);
	x.p.count = x.p.flags & COUNT_MASK;
	 
	if (!(x.p.flags & ADAPT_FIELD) && (x.p.flags & PAYLOAD)){
		//no adapt. field only payload
		stream.read((char *)x.p.data,184);
		x.p.rest = 184;
		return stream;
	} 

	if ( x.p.flags & ADAPT_FIELD ) {
		// adaption field
		stream.read((char *)&x.p.adapt_length,1);
		if (x.p.adapt_length){
			p = stream.tellg();
			stream.read((char *)&x.p.adapt_flags,1);
			
			if ( x.p.adapt_flags & PCR_FLAG )
				stream.read((char *) x.p.pcr,6);

			if ( x.p.adapt_flags & OPCR_FLAG )
				stream.read((char *) x.p.opcr,6);

			if ( x.p.adapt_flags & SPLICE_FLAG )
				stream.read((char *) &x.p.splice_count,1);
			
			if( x.p.adapt_flags & TRANS_PRIV){
				stream.read((char *)&x.p.priv_dat_len,1);
				x.p.priv_dat = new uint8_t[x.p.priv_dat_len];
				stream.read((char *)x.p.priv_dat,x.p.priv_dat_len);
			}
			
			if( x.p.adapt_flags & ADAP_EXT_FLAG){
				stream.read((char *)&x.p.adapt_ext_len,1);
				stream.read((char *)&x.p.adapt_eflags,1);
				if( x.p.adapt_eflags & LTW_FLAG)
					stream.read((char *)x.p.ltw,2);
				
				if( x.p.adapt_eflags & PIECE_RATE)
					stream.read((char *)x.p.piece_rate,3);
				
				if( x.p.adapt_eflags & SEAM_SPLICE)
					stream.read((char *)x.p.dts,5);
			}
			q = stream.tellg();
			x.p.stuffing = x.p.adapt_length -(q-p);
			x.p.rest = 183-x.p.adapt_length;
			stream.seekg(q+streampos(x.p.stuffing));
			if (x.p.flags & PAYLOAD) // payload
				stream.read((char *)x.p.data,x.p.rest);
			else 
				stream.seekg(q+streampos(x.p.rest));
		} else {
			x.p.rest = 182;
			stream.read((char *)x.p.data, 183);
			return stream;
		}

	}
	return stream;
}


ostream & operator << (ostream & stream, PS_Packet & x){

	uint8_t buf[PS_MAX];
	int length = cwrite_ps(buf,&(x.p),PS_MAX);
	stream.write((char *)buf,length);

	return stream;
}

istream & operator >> (istream & stream, PS_Packet & x){
	uint8_t headr[4];
	int found=0;
	streampos p = 0;
	streampos q = 0;
	int count = 0;

	p = stream.tellg();
	while (!stream.eof() && !found && count < MAX_SEARCH) {
		stream.read((char *)&headr,4);
		if (headr[0] == 0x00 && headr[1] == 0x00 && headr[2] == 0x01)
			if ( headr[3] == 0xBA ) 
				found = 1;
			else if ( headr[3] == 0xB9 ) break;
			else stream.seekg(p+streampos(1));
		count++;
	}
	
	if (found){
		stream.read((char *)x.p.scr,6);
		if (x.p.scr[0] & 0x40)
			x.p.mpeg = 2;
		else
			x.p.mpeg = 1;

		if (x.p.mpeg == 2){
			stream.read((char *)x.p.mux_rate,3);
			stream.read((char *)&x.p.stuff_length,1);
			p = stream.tellg();
			stream.seekg(p+streampos(x.p.stuff_length & 3));
		} else {
			x.p.mux_rate[0] = x.p.scr[5]; //mpeg1 scr is only 5 bytes
			stream.read((char *)x.p.mux_rate+1,2);
		}
			
		p=stream.tellg();
		stream.read((char *)headr,4);
		if (headr[0] == 0x00 && headr[1] == 0x00 && 
		    headr[2] == 0x01 && headr[3] == 0xBB ) {
			stream.read((char *)x.p.sheader_llength,2);
			x.setlength();
			if (x.p.mpeg == 2){
				stream.read((char *)x.p.rate_bound,3);
				stream.read((char *)&x.p.audio_bound,1);
				stream.read((char *)&x.p.video_bound,1);
				stream.read((char *)&x.p.reserved,1);
			}
			stream.read((char *)x.p.data,x.p.sheader_length);
		} else {
			stream.seekg(p);
			x.p.sheader_length = 0;
		}

		int i = 0;
		int done = 0;
		q = stream.tellg();
		PES_Packet pes;
		do {
			p=stream.tellg();
			stream.read((char *)headr,4);
			stream.seekg(p);
			if ( headr[0] == 0x00 && headr[1] == 0x00 
			     && headr[2] == 0x01 && headr[3] != 0xBA){
				pes.init();
				stream >> pes;
			        i++;
			} else done = 1;
		} while (!stream.eof() && !done);
		x.p.npes = i;
		stream.seekg(q);
	} 
 	return stream;
}

void extract_audio_from_PES(istream &in, ostream &out){
	PES_Packet pes;
	
	while(!in.eof()){
		pes.init();
		in >> pes ;
		if (pes.Stream_ID() == 0xC0)
			out << pes;
	}
}

void extract_video_from_PES(istream &in, ostream &out){
	PES_Packet pes;
	
	while(!in.eof()){
		pes.init();
		in >> pes ;
		if (pes.Stream_ID() == 0xE0)
			out << pes;
	}
}

void extract_es_audio_from_PES(istream &in, ostream &out){
	PES_Packet pes;
	
	while(!in.eof()){
		pes.init();
		in >> pes ;
		if (pes.Stream_ID() == 0xC0)
		  out.write((char *)pes.Data(),pes.Length());
	}
}

void extract_es_video_from_PES(istream &in, ostream &out){
	PES_Packet pes;
	
	while(!in.eof()){
		pes.init();
		in >> pes ;
		if (pes.Stream_ID() == 0xE0)
		  out.write((char *)pes.Data(),pes.Length());
	}
}



#define MAX_PID 20
int TS_PIDS(istream &in, ostream &out){
	int pid[MAX_PID];
	TS_Packet ts;
	int npid=0;
	
	for (int i=0 ; i<MAX_PID; i++)
		pid[i] = -1;
	while (!in.eof()) {
		ts.init();
		in >> ts;
		int j;
		int found = 0;
		for (j=0;j<npid;j++){
			if ( ts.PID() == pid[j] )
				found = 1;
		}
		if (! found){ 
			out << ts.PID() << " ";
			pid[npid] = ts.PID();
			npid++;
			if (npid == MAX_PID) return -1;
		}
	}
	out << endl;
	return 0;
}

int tv_norm(istream &stream){
	uint8_t headr[4];
	int found=0;
	streampos p = 0;
	streampos q = 0;
	int hsize,vsize;
	int form= 0;

	q = stream.tellg();
	while (!stream.eof() && !found) {
		p = stream.tellg();
		stream.read((char *)headr,4);
		if (headr[0] == 0x00 && headr[1] == 0x00 && headr[2] == 0x01)
			if ( headr[3] == 0xB3 ){ 
				found = 1;
			}
		if (! found) stream.seekg(p+streampos(1));
	}
	stream.read((char *)headr,4);

	hsize = (headr[1] &0xF0) >> 4 | headr[0] << 4;
	vsize = (headr[1] &0x0F) << 8 | headr[2];
	cerr << "SIZE: " << hsize << "x" << vsize << endl;
	
	switch(((headr[3]&0xF0) >>4)){
	case 1:
		cerr << "ASPECT: 1:1" << endl;
		break;
	case 2:
		cerr << "ASPECT: 4:3" << endl;
		break;
	case 3:
		cerr << "ASPECT: 16:9" << endl;
		break;
	case 4:
		cerr << "ASPECT: 2.21:1" << endl;
		break;
	}

	switch (int(headr[3]&0x0F)){
	case 1:
		cerr << "FRAMERATE: 23.976" << endl;
		form = pDUNNO;
		break;
	case 2:
		cerr << "FRAMERATE: 24" << endl;
		form = pDUNNO;
		break;
	case 3:
		cerr << "FRAMERATE: 25" << endl;
		form = pPAL;
		break;
	case 4:
		cerr << "FRAMERATE: 29.97" << endl;
		form = pNTSC;
		break;
	case 5:
		cerr << "FRAMERATE: 30" << endl;
		form = pNTSC;
		break;
	case 6:
		cerr << "FRAMERATE: 50" << endl;
		form = pPAL;
		break;
	case 7:
		cerr << "FRAMERATE: 59.94" << endl;
		form = pNTSC;
		break;
	case 8:
		cerr << "FRAMERATE: 60" << endl;
		form = pNTSC;
		break;
	}

	int mpeg = 0;
	found = 0;
	while (!stream.eof() && !found) {
		p = stream.tellg();
		stream.read((char *)headr,4);
		if (headr[0] == 0x00 && headr[1] == 0x00 && headr[2] == 0x01)
			if ( headr[3] == 0xB5 ){ 
				char *profile[] = {"reserved", "High", "Spatially Scalable",
						   "SNR Scalable", "Main", "Simple", "undef",
						   "undef"};
				char *level[]   = {"res", "res", "res", "res",
						   "High","res", "High 1440", "res",
						   "Main","res", "Low", "res",
						   "res", "res", "res", "res"};
				char *chroma[]  = {"res", "4:2:0", "4:2:2", "4:4:4:"};
				mpeg = 2;
				stream.read((char *)headr,4);
				cerr << "PROFILE: " << profile[headr[0] & 0x7] << endl;
				cerr << "LEVEL: " << level[headr[1]>>4 & 0xF] << endl;
				cerr << "CHROMA: " << chroma[headr[1]>>1 & 0x3] << endl;
				found = 1;
			} else {
				mpeg = 1;
				found = 1;
			}				
		if (! found) stream.seekg(p+streampos(1));
	}

	stream.seekg(q);
	return (form | mpeg << 4);
}



int stream_type(istream &fin){
	uint8_t headr[4];
	streampos p=fin.tellg();
	
	TS_Packet ts;
	fin >> ts;
	fin.read((char *)headr,1);
	fin.seekg(p);
	if(fin && headr[0] == 0x47){
		return TS_STREAM;
	}

	PS_Packet ps;
	fin >> ps;
	PES_Packet pes;
	for(int j=0;j < ps.NPES();j++){
		fin >> pes;
	}
	fin.read((char *)headr,4);
	fin.seekg(p);
	if (fin && headr[0] == 0x00 && headr[1] == 0x00
	    && headr[2] == 0x01 && headr[3] == 0xBA){
		return PS_STREAM;
	}
	
	fin >> pes;
	fin.read((char *)!headr,4);
			fin.seekg(p);
	if (fin && headr[0] == 0x00 && headr[1] == 0x00
	    && headr[2] == 0x01 ){
		int found = 0;
		switch ( headr[3] ) {
			
		case PROG_STREAM_MAP:
		case PRIVATE_STREAM2:
		case PROG_STREAM_DIR:
		case ECM_STREAM     :
		case EMM_STREAM     :
		case PADDING_STREAM :
		case DSM_CC_STREAM  :
		case ISO13522_STREAM:
		case PRIVATE_STREAM1:
		case AUDIO_STREAM_S ... AUDIO_STREAM_E:
		case VIDEO_STREAM_S ... VIDEO_STREAM_E:
			found = 1;
			break;
		}
		if (found){
			return PES_STREAM;
		}
	}

	

	return -1;
}


void analyze(istream &fin)
{
	PS_Packet ps;
	PES_Packet pes;
	int fc = 0;
	char *frames[3] = {"I-Frame","P-Frame","B-Frame"};
			
	while(fin){
		uint32_t pts;
		fin >> ps;
		cerr << "SCR base: " << hex << setw(5) 
		     << setfill('0') \
		     << ps.SCR_base() << " " << dec
		     << "ext : " << ps.SCR_ext();

		cerr << "   MUX rate: " << ps.MUX()*50*8/1000000.0
		     << " Mbit/s   ";
		cerr << "RATE bound: " << ps.Rate()*50*8/1000000.0
		     << " Mbit/s" << endl;
		cerr << "                             Audio bound: " 
		     << hex << "0x" 
		     << int(ps.P()->audio_bound);
		cerr << "         Video bound: " << hex << "0x" 
		     << int(ps.P()->video_bound)
		     << dec
		     << endl;
		cerr << endl;

		for (int i=0; i < ps.NPES(); i++){
			pes.init();
			fin >> pes;
			pts2pts((uint8_t *)&pts,pes.PTS());
			pes.Info() = 1;
			cerr  << pes;

			uint8_t *buf = pes.P()->pes_pckt_data;
			int c = 0;
			int l;
			switch (pes.P()->stream_id){
			case VIDEO_STREAM_S ... VIDEO_STREAM_E:
				l=pes.P()->length;
				break;
			default:
				l = 0;
				break;
			}
			while ( c < l - 6){
				if (buf[c] == 0x00 && 
				    buf[c+1] == 0x00 &&
				    buf[c+2] == 0x01 && 
				    buf[c+3] == 0xB8) {
					c += 4;
					cerr << "TIME   hours: " 
					     << int((buf[c]>>2)& 0x1F)
					     << "  minutes: "
					     << int(((buf[c]<<4)& 0x30)|
						    ((buf[c+1]>>4)& 0x0F))
					     << " seconds: "
					     << int(((buf[c+1]<<3)& 0x38)|
						    ((buf[c+2]>>5)& 0x0F))
					     << endl;
				}
				
				if ( buf[c] == 0x00 && 
				     buf[c+1] == 0x00 &&
				     buf[c+2] == 0x01 && 
				     buf[c+3] == 0x00) {
					fc++;
					c += 4;
					cerr << "picture: " 
					     << fc 
					     << " ("
					     << frames[((buf[c+1]&0x38) >>3)-1]
					     << ")" << endl << endl;
				} else c++;
			}
		}
	}
}


