#ifndef TOOLBOX_SHARED_H
#define TOOLBOX_SHARED_H

#include "tools/tools.h"

struct cSharedData {
private:
	uint  m_Length;
	uint  m_NumRefs;

public:
	static cSharedData *Construct (size_t Length);
	
	operator char * () { return this ? (char*)(this+1) : 0; }

	uint operator++ () { return ++m_NumRefs; }
	uint operator-- () { return --m_NumRefs; }

	size_t Size() const { return m_Length; }

	uint Refs () const { return m_NumRefs; }
};

class cTBShared {
private:
	cSharedData *m_Buffer;

protected:
	void Release();
	void Exclusive();
	void Allocate(size_t len, bool keep = false);
	
	char *Buffer() const { return m_Buffer ? (char*)*m_Buffer : (char*)0; }
	
public:
	cTBShared (void);
	cTBShared (const cTBShared &src);
	virtual ~cTBShared ();

	virtual void  Clear    ();
	virtual void  Set      (const cTBShared &src);
	
	virtual char *Buffer   (uint size);
	virtual void  Release  (uint newsize);

	cTBShared &operator= (const cTBShared &src) { Set(src); return *this; }

	operator const void * () const { return m_Buffer ? (const void*)*m_Buffer : (const void*)0; }
	operator void * () const { return m_Buffer ? (void*)*m_Buffer : (void*)0; }

	operator const char * () const { return m_Buffer ? (const char*)*m_Buffer : (const char*)0; }

	size_t Size() const { return m_Buffer ? m_Buffer->Size() : 0; }
	size_t Length() const { return m_Buffer ? m_Buffer->Size() : 0; }

	// friend cSource &operator>> (cSource &dest, cTBShared &str);
};

inline char *cTBShared::Buffer(uint size) {
	if ((!m_Buffer) || (m_Buffer->Refs() > 1) || (size > m_Buffer->Size()))
		Allocate(size, true);
	return Buffer();
}

#endif // TOOLBOX_SHARED_H
