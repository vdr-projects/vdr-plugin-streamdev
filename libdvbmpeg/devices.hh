#ifndef _channel_hh
#define _channel_hh

using namespace std;
#include <stdint.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "DVB.hh"

#define MAXNAM 80
#define MAXKEY 15

const int maxname=80;
const int MAXAPIDS=32;
const uint32_t UNSET=0xffffffff;
const uint16_t NOID=0xffff;
const uint16_t NOPID=0xffff;

class Transponder {
public:
        uint16_t id;
        uint16_t onid;
        uint16_t satid;
        int type;
        char name[maxname+1];
        uint32_t freq;
        int pol;
        int qam;
        uint32_t srate;
        int fec;
        int band;
        int hp_rate;         
	int lp_rate; 
        int mod;    
        int transmode;
        int guard;
        int hierarchy;

        struct Sat *sat;
        
        Transponder() {
		name[0]='\0';
		id = NOID;
		onid = NOID;
		satid = NOID;
		type = 0;
	}

        friend ostream &operator<<(ostream &stream, Transponder &x);
        friend istream &operator>>(istream &stream, Transponder &x);
};

class Sat {
public:
        uint16_t id;
        char name[maxname+1];
        unsigned int lnbid;
        struct Lnb *lnb;
        unsigned int rotorid;
        unsigned int fmin;
        unsigned int fmax;

        Sat() {
	        id=NOID;
		name[0]='\0';
		lnb=NULL;
	        rotorid=NOID;
	        lnbid=NOID;
		fmin=fmax=0;
	};
	int set(int sid, char *sname, int slnbid, int srotorid) {
	  return 0;
        }; 

        friend ostream &operator<<(ostream &stream, Sat &x);
        friend istream &operator>>(istream &stream, Sat &x);
};


class Lnb {
public:
        Sat *sat;
        uint16_t id;
        struct DVB *dvbd;
        char name[maxname+1];
        int type;
        unsigned int lof1;
        unsigned int lof2;
        unsigned int slof;
        int diseqcnr;
        uint16_t diseqcid;
        uint16_t swiid;


        void cpy (const Lnb &olnb){
	  this->id=olnb.id;
	  this->type=olnb.type;
	  this->lof1=olnb.lof1;
	  this->lof2=olnb.lof2;
	  this->slof=olnb.slof;
	  this->diseqcnr=olnb.diseqcnr;
	  this->diseqcid=olnb.diseqcid;
	  this->swiid=olnb.swiid;
	  strncpy(this->name,olnb.name,maxname);
	}

        void init(int t, uint l1, uint l2, uint sl,
		  int dnr, int disid, int sw) {
		type=t;
		lof1=l1;
		lof2=l2;
		slof=sl;
		diseqcnr=dnr;
		diseqcid=disid;
		swiid=sw;
		dvbd=0;
		name[0]='\0';
	}

        Lnb () {
	        lof1=lof2=slof=0;
	        swiid=NOID;
		diseqcid=NOID;
		diseqcnr=-1;
		name[0]='\0';
	}
  
        Lnb (const Lnb &olnb){
          cpy(olnb);
	}

  

        friend ostream &operator<<(ostream &stream, Lnb &x);
        friend istream &operator>>(istream &stream, Lnb &x);
};

struct diseqcmsg {
        int burst;
        int len;
        unsigned char msg[8];
};

class DiSEqC {
public:
        uint16_t id;
        char name[maxname+1];
        diseqcmsg msgs[4];

        friend ostream &operator<<(ostream &stream, DiSEqC &x);
        friend istream &operator>>(istream &stream, DiSEqC &x);
};

class Rotor {
public:
        uint16_t id;
        char name[maxname+1];
        diseqcmsg msgs[4];

        friend ostream &operator<<(ostream &stream, Rotor &x);
        friend istream &operator>>(istream &stream, Rotor &x);
};

class Switch {
public:
        uint16_t id;
        int switchid;
        char name[maxname+1];
        diseqcmsg msg;

        friend ostream &operator<<(ostream &stream, Switch &x);
        friend istream &operator>>(istream &stream, Switch &x);
};

class Network {
public:
        uint16_t id;
        char name[maxname+1];

        friend ostream &operator<<(ostream &stream, Network &x);
        friend istream &operator>>(istream &stream, Network &x);
};

class Bouquet {
public:
        uint16_t id;
        char name[maxname+1];

        friend ostream &operator<<(ostream &stream, Bouquet &x);
        friend istream &operator>>(istream &stream, Bouquet &x);
};


#define MAX_ECM 16
#define MAX_ECM_DESC 256
typedef struct ecm_struct {
        int num;
        uint16_t sysid[MAX_ECM];
        uint16_t pid[MAX_ECM];
	uint16_t length[MAX_ECM];
	uint8_t data[MAX_ECM*MAX_ECM_DESC];
} ecm_t;



class Channel{
public:
        Channel *next;
        uint32_t id;
        char name[maxname+1];
        int32_t type;
	int checked;
  
        uint16_t pnr;
        uint16_t vpid;
        uint16_t apids[MAXAPIDS];
        char apids_name[MAXAPIDS*4];
        int32_t  apidnum;
        int  last_apidn;
        uint16_t ac3pid;
        uint16_t ttpid;
        uint16_t pmtpid;
        uint16_t pcrpid;
        uint16_t casystem;
        uint16_t capid;

	ecm_t  ecm;
	int (*ecm_callback)(Channel *chan);

	int has_eit;
	int pres_follow;

        uint16_t satid;
        uint16_t tpid;
        uint16_t onid;
        uint16_t bid;
	int8_t eit_ver_n;
	int8_t eit_ver_c;

        void clearall(void) {
	        id=UNSET;
		name[0]='\0';
		type=0;
		checked = 0;
		has_eit = -1;
		pres_follow = -1;
		eit_ver_c = -1;
		eit_ver_n = -1;
		
		pnr=NOPID;
		vpid=NOPID;
		memset(apids, 0, sizeof(uint16_t)*MAXAPIDS);
		memset(apids_name, 0, sizeof(char)*MAXAPIDS*4);
		apidnum=0;
		last_apidn=-1;
		ac3pid=NOPID;
		ttpid=NOPID;
		pmtpid=NOPID;
		pcrpid=NOPID;
		capid=NOPID;

		satid=NOID;
		tpid=NOID;
		onid=NOID;
		bid=NOID;
		ecm_callback = NULL;
		memset(&ecm,0, sizeof(ecm_t));
	};

        Channel() {
	        clearall();
	}
  
        Channel(int cid, char *nam, int ty, int prognr,
		int vid, int aid, int tid) {
	        int l=strlen(nam);

		clearall();
		if (l>maxname){
		  cerr << "" << endl; 
		  l=maxname;
		}
		strncpy(name, nam, l);
		name[l]='\0';
		type=ty;
		pnr=prognr;
		vpid=vid;
		apids[0]=aid;
	}

#ifdef DEBUG
        ~Channel(){
	        cerr <<"Channel " << name << "  destroyed" << endl;
	}
#endif
  
        friend ostream &operator<<(ostream &stream, Channel &x);
        friend istream &operator>>(istream &stream, Channel &x);
};

int findkey(char *name, char *keys[]);
void getname(char *name,istream &ins);
#endif /*channel.h*/
