#include "remux/extern.h"
#include "server/streamer.h"
#include <vdr/tools.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

const char *g_ExternRemux = "/root/externremux.sh";

class cTSExt: public cThread {
private:
	cRingBufferLinear *m_ResultBuffer;
	bool               m_Active;
	int                m_Process;
	int                m_Inpipe, m_Outpipe;

protected:
	virtual void Action(void);

public:
	cTSExt(cRingBufferLinear *ResultBuffer);
	virtual ~cTSExt();

	void Put(const uchar *Data, int Count);
};

cTSExt::cTSExt(cRingBufferLinear *ResultBuffer):
		m_ResultBuffer(ResultBuffer),
		m_Active(false),
		m_Process(0),
		m_Inpipe(0),
		m_Outpipe(0)
{
	int inpipe[2];
	int outpipe[2];

	if (pipe(inpipe) == -1) {
		LOG_ERROR_STR("pipe failed");
		return;
	}

	if (pipe(outpipe) == -1) {
		LOG_ERROR_STR("pipe failed");
		close(inpipe[0]);
		close(inpipe[1]);
		return;
	}

	if ((m_Process = fork()) == -1) {
		LOG_ERROR_STR("fork failed");
		close(inpipe[0]);
		close(inpipe[1]);
		close(outpipe[0]);
		close(outpipe[1]);
		return;
	}

	if (m_Process == 0) {
		// child process
		dup2(inpipe[0], STDIN_FILENO);
		close(inpipe[1]);
		dup2(outpipe[1], STDOUT_FILENO);
		close(outpipe[0]);

		int MaxPossibleFileDescriptors = getdtablesize();
		for (int i = STDERR_FILENO + 1; i < MaxPossibleFileDescriptors; i++)
			close(i); //close all dup'ed filedescriptors

		//printf("starting externremux.sh\n");
		execl("/bin/sh", "sh", "-c", g_ExternRemux, NULL);
		//printf("failed externremux.sh\n");
		_exit(-1);
	}

	close(inpipe[0]);
	close(outpipe[1]);
	m_Inpipe = inpipe[1];
	m_Outpipe = outpipe[0];
	Start();
}

cTSExt::~cTSExt()
{
	m_Active = false;
	Cancel(3);
	if (m_Process > 0) {
		close(m_Outpipe);
		close(m_Inpipe);
		kill(m_Process, SIGTERM);
		for (int i = 0; waitpid(m_Process, NULL, WNOHANG) == 0; i++) {
			if (i == 20) {
				esyslog("streamdev-server: externremux process won't stop - killing it");
				kill(m_Process, SIGKILL);
			}
			cCondWait::SleepMs(100);
		}
	}
}

void cTSExt::Action(void)
{
	m_Active = true;
	while (m_Active) {
		fd_set rfds;
		struct timeval tv;

		FD_ZERO(&rfds);
		FD_SET(m_Outpipe, &rfds);

		while (FD_ISSET(m_Outpipe, &rfds)) {
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			if (select(m_Outpipe + 1, &rfds, NULL, NULL, &tv) == -1) {
				LOG_ERROR_STR("poll failed");
				break;;
			}

			if (FD_ISSET(m_Outpipe, &rfds)) {
				int result;
				//Read returns 0 if buffer full or EOF
				bool bufferFull = m_ResultBuffer->Free() <= 0; //Free may be < 0
				while ((result = m_ResultBuffer->Read(m_Outpipe)) == 0 && bufferFull)
					dsyslog("streamdev-server: buffer full while reading from externremux");

				if (result == -1) {
					if (errno != EINTR) {
						LOG_ERROR_STR("read failed");
						m_Active = false;
					}
					break;
				}
				else if (result == 0) {
					esyslog("streamdev-server: EOF reading from externremux");
					m_Active = false;
					break;
				}
			}
		}
	}
	m_Active = false;
}


void cTSExt::Put(const uchar *Data, int Count)
{
	if (safe_write(m_Inpipe, Data, Count) == -1) {
		LOG_ERROR_STR("write failed");
		return;
	}
}

cExternRemux::cExternRemux(int VPid, const int *APids, const int *Dpids, const int *SPids):
		m_ResultBuffer(new cRingBufferLinear(WRITERBUFSIZE, TS_SIZE * 2)),
		m_Remux(new cTSExt(m_ResultBuffer))
{
	m_ResultBuffer->SetTimeouts(500, 100);
}

cExternRemux::~cExternRemux()
{
	delete m_Remux;
	delete m_ResultBuffer;
}

int cExternRemux::Put(const uchar *Data, int Count) 
{
	m_Remux->Put(Data, Count);
	return Count;
}
