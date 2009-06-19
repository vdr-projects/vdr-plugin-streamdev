#include "remux/extern.h"
#include "server/server.h"
#include "server/streamer.h"
#include <vdr/tools.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

namespace Streamdev {

class cTSExt: public cThread {
private:
	cRingBufferLinear *m_ResultBuffer;
	bool               m_Active;
	int                m_Process;
	int                m_Inpipe, m_Outpipe;

protected:
	virtual void Action(void);

public:
	cTSExt(cRingBufferLinear *ResultBuffer, std::string Parameter);
	virtual ~cTSExt();

	void Put(const uchar *Data, int Count);
};

} // namespace Streamdev
using namespace Streamdev;

cTSExt::cTSExt(cRingBufferLinear *ResultBuffer, std::string Parameter):
		m_ResultBuffer(ResultBuffer),
		m_Active(false),
		m_Process(-1),
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

		std::string cmd = std::string(opt_remux) + " " + Parameter;
		if (execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL) == -1) {
			esyslog("streamdev-server: externremux script '%s' execution failed: %m", cmd.c_str());
			_exit(-1);
		}
		// should never be reached
		_exit(0);
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
		// close pipes
		close(m_Outpipe);
		close(m_Inpipe);
		// signal and wait for termination
		if (kill(m_Process, SIGINT) < 0) {
			esyslog("streamdev-server: externremux SIGINT failed: %m");
		}
		else {
			int i = 0;
			int retval;
			while ((retval = waitpid(m_Process, NULL, WNOHANG)) == 0) {

				if ((++i % 20) == 0) {
					esyslog("streamdev-server: externremux process won't stop - killing it");
					kill(m_Process, SIGKILL);
				}
				cCondWait::SleepMs(100);
			}

			if (retval < 0)
				esyslog("streamdev-server: externremux process waitpid failed: %m");
			else
				Dprintf("streamdev-server: externremux child (%d) exited as expected\n", m_Process);
		}
		m_Process = -1;
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

cExternRemux::cExternRemux(int VPid, const int *APids, const int *Dpids, const int *SPids, std::string Parameter):
		m_ResultBuffer(new cRingBufferLinear(WRITERBUFSIZE, TS_SIZE * 2)),
		m_Remux(new cTSExt(m_ResultBuffer, Parameter))
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
