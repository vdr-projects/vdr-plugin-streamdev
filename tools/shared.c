#include "tools/shared.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

cSharedData *cSharedData::Construct (size_t Length) {
	size_t reallength = sizeof(cSharedData) + Length;
	cSharedData *ret = (cSharedData*)new char[reallength];

	ret->m_Length = Length;
	ret->m_NumRefs = 0;
	return ret;
}

cTBShared::cTBShared(void) {
	m_Buffer = NULL;
}

cTBShared::cTBShared (const cTBShared &src) {
	m_Buffer = src.m_Buffer;
	if (m_Buffer)
		++*m_Buffer;
}

cTBShared::~cTBShared () {
	if (m_Buffer)
		Release();
}

void cTBShared::Clear () {
	if (m_Buffer)
		Release();
	m_Buffer = 0;
}

void cTBShared::Set (const cTBShared &src) {
	if (m_Buffer)
		Release();

	m_Buffer = src.m_Buffer;
	if (m_Buffer)
		++*m_Buffer;
}

void cTBShared::Release () {
	CHECK_PTR(m_Buffer);

	if (--*m_Buffer == 0)
		delete[] (char*)m_Buffer;

	m_Buffer = 0;
}

void cTBShared::Release(uint newsize) {
	CHECK_PTR(m_Buffer);

	Allocate(newsize, true);
}

void cTBShared::Exclusive () {
	CHECK_PTR(m_Buffer);

	if (m_Buffer->Refs() == 1)
		return;

	cSharedData *copy = cSharedData::Construct(m_Buffer->Size());
	memcpy(*copy, *m_Buffer, m_Buffer->Size());

	Release();

	m_Buffer = copy;
	++*m_Buffer;
}

void cTBShared::Allocate (size_t len, bool keep /* = false */) {
	if (m_Buffer && (m_Buffer->Refs() == 1) && (m_Buffer->Size() == len))
		return;

	cSharedData *newBuffer = cSharedData::Construct(len);
	if (m_Buffer) {
		if (keep)
			memcpy(*newBuffer, *m_Buffer, len < m_Buffer->Size() ? len : m_Buffer->Size());

		Release();
	}
	m_Buffer = newBuffer;
	++*m_Buffer;
}

