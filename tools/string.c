#include "tools/string.h"
#ifdef TOOLBOX_REGEX
#	include "tools/regex.h"
#endif

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <errno.h>

const cTBString cTBString::Null;

cTBString::cTBString(): 
		cTBShared(),
		m_StringLen(0) {
}

cTBString::cTBString(const cTBString &src): 
		cTBShared(src),
		m_StringLen(src.m_StringLen) {
}

cTBString::cTBString(const char *src) {
	Set(src);
}

cTBString::cTBString(const uchar *src) {
	Set(src);
}

cTBString::cTBString(char src) {
	Set(src);
}

cTBString::~cTBString () {
}

void cTBString::Release(uint newsize) {
	m_StringLen = newsize;
	cTBShared::Release(m_StringLen + 1);
	Buffer()[m_StringLen] = 0;
}

void cTBString::Clear() {
	cTBShared::Clear();
	m_StringLen = 0;
}

void cTBString::Set(const cTBString &String) {
	cTBShared::Set(String);
	m_StringLen = String.m_StringLen;
}

void cTBString::Set (const char *String) {
	m_StringLen = strlen(String);
	Allocate(m_StringLen + 1);

	memcpy(Buffer(), String, m_StringLen);
	Buffer()[m_StringLen] = 0;
}

void cTBString::Set (const uchar *String) {
	Set((const char*)String);
}

void cTBString::Set (char Character) {
	m_StringLen = 1;
	Allocate(m_StringLen + 1);

	Buffer()[0] = Character;
	Buffer()[1] = 0;
}

void cTBString::Fill(char Character, int Length) {
	if (Length != -1) {
		m_StringLen = Length;
		Allocate(m_StringLen + 1);
	}
	memset(Buffer(), Character, m_StringLen);
	Buffer()[m_StringLen] = 0;
}

void cTBString::Append(const cTBString &src) {
	Allocate(m_StringLen + src.m_StringLen + 1, true);

	memcpy(Buffer() + m_StringLen, src.Buffer(), src.m_StringLen);
	m_StringLen += src.m_StringLen;
	Buffer()[m_StringLen] = 0;
}

void cTBString::Append(const char *src) {
	uint len = strlen(src);
	Allocate(m_StringLen + len + 1, true);

	memcpy(Buffer() + m_StringLen, src, len);
	m_StringLen += len;
	Buffer()[m_StringLen] = 0;
}

void cTBString::Append(char src) {
	Allocate(m_StringLen + 2, true);

	Buffer()[m_StringLen] = src;
	++m_StringLen;
	Buffer()[m_StringLen] = 0;
}

void cTBString::Prepend(const cTBString &src) {
	Allocate(m_StringLen + src.m_StringLen + 1, true);

	memmove(Buffer() + src.m_StringLen, Buffer(), m_StringLen);
	memcpy(Buffer(), src.Buffer(), src.m_StringLen);
	m_StringLen += src.m_StringLen;
	Buffer()[m_StringLen] = 0;
}

void cTBString::Prepend(const char *src) {
	uint len = strlen(src);
	Allocate(m_StringLen + len + 1, true);

	memmove(Buffer() + len, Buffer(), m_StringLen);
	memcpy(Buffer(), src, len);
	m_StringLen += len;
	Buffer()[m_StringLen] = 0;
}

void cTBString::Prepend(char src) {
	Allocate(m_StringLen + 2, true);

	memmove(Buffer() + 1, Buffer(), m_StringLen);
	Buffer()[0] = src;
	Buffer()[++m_StringLen] = 0;
}

void cTBString::Insert(uint Index, const cTBString &String) {
	Allocate(m_StringLen + String.m_StringLen + 1, true);

	memmove(Buffer() + Index + String.m_StringLen, Buffer() + Index, m_StringLen - Index);
	memcpy(Buffer() + Index, String.Buffer(), String.m_StringLen);
	m_StringLen += String.m_StringLen;
	Buffer()[m_StringLen] = 0;
}

void cTBString::Insert(uint Index, const char *String) {
	uint len = strlen(String);
	Allocate(m_StringLen + len + 1, true);

	memmove(Buffer() + Index + len, Buffer() + Index, m_StringLen - Index);
	memcpy(Buffer() + Index, String, len);
	m_StringLen += len;
	Buffer()[m_StringLen] = 0;
}

void cTBString::Insert(uint Index, char Character) {
	Allocate(m_StringLen + 2, true);

	memmove(Buffer() + Index + 1, Buffer() + Index, m_StringLen - Index);
	Buffer()[Index] = Character;
	Buffer()[++m_StringLen] = 0;
}

RETURNS(cTBString, cTBString::Left(uint count) const, ret)
	if (count > m_StringLen)
		count = m_StringLen;

	ret.Allocate(count + 1);
	memcpy(ret.Buffer(), Buffer(), count);
	ret.Buffer()[count] = 0;
	ret.m_StringLen = count;
RETURN(ret)

RETURNS(cTBString, cTBString::Right(uint count) const, ret)
	if (count > m_StringLen)
		count = m_StringLen;

	ret.Allocate(count + 1);
	memcpy(ret.Buffer(), Buffer() + m_StringLen - count, count);
	ret.Buffer()[count] = 0;
	ret.m_StringLen = count;
RETURN(ret)

RETURNS(cTBString, cTBString::Mid(int idx, int count) const, ret)
	if (idx < 0)
		idx = m_StringLen + idx;

	if ((count < 0) || (count > (int)m_StringLen - idx))
		count = m_StringLen - idx;

	ret.Allocate(count + 1);
	memcpy(ret.Buffer(), Buffer() + idx, count);
	ret.Buffer()[count] = 0;
	ret.m_StringLen = count;
RETURN(ret)

int cTBString::Find (const cTBString &String, uint Offset) const {
	if (Offset >= m_StringLen)
		return -1;

	char *pos = strstr(Buffer() + Offset, String.Buffer());
	if (pos) return (pos - Buffer());
	else     return -1;
}

int cTBString::Find (const char *String, uint Offset) const {
	if (Offset >= m_StringLen)
		return -1;

	char *pos = strstr(Buffer() + Offset, String);
	if (pos) return (pos - Buffer());
	else     return -1;
}

int cTBString::Find (char Character, uint Offset) const {
	if (Offset >= m_StringLen)
		return -1;

	char *pos = strchr(Buffer() + Offset, Character);
	if (pos) return (pos - Buffer());
	else     return -1;
}

#ifdef TOOLBOX_REGEX
bool cTBString::Find (cTBRegEx &Regex, uint Offset) const {
	return Regex.Match(Buffer(), Offset);
}
#endif

void cTBString::Format (const char *fmt, ...) {
	int n, size = 128;
	va_list ap;

	char *buf = Buffer(size);

	while (1) {
		va_start(ap, fmt);
		n = vsnprintf(buf, size, fmt, ap);
		va_end(ap);

		if ((n > -1) && (n < size))
			break;

		if (n > -1)
			size = n + 1;
		else
			size *= 2;

		buf = Buffer(size);
	}
	Release(n);
}

void cTBString::Format(const cTBString &fmt, ...) {
	int n, size = 128;
	va_list ap;

	char *buf = Buffer(size);

	while (1) {
		va_start(ap, &fmt);
		n = vsnprintf(buf, size, fmt, ap);
		va_end(ap);

		if ((n > -1) && (n < size))
			break;

		if (n > -1)
			size = n + 1;
		else
			size *= 2;

		buf = Buffer(size);
	}
	Release(n);
}

template<cTBString::TOFUNC F>
cTBString cTBString::ToAnything(void) const {
	const char *src;
	char *dest;
	cTBString ret;

	src = Buffer();
	dest = ret.Buffer(m_StringLen + 1);

	for (; src < Buffer() + m_StringLen; ++src, ++dest)
		*dest = F(*src);

	*dest = '\0';

	ret.Release(m_StringLen);
	return ret;
}

template<cTBString::ISFUNC F>
bool cTBString::IsAnything(void) const {
	const char *ptr = Buffer();

	for (; ptr < Buffer() + m_StringLen; ++ptr)
		if (!F(*ptr)) return false;

	return true;
}

short cTBString::ToShort(bool *Ok) const {
	long ret;
	char *endptr;
	bool res = false;

	ret = strtol(Buffer(), &endptr, 0);

	if (!IsEmpty() && *endptr == '\0' && ret >= SHRT_MIN && ret <= SHRT_MAX)
		res = true;

	if (Ok) *Ok = res;
	return (short)ret;
}

ushort cTBString::ToUShort(bool *Ok) const {
	ulong ret;
	char *endptr;
	bool res = false;

	ret = strtoul(Buffer(), &endptr, 0);

	if (!IsEmpty() && *endptr == '\0' && ret <= USHRT_MAX)
		res = true;

	if (Ok) *Ok = res;
	return (ushort)ret;
}

int cTBString::ToInt(bool *Ok) const {
	long ret;
	char *endptr;
	bool res = false;

	ret = strtol(Buffer(), &endptr, 0);

	if (!IsEmpty() && *endptr == '\0' && ret >= INT_MIN && ret <= INT_MAX)
		res = true;

	if (Ok) *Ok = res;
	return (int)ret;
}

uint cTBString::ToUInt(bool *Ok) const {
	ulong ret;
	char *endptr;
	bool res = false;

	ret = strtoul(Buffer(), &endptr, 0);

	if (!IsEmpty() && *endptr == '\0' && ret <= UINT_MAX)
		res = true;

	if (Ok) *Ok = res;
	return (uint)ret;
}

long cTBString::ToLong(bool *Ok) const {
	long ret;
	char *endptr;
	bool res = false;

	errno = 0;
	ret = strtol(Buffer(), &endptr, 0);

	if (!IsEmpty() && *endptr == '\0' && errno != ERANGE)
		res = true;

	if (Ok) *Ok = res;
	return (long)ret;
}

ulong cTBString::ToULong(bool *Ok) const {
	ulong ret;
	char *endptr;
	bool res = false;

	errno = 0;
	ret = strtoul(Buffer(), &endptr, 0);

	if (!IsEmpty() && *endptr == '\0' && errno != ERANGE)
		res = true;

	if (Ok) *Ok = res;
	return (ulong)ret;
}

float cTBString::ToFloat(bool *Ok) const {
	double ret;
	char *endptr;
	bool res = false;

	ret = strtod(Buffer(), &endptr);

	if (!IsEmpty() && *endptr == '\0' && errno != ERANGE)
		res = true;

	if (Ok) *Ok = res;
	return (float)ret;
}

double cTBString::ToDouble(bool *Ok) const {
	double ret;
	char *endptr;
	bool res = false;

	errno = 0;
	ret = strtol(Buffer(), &endptr, 0);
	
	if (!IsEmpty() && *endptr == '\0' && errno != ERANGE)
		res = true;

	if (Ok) *Ok = res;
	return (double)ret;
}

RETURNS(cTBString, cTBString::Number(short Num), ret)
	ret.Format("%hd", Num);
RETURN(ret)

RETURNS(cTBString, cTBString::Number(ushort Num), ret)
	ret.Format("%hu", Num);
RETURN(ret)

RETURNS(cTBString, cTBString::Number(int Num), ret)
	ret.Format("%d", Num);
RETURN(ret)

RETURNS(cTBString, cTBString::Number(uint Num), ret)
	ret.Format("%u", Num);
RETURN(ret)

RETURNS(cTBString, cTBString::Number(long Num), ret)
	ret.Format("%ld", Num);
RETURN(ret)

RETURNS(cTBString, cTBString::Number(ulong Num), ret)
	ret.Format("%lu", Num);
RETURN(ret)

RETURNS(cTBString, cTBString::Number(float Num), ret)
	ret.Format("%f", Num);
RETURN(ret)

RETURNS(cTBString, cTBString::Number(double Num), ret)
	ret.Format("%f", Num);
RETURN(ret)

