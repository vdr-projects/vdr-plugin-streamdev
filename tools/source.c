#include "tools/source.h"
#include "tools/select.h"
#include "common.h"

#include <vdr/tools.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

cTBSource::cTBSource(void) {
	m_BytesRead = 0;
	m_BytesWritten = 0;
	m_Filed = -1;
}

bool cTBSource::Open(int Filed, bool IsUnixFd) {
	if (IsOpen())
		Close();

	m_Filed = Filed;
	if (IsUnixFd && ::fcntl(m_Filed, F_SETFL, O_NONBLOCK) == -1)
		return false;

	return true;
}

cTBSource::~cTBSource() {
}

bool cTBSource::Close(void) {
	if (!IsOpen()) {
		errno = EBADF;
		return false;
	}

	m_Filed = -1;
	return true;
}

ssize_t cTBSource::Read(void *Buffer, size_t Length) {
	ssize_t res;
	while ((res = SysRead(Buffer, Length)) < 0 && errno == EINTR)
		errno = 0;
	if (res > 0) m_BytesRead += res;
	return res;
}

ssize_t cTBSource::Write(const void *Buffer, size_t Length) {
	ssize_t res;
	while ((res = SysWrite(Buffer, Length)) < 0 && errno == EINTR)
		errno = 0;
	if (res > 0) m_BytesWritten += res;
	return res;
}

bool cTBSource::TimedWrite(const void *Buffer, size_t Length, uint TimeoutMs) {
	cTBSelect sel;
	time_t st;
	int ms, offs;

	st = time_ms();
	ms = TimeoutMs;
	offs = 0;
	while (Length > 0) {
		int b;

		sel.Clear();
		sel.Add(m_Filed, true);
		if (sel.Select(ms) == -1)
			return false;

		if (sel.CanWrite(m_Filed)) {
			if ((b = Write((char*)Buffer + offs, Length)) == -1)
				return false;
			offs += b;
			Length -= b;
		}

		ms -= time_ms() - st;
		if (ms <= 0) {
			errno = ETIMEDOUT;
			return false;
		}
	}
	return true;
}

ssize_t cTBSource::ReadUntil(void *Buffer, size_t Length, const char *Seq,
		uint TimeoutMs) {
	char *offs;
	time_t st;
	int seqlen, ms;
	size_t olen;
	cTBSelect sel;

	seqlen = strlen(Seq);
	if ((offs = (char*)memmem(m_LineBuffer, m_LineBuffer.Length(), Seq, seqlen))){
		olen = offs - m_LineBuffer;
		if (olen >= Length) {
			errno = ENOBUFS;
			return -1;
		}
		memcpy(Buffer, m_LineBuffer, olen);
		m_LineBuffer = m_LineBuffer.Mid(olen + seqlen);
		Dprintf("ReadUntil: Served from Linebuffer: %d, |%.*s|\n", olen, olen - 1,
				(char*)Buffer);
		return olen;
	}

	st = time_ms();
	ms = TimeoutMs;
	while (m_LineBuffer.Length() < BUFSIZ) {
		int b;

		sel.Clear();
		sel.Add(m_Filed, false);

		if (sel.Select(ms) == -1)
			return -1;
		
		if (sel.CanRead(m_Filed)) {
			offs = m_LineBuffer.Buffer(BUFSIZ);
			if ((b = Read(offs + m_LineBuffer.Length(), BUFSIZ 
					- m_LineBuffer.Length())) == -1)
				return -1;

			m_LineBuffer.Release(m_LineBuffer.Length() + b);
			if ((offs = (char*)memmem(m_LineBuffer, m_LineBuffer.Length(), Seq, 
					seqlen))) {
				olen = offs - m_LineBuffer;
				if (olen >= Length) {
					errno = ENOBUFS;
					return -1;
				}
				memcpy(Buffer, m_LineBuffer, olen);
				m_LineBuffer = m_LineBuffer.Mid(olen + seqlen, m_LineBuffer.Length() 
						- olen - seqlen);
				Dprintf("ReadUntil: Served after Read: %d, |%.*s|\n", olen, olen-1,
						(char*)Buffer);
				return olen;
			}
		}

		ms -= time_ms() - st;
		if (ms <= 0) {
			errno = ETIMEDOUT;
			return -1;
		}
	}
	errno = ENOBUFS;
	return -1;
	
	

/*
	cTBSelect sel;
	time_t st, et;
	int ms, seqlen, offs;

	seqlen = strlen(Seq);
	st = time_ms();
	ms = TimeoutMs;
	offs = 0;
	while (Length > 0) {
		int b;

		sel.Clear();
		sel.Add(m_Filed, false);
		if (sel.Select(ms) == -1)
			return -1;

		if (sel.CanRead(m_Filed)) {
			if ((b = Read((char*)Buffer + offs, Length)) == -1)
				return -1;

			offs += b;
			Length -= b;

			if (memmem(Buffer, offs, Seq, seqlen) != NULL)
				return offs;
		}

		et = time_ms();
		ms -= et - st;
		if (ms <= 0) {
			errno = ETIMEDOUT;
			return -1;
		}
	}
	errno = ENOBUFS;
	return -1;
*/
}

