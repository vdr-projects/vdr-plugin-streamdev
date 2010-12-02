#ifndef _DVB_DEV_HH_
#define _DVB_DEV_HH_

extern "C" {
#include <asm/errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define NEWSTRUCT
#include <channel.h>
}

#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <osd.hh>
#include <devices.hh>

#ifndef MAXNAM
#define MAXNAM 80
#endif



#define FRONT_DVBS 1
#define FRONT_DVBC 2
#define FRONT_DVBT 3

#define VTXDIR "/var/vtx"

#define DEC(N) dec << setw(N) << setfill('0') 
#define HEX(N) hex << setw(N) << setfill('0') 

#define MAXSECSIZE 4096

#define NK 10
enum {LNB=0,DIS,ROTOR,TRANS,CHAN,BOU,SAT,PICS,SWI,NTW};
static const int nums[]={LNB,DIS,ROTOR,TRANS,CHAN,BOU,SAT,PICS,SWI,NTW};
static const int maxs[]={ 32, 32,   32,  512,16384,512,100, 50, 10, 100};

#define MAX_TRANS_CHAN  1024

enum{DVB_ORIG=0, DVB_NOKIA, DVB_XML, DVB_SATCO};

typedef struct frontend_stat_s{
	fe_status_t status;
	uint16_t snr;
	uint16_t strength;
	uint32_t ber;
	uint32_t u_blocks;
} frontend_stat;


extern uint8_t hamtab[256];
extern uint8_t invtab[256];

#define MAX_MAG 8
typedef struct mag_struct_ {
        int valid;
        int magn;
        uint8_t flags;
        uint8_t lang;
        int pnum,sub;
        uint8_t pagebuf[25*40];
} magazin_t;


class DVB {
public:
	int no_open;
	int fd_frontend;
	int fd_demuxa;
	int fd_demuxv;
	int fd_demuxpcr;
	int fd_demuxtt;
        int fdvb;

	int minor;
	int adapter;
	int max_tpid;
	int max_satid;
	int max_chanid;
	
	frontend_stat festat;

	struct dvb_diseqc_master_cmd dcmd;
	fe_sec_tone_mode_t tone;
	fe_sec_voltage_t voltage;
        int burst;
 	struct dmx_pes_filter_params pesFilterParamsV; 
	struct dmx_pes_filter_params pesFilterParamsA; 
	struct dmx_pes_filter_params pesFilterParamsP; 
	struct dmx_pes_filter_params pesFilterParamsTT; 
	struct dvb_frontend_parameters front_param;
        int front_type;
	int dvr_enabled;
        OSD osd;
	uint32_t transponder_freq;
	char transponder_pol;
	uint32_t transponder_srate;



	fe_status_t status;
	uint32_t ber, uncorrected_blocks;
	uint16_t snr, signal;


        struct Lnb *lnbs;
        struct DiSEqC *diseqcs;
        struct Rotor *rotors;
        struct Transponder *tps;
        struct Channel *chans;
        struct Bouquet *bouqs;
        struct Sat *sats;
        struct Picture *pics;
        struct Switch *swis;
        struct Network *ntws;
        int num[NK];
	int oldsec;
	int tryit;
	int oldpol;

	char *vtxdir;
	magazin_t magazin[MAX_MAG]; 

	DVB(){
		no_open = 0;
		max_tpid = 0;
		max_satid = 0;
		max_chanid = 0;
		minor = 0;

		fd_frontend = -1;
		fd_demuxa = -1;
		fd_demuxpcr = -1;
		fd_demuxv = -1;
		fd_demuxtt = -1;
		fdvb = -1;
		vtxdir = NULL;
		transponder_freq=0;
		transponder_pol=0;
		transponder_srate=0;
	}

	DVB(int i){
		if (i >= 0) 
			no_open = 0;
		else 
			no_open = 1;
		max_tpid = 0;
		max_satid = 0;
		max_chanid = 0;
	
		fd_frontend = -1;
		fd_demuxa = -1;
		fd_demuxpcr = -1;
		fd_demuxv = -1;
		fd_demuxtt = -1;
		fdvb = -1;
		vtxdir = NULL;
		transponder_freq=0;
		transponder_pol=0;
		transponder_srate=0;

	        init("","",i);
	}

        DVB(char *a, char *b) {
		max_tpid = 0;
		max_satid = 0;
		max_chanid = 0;
	
		fd_frontend = -1;
		fd_demuxa = -1;
		fd_demuxpcr = -1;
		fd_demuxv = -1;
		fd_demuxtt = -1;

		fdvb = -1;
		vtxdir = NULL;
	        init(a,b,0);
	}

	~DVB();
  
	void use_osd(int fd = -1){
		char dvn[32];
		if (no_open) return;
		if (fd < 0) fd = 0;
		sprintf(dvn,OSD_DEV,adapter,fd);
		fdvb = open(dvn, O_RDWR);
		
		if (fdvb >= 0){
			cerr << dvn <<  " for OSD" << endl;
			osd.init(fdvb);
		} else perror("osd");
		osd.Open(80, 500, 640, 540, 2, 0, 2);
		osd.SetColor(0, 0, 0, 0, 255);
		osd.SetColor(1, 240, 240, 240, 255);
		osd.Show();
	}

	void set_vtxdir(char *newname){
		if (!newname) return;
		if (vtxdir) free(vtxdir);
		vtxdir = (char *) malloc(sizeof(char)*(strlen(newname)+1));
		if (vtxdir)
			strncpy(vtxdir, newname, strlen(newname));
	}

	void close_osd(){
		osd.Close(fdvb);
		close(fdvb);
	}
  
	int DVR_enabled(){
		if (no_open) return -1;
		return dvr_enabled;
	}

	void enable_DVR(){
		if (no_open) return;
		dvr_enabled = 1;
	}

	void enable_DVR_other(){
		if (no_open) return;
		dvr_enabled = 2;
	}

	void disable_DVR(){
		if (no_open) return;
		dvr_enabled = 0;
	}

        void init(char *a="/dev/video0", char *b="/dev/vbi0",int adapt=0,
		  int minor = 0); 
		  

	inline void init(char *a, char *b){
		if (no_open) return;
		init(a,b,0,0);
	}

	int check_frontend();

	void set_apid(ushort apid);
	void set_vpid(ushort vpid); 
	void set_pcrpid(ushort vpid); 
	void set_ttpid(ushort ttpid); 
        int set_apid_fd(ushort apid, int fd);
        int set_vpid_fd(ushort vpid, int fd);
        int set_ttpid_fd(ushort ttpid, int fd);
        int set_pcrpid_fd(ushort pcrpid, int fd);
        int set_otherpid_fd(ushort otherpid, int fd);


        int set_lnb(int dis);
	void set_diseqc_nb(int nr); 
	int set_front(void); 
	void get_front(void); 

	void scan_pf_eit(int chnr,
			 int (*callback)(uint8_t *data, int l, int pnr, 
					  int c_n, uint8_t *t));

	void scan_pf_eit(Channel *chan, 
			 int (*callback)(uint8_t *data, int l, int pnr, 
					  int c_n, uint8_t *t));
	void scan_pf_eit(int chnr);


	int search_in_TP(Transponder &tp, int show=1, int verbose=0);
	int search_in_TP(uint16_t tpid, uint16_t satid, int show=1,
			 int verbose=0);
	int scan_TP(uint16_t tpid, uint16_t satid, int timeout=-1, int verbose=0);

	int GetSection(uint8_t *buf, 
		       uint16_t PID, uint8_t TID, uint16_t TIDExt, 
		       uint16_t FilterTIDExt, 
		       uint8_t secnum, uint8_t &msecnum);
	int GetSection(uint8_t *buf, 
		       uint16_t PID, uint8_t *filter, uint8_t *mask,
		       uint8_t secnum, uint8_t &msecnum); 
	int GetSection(uint8_t *buf, ushort PID, uint8_t sec,
		       uint8_t secnum, uint8_t &msecnum); 
	int SetFilter(uint16_t pid, uint8_t *filter, 
		      uint8_t *mask,
		      uint32_t timeout, uint32_t flags); 
	uint16_t SetFilter(uint16_t pid, uint16_t section, uint16_t mode); 
	int CloseFilter(int h);
	

	void bar2(int x, int y, int w, int h, int val, int col1, int col2);

        int SetTP(unsigned int, unsigned int);
        int scan(void);
        int scan_all_tps(void);
        int scan_lnb(struct Lnb &);
        int scan_cable(Sat &sat);
        int scan_sat(struct Sat &);
        int scan_tp(struct Transponder &);

        int AddLNB(int id, int t, uint l1, uint l2, uint sl,
		   int dnr, int dis, int sw);
	int AddSat(Sat &sat);
        int AddSat(int satid, unsigned int lnbid, char *name, uint fmin, uint fmax);
        int AddTP(Transponder &tp);
        int AddChannel(Channel &chan);
	int parse_descriptor(Channel *chan, uint8_t *data, int length);
	int parse_pmt(Channel *chan, uint8_t *data);
	int parse_pat(Channel *chan, uint8_t *data);

	int check_pids(Channel *chan);
	void check_all_pids();
	void scan_sdt(Channel *chan);
	int scan_sdts(int *chs, int n);

        int channel_num(void) {
	        return num[CHAN];
	};
	  
        int channel_change(int n) {
	        return 0;
	};
        int SetChannel(uint16_t, uint16_t, uint16_t, uint16_t);
	int SetChannel(Channel *chan,  char* apref=NULL, uint16_t *apidp=NULL, 
		       uint16_t *vpidp=NULL) ;
        int SetChannel(int chnr, char *apref=NULL, uint16_t *apidp=NULL, 
		       uint16_t *vpidp=NULL);
        int GetChannel(int chnr, struct channel *);
        int NumChannel(void) {
	        return num[CHAN];
	}
	int tune_it(struct dvb_frontend_parameters *qpsk);
	void find_satid(Channel &chan);
	int check_input_format(istream &ins);
	void read_original(istream &ins);
	int get_all_progs(uint16_t *progbuf, uint16_t *pnrbuf, int length);
	uint16_t find_pnr(uint16_t vpid, uint16_t apid);
	int get_pids(uint16_t prog_pid, uint16_t *vpid, uint16_t *apids, 
		     uint16_t *ttpid, uint8_t *apids_name=NULL);
	void AddECM(Channel *chan, uint8_t *data, int length);
	int check_ecm(Channel *chan);
	void add_vtx_line(magazin_t *mag, int line, uint8_t *data, int pnr);
	
        friend ostream &operator<<(ostream &stream, DVB &x);
        friend istream &operator>>(istream &stream, DVB &x);

};

#define NOKIA_MAX_SAT 4
class nokiaconv{
public:
	DVB *dvb;
	struct lnb_sat_l{
		int n;
		int diseqc[NOKIA_MAX_SAT];
		char sat_names[NOKIA_MAX_SAT][MAXNAM+1];
		int satid[NOKIA_MAX_SAT];
	} lnb_sat;

	nokiaconv(DVB *d){
		dvb = d;
	}

        friend istream &operator>>(istream &stream, nokiaconv &x);
};

#define XML_MAX_SAT 4
class xmlconv{
public:
	DVB *dvb;
	struct lnb_sat_l{
		int n;
		int diseqc[XML_MAX_SAT];
		char sat_names[XML_MAX_SAT][MAXNAM+1];
		int satid[XML_MAX_SAT];
	} lnb_sat;

	xmlconv(DVB *d){
		dvb = d;
	}
	int read_stream(istream &ins, int nchan);
	int read_desc(istream &ins, int nchan);
	int read_serv(istream &ins, int ctp, int csat);
	int read_trans(istream &ins, int satid);
	int read_sat(istream &ins, int satid = -1);
	int skip_tag(istream &ins, char *tag);
	int read_iso639(istream &ins, int nchan, int apids);

        friend istream &operator>>(istream &stream, xmlconv &x);
};



#define SATCO_MAX_SAT 10
class satcoconv{
public:
	DVB *dvb;
	int nlnb;

	satcoconv(DVB *d){
		dvb = d;
	}

        friend istream &operator>>(istream &stream, satcoconv &x);
};

void hdump(uint8_t *buf, int n);
int get_dvbrc(char *path, DVB &dv, int dev, int len);
int set_dvbrc(char *path, DVB &dv, int dev, int len);
void dvb2txt(char *out, char *in, int len);
int set_sfront(int fdf, uint32_t freq, uint32_t pol, uint32_t sr , int snum, fe_code_rate_t fec);
void set_pes_filt(int fd,uint16_t pes_pid);
void set_diseqc(int fdf, int snum, fe_sec_voltage_t v, fe_sec_tone_mode_t t);
int tune(int fdf, uint32_t freq, uint32_t sr, fe_code_rate_t fec);
int set_sfront(int fdf, uint32_t freq, uint32_t pol, uint32_t sr , int snum, 
	       fe_code_rate_t fec);


struct in_addr getaddress (const char *name);
int tcp_client_connect(const char *hostname, int sckt);
int udp_client_connect(const char *filename);
void client_send_msg(int fd, uint8_t *msg, int size);
int chck_frontend (int fefd, frontend_stat *festat);

uint8_t deham(uint8_t x, uint8_t y);

#endif
