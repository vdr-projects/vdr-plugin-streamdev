#include "tools/select.h"

#include <vdr/tools.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

cTBSelect::cTBSelect(void) {
	Clear();
}

cTBSelect::~cTBSelect() {
}

int cTBSelect::Select(uint TimeoutMs) {
	struct timeval tv;
	time_t st, et;
	ssize_t res;
	int ms;

	tv.tv_usec = (TimeoutMs % 1000) * 1000;
	tv.tv_sec = TimeoutMs / 1000;

	if (TimeoutMs == 0)
		return ::select(m_MaxFiled + 1, &m_Rfds, &m_Wfds, NULL, &tv);

	st = time_ms();
	ms = TimeoutMs;
	while (ms > 0 && (res = ::select(m_MaxFiled + 1, &m_Rfds, &m_Wfds, NULL, 
			&tv)) == -1 && errno == EINTR) {
		et = time_ms();
		ms -= et - st;
		tv.tv_usec = (ms % 1000) * 1000;
		tv.tv_sec = ms / 1000;
		st = et;
	}
	if (ms <= 0) {
		errno = ETIMEDOUT;
		return -1;
	}
	return res;
}

int cTBSelect::Select(void) {
	ssize_t res;
	while ((res = ::select(m_MaxFiled + 1, &m_Rfds, &m_Wfds, NULL, NULL)) == -1
			&& errno == EINTR)
		;
	return res;
}
