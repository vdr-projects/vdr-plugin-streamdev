#include "tools/file.h"

#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

cTBFile::cTBFile(void) {
}

cTBFile::~cTBFile() {
	Close();
}

bool cTBFile::Open(const cTBString &Filename, int Mode, mode_t Attribs) {
	int filed;

	if (IsOpen()) Close();

	if ((filed = ::open(Filename, Mode, Attribs)) == -1)
		return false;

	if (!cTBSource::Open(filed))
		return false;

	m_Filename = Filename;
	m_Anonymous = false;
	return true;
}

bool cTBFile::Open(uint Fileno) {
	if (IsOpen()) Close();

	if (!cTBSource::Open(Fileno))
		return false;

	m_Filename.Format("<&%d>", Fileno);
	m_Anonymous = true;
	return true;
}

bool cTBFile::Close(void) {
	bool ret = true;

	if (!IsOpen())
		ERRNUL(EBADF);

	if (::close(*this) == -1)
		ret = false;

	if (!cTBSource::Close())
		ret = false;

	m_Filename.Clear();
	return ret;
}

bool cTBFile::Unlink(void) const {
	if (m_Filename.IsNull())
		ERRNUL(ENOENT);

	if (!IsOpen())
		ERRNUL(EBADF);

	if (m_Anonymous)
		ERRNUL(EINVAL);

	return cTBFile::Unlink(m_Filename);
}

bool cTBFile::Unlink(const cTBString &Filename) {
	return (::unlink(Filename) != -1);
}

ssize_t cTBFile::Size(void) const {
	struct stat buf;
	
	if (!IsOpen())
		ERRSYS(EBADF);

	if (fstat(*this, &buf) == -1)
		return -1;

	return buf.st_size;
}

ssize_t cTBFile::Size(const cTBString &Filename) {
	struct stat buf;
	
	if (stat(Filename, &buf) == -1)
		return -1;

	return buf.st_size;
}
