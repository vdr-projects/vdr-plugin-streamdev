#ifndef TOOLBOX_FILE_H
#define TOOLBOX_FILE_H

#include "tools/tools.h"
#include "tools/source.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

/* cTBFile provides a cTBSource-derived interface for input and output on UNIX 
   files. */

class cTBFile: public cTBSource {
private:
	bool m_Anonymous;
	std::string m_Filename;

	/* Unhide and forbid baseclass method */
	virtual bool Open(int Fd, bool IsUnixFd = false) { return false; }

public:
	cTBFile(void);
	virtual ~cTBFile();

	/* enum eFileType represents the modes a file can be opened with. The full
	   open mode is one of the first three, maybe or'ed with one of the others.
	   */
	enum eFileType {
		ReadOnly  = O_RDONLY,
		WriteOnly = O_WRONLY,
		ReadWrite = O_RDWR,
		
		Create    = O_CREAT,
		Exclude   = O_EXCL,
		Truncate  = O_TRUNC,
		Append    = O_APPEND
	};

	/* See cTBSource::SysRead() 
	   Reimplemented for UNIX files. */
	virtual ssize_t SysRead(void *Buffer, size_t Length) const;

	/* See cTBSource::SysWrite() 
	   Reimplemented for UNIX files. */
	virtual ssize_t SysWrite(const void *Buffer, size_t Length) const;

	/* Open() opens the file referred to by Filename according to the given 
	   Mode. If the file is created, it receives the attributes given by 
	   Attribs, defaulting to rw-------. Returns true on success and false on
	   error, setting errno appropriately. */
	virtual bool Open(const std::string &Filename, int Mode, 
			mode_t Attribs = S_IRUSR + S_IWUSR);

	/* Open() associates this file object with Fileno. Fileno must refer to a
	   previously opened file descriptor, which will be set non-blocking by
	   this call. If successful, true is returned, false otherwise and errno
	   is set appropriately. */
	virtual bool Open(uint Fileno);

	/* Close() closes the associated file descriptor and releases all 
	   structures. Returns true on success and false otherwise, setting errno
	   appropriately. The object is in the closed state afterwards, even if
	   an error occured. */
	virtual bool Close(void);
	
	/* Unlink() unlinks (deletes) the associated file from the underlying 
	   filesystem. Returns true on success and false otherwise, setting errno
	   appropriately. The file must be opened by filename to use this. */
	virtual bool Unlink(void) const;

	/* Unlink() unlinks (deletes) the file referred to by Filename from the
	   underlying filesystem. Returns true on success and false otherwise, 
	   setting errno appropriately. */
	static  bool Unlink(const std::string &Filename);

	/* Size() returns the current size of the associated file. Returns the 
	   exact size of the file in bytes. Returns -1 on error, setting errno to
	   an appropriate value. */
	virtual ssize_t Size(void) const;

	/* Size() returns the current size of the file referred to by Filename.
	   Symbolic links are followed (the size of the link-target is returned).
	   Returns the exact size of the file in bytes. Returns -1 on error, 
	   setting errno to an appropriate value. */
	static  ssize_t Size(const std::string &Filename);
};

inline ssize_t cTBFile::SysRead(void *Buffer, size_t Length) const {
	return ::read(*this, Buffer, Length);
}

inline ssize_t cTBFile::SysWrite(const void *Buffer, size_t Length) const {
	return ::write(*this, Buffer, Length);
}


#endif // TOOLBOX_FILE_H
