#ifndef TOOLBOX_STRING_H
#define TOOLBOX_STRING_H

#include "tools/tools.h"
#include "tools/shared.h"
//#include "tools/source.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#ifdef TOOLBOX_REGEX
class cTBRegEx;
#endif

class cTBString: public cTBShared {
private:
	uint m_StringLen;
	
	/* Unhide and forbid baseclass method */
	virtual void  Set (const cTBShared &src) {}

public:
	cTBString ();
	cTBString (const cTBString &src);
	cTBString (const uchar *src);
	cTBString (const char *src);
	cTBString (char src);
	virtual ~cTBString ();

	static const cTBString Null;

	void Clear ();
	void Set   (const cTBString &String);
	void Set   (const uchar *String);
	void Set   (const char *String);
	void Set   (char Character);

	void Fill  (char Character, int Length = -1);

	void Release (uint newsize);

	cTBString &operator= (const cTBString &src) { Set(src); return *this; }
	cTBString &operator= (const char *src) 	{ Set(src); return *this; }
	cTBString &operator= (char src)			{ Set(src); return *this; }

	void Append (const cTBString &src);
	void Append (const char *src);
	void Append (char src);

	friend cTBString operator+ (const cTBString &a, const cTBString &b);
	friend cTBString operator+ (const cTBString &a, const char *b);
	friend cTBString operator+ (const char *a, const cTBString &b);
	friend cTBString operator+ (const cTBString &a, char b);
	friend cTBString operator+ (char a, const cTBString &b);

	friend cTBString &operator+= (cTBString &a, const cTBString &b);
	friend cTBString &operator+= (cTBString &a, const char *b);
	friend cTBString &operator+= (cTBString &a, char b);

	void Prepend (const cTBString &src);
	void Prepend (const char *src);
	void Prepend (char src);
	
	void Insert (uint Index, const cTBString &src);
	void Insert (uint Index, const char *src);
	void Insert (uint Index, char src);

	char At (uint i) const;
	char operator[] (int i) const { return At((uint)i); }

	char &At (uint i);
	char &operator[] (int i) { return At(i); }

	cTBString Left  (uint Count) const;
	cTBString Right (uint Count) const;
	cTBString Mid   (int idx, int Count = -1) const;

	int Find (const cTBString &String, uint Offset = 0) const;
	int Find (const char *String, uint Offset = 0) const;
	int Find (char Character, uint Offset = 0) const;
#ifdef TOOLBOX_REGEX
	bool Find (cTBRegEx &Regex, uint Offset = 0) const;
#endif

	void Format (const char *fmt, ...)
#if defined(__GNUC__)
		__attribute__ ((format (printf, 2, 3)))
#endif
		;
	void Format (const cTBString &fmt, ...);

	typedef int(*TOFUNC)(int);
	template<TOFUNC F> cTBString ToAnything(void) const;

	cTBString ToUpper (void) const { return ToAnything<toupper>(); }
	cTBString ToLower (void) const { return ToAnything<tolower>(); }

	typedef int(*ISFUNC)(int);
	template<ISFUNC F> bool IsAnything(void) const;

	bool IsAlnum(void) const { return IsAnything<isalnum>(); }
	bool IsAlpha(void) const { return IsAnything<isalpha>(); }
	bool IsAscii(void) const { return IsAnything<isascii>(); }
	bool IsCntrl(void) const { return IsAnything<iscntrl>(); }
	bool IsDigit(void) const { return IsAnything<isdigit>(); }
	bool IsGraph(void) const { return IsAnything<isgraph>(); }
	bool IsLower(void) const { return IsAnything<islower>(); }
	bool IsPrint(void) const { return IsAnything<isprint>(); }
	bool IsPunct(void) const { return IsAnything<ispunct>(); }
	bool IsSpace(void) const { return IsAnything<isspace>(); }
	bool IsUpper(void) const { return IsAnything<isupper>(); }
	bool IsXdigit(void) const { return IsAnything<isxdigit>(); }
	
#if defined(_GNU_SOURCE)		
	bool IsBlank(void) const { return IsAnything<isblank>(); }
#endif

	uint Length  (void) const { return m_StringLen; }
	bool IsEmpty (void) const { return m_StringLen == 0; }
	bool IsNull  (void) const { return Buffer() == 0; }

	short ToShort(bool *Ok = NULL) const;
	ushort ToUShort(bool *Ok = NULL) const;
	int ToInt(bool *Ok = NULL) const;
	uint ToUInt(bool *Ok = NULL) const;
	long ToLong(bool *Ok = NULL) const;
	ulong ToULong(bool *Ok = NULL) const;
	float ToFloat(bool *Ok = NULL) const;
	double ToDouble(bool *Ok = NULL) const;

	static cTBString Number(short Num);
	static cTBString Number(ushort Num);
	static cTBString Number(int Num);
	static cTBString Number(uint Num);
	static cTBString Number(long Num);
	static cTBString Number(ulong Num);
	static cTBString Number(float Num);
	static cTBString Number(double Num);

	friend bool operator== (const cTBString &str1, const cTBString &str2);
	friend bool operator== (const cTBString &str1, const char *str2);
	friend bool operator== (const char *str1, const cTBString &str2);

	friend bool operator!= (const cTBString &str1, const cTBString &str2);
	friend bool operator!= (const cTBString &str1, const char *str2);
	friend bool operator!= (const char *str1, const cTBString &str2);

	friend bool operator< (const cTBString &str1, const cTBString &str2);
	friend bool operator< (const cTBString &str1, const char *str2);
	friend bool operator< (const char *str1, const cTBString &str2);
	
	friend bool operator> (const cTBString &str1, const cTBString &str2);
	friend bool operator> (const cTBString &str1, const char *str2);
	friend bool operator> (const char *str1, const cTBString &str2);
	
	friend bool operator<= (const cTBString &str1, const cTBString &str2);
	friend bool operator<= (const cTBString &str1, const char *str2);
	friend bool operator<= (const char *str1, const cTBString &str2);
	
	friend bool operator>= (const cTBString &str1, const cTBString &str2);
	friend bool operator>= (const cTBString &str1, const char *str2);
	friend bool operator>= (const char *str1, const cTBString &str2);
};

inline char cTBString::At(uint idx) const {
	ASSERT(idx >= m_StringLen);
	return Buffer() ? Buffer()[idx] : 0;
}

inline char &cTBString::At(uint idx) {
	static char null = 0;
	ASSERT(idx >= m_StringLen);
	if (Buffer()) {
		Exclusive();
		return Buffer()[idx];
	} else
		return (null = 0);
}

inline 
RETURNS(cTBString, operator+(const cTBString &a, const cTBString &b), ret(a))
	ret.Append(b);
RETURN(ret)

inline 
RETURNS(cTBString, operator+ (const cTBString &a, const char *b), ret(a))
	ret.Append(b);
RETURN(ret)

inline 
RETURNS(cTBString, operator+ (const char *a, const cTBString &b), ret(a))
	ret.Append(b);
RETURN(ret)

inline 
RETURNS(cTBString, operator+ (const cTBString &a, char b), ret(a))
	ret.Append(b);
RETURN(ret)

inline 
RETURNS(cTBString, operator+ (char a, const cTBString &b), ret(a))
	ret.Append(b);
RETURN(ret)

inline cTBString &operator+= (cTBString &a, const cTBString &b) {
	a.Append(b);
	return a;
}

inline cTBString &operator+= (cTBString &a, const char *b) {
	a.Append(b);
	return a;
}

inline cTBString &operator+= (cTBString &a, char b) {
	a.Append(b);
	return a;
}

inline bool operator== (const cTBString &str1, const cTBString &str2) {
	if (str1.Length() != str2.Length())
		return false;
	return memcmp(str1.Buffer(), str2.Buffer(), str1.Length()) == 0;
}

inline bool operator== (const cTBString &str1, const char *str2) {
	uint len = strlen(str2);
	if (str1.Length() != len)
		return false;
	return memcmp(str1.Buffer(), str2, len) == 0;
}

inline bool operator== (const char *str1, const cTBString &str2) {
	uint len = strlen(str1);
	if (len != str2.Length())
		return false;
	return memcmp(str1, str2.Buffer(), len) == 0;
}

inline bool operator!= (const cTBString &str1, const cTBString &str2) {
	if (str1.Length() != str2.Length())
		return true;
	return memcmp(str1.Buffer(), str2.Buffer(), str1.Length()) != 0;
}

inline bool operator!= (const cTBString &str1, const char *str2) {
	uint len = strlen(str2);
	if (str1.Length() != len)
		return true;
	return memcmp(str1.Buffer(), str2, len) != 0;
}

inline bool operator!= (const char *str1, const cTBString &str2) {
	uint len = strlen(str1);
	if (len != str2.Length())
		return true;
	return memcmp(str1, str2.Buffer(), len) != 0;
}

inline bool operator< (const cTBString &str1, const cTBString &str2) {
	int ret = memcmp(str1.Buffer(), str2.Buffer(), str1.Length() < str2.Length() ? str1.Length() : str2.Length());
	if ((ret < 0) || ((ret == 0) && (str1.Length() < str2.Length())))
		return true;
	return false;
}

inline bool operator< (const cTBString &str1, const char *str2) {
	uint len = strlen(str2);
	int ret = memcmp(str1.Buffer(), str2, str1.Length() < len ? str1.Length() : len);
	if ((ret < 0) || ((ret == 0) && (str1.Length() < len)))
		return true;
	return false;
}

inline bool operator< (const char *str1, const cTBString &str2) {
	uint len = strlen(str1);
	int ret = memcmp(str1, str2.Buffer(), len < str2.Length() ? len : str2.Length());
	if ((ret < 0) || ((ret == 0) && (len < str2.Length())))
		return true;
	return false;
}

inline bool operator> (const cTBString &str1, const cTBString &str2) {
	int ret = memcmp(str1.Buffer(), str2.Buffer(), str1.Length() < str2.Length() ? str1.Length() : str2.Length());
	if ((ret > 0) || ((ret == 0) && (str1.Length() < str2.Length())))
		return true;
	return false;
}

inline bool operator> (const cTBString &str1, const char *str2) {
	uint len = strlen(str2);
	int ret = memcmp(str1.Buffer(), str2, str1.Length() < len ? str1.Length() : len);
	if ((ret > 0) || ((ret == 0) && (str1.Length() < len)))
		return true;
	return false;
}

inline bool operator> (const char *str1, const cTBString &str2) {
	uint len = strlen(str1);
	int ret = memcmp(str1, str2.Buffer(), len < str2.Length() ? len : str2.Length());
	if ((ret > 0) || ((ret == 0) && (len < str2.Length())))
		return true;
	return false;
}

inline bool operator<= (const cTBString &str1, const cTBString &str2) {
	int ret = memcmp(str1.Buffer(), str2.Buffer(), str1.Length() < str2.Length() ? str1.Length() : str2.Length());
	if ((ret < 0) || ((ret == 0) && (str1.Length() <= str2.Length())))
		return true;
	return false;
}

inline bool operator<= (const cTBString &str1, const char *str2) {
	uint len = strlen(str2);
	int ret = memcmp(str1.Buffer(), str2, str1.Length() < len ? str1.Length() : len);
	if ((ret < 0) || ((ret == 0) && (str1.Length() <= len)))
		return true;
	return false;
}

inline bool operator<= (const char *str1, const cTBString &str2) {
	uint len = strlen(str1);
	int ret = memcmp(str1, str2.Buffer(), len < str2.Length() ? len : str2.Length());
	if ((ret < 0) || ((ret == 0) && (len <= str2.Length())))
		return true;
	return false;
}

inline bool operator>= (const cTBString &str1, const cTBString &str2) {
	int ret = memcmp(str1.Buffer(), str2.Buffer(), str1.Length() < str2.Length() ? str1.Length() : str2.Length());
	if ((ret > 0) || ((ret == 0) && (str1.Length() >= str2.Length())))
		return true;
	return false;
}

inline bool operator>= (const cTBString &str1, const char *str2) {
	uint len = strlen(str2);
	int ret = memcmp(str1.Buffer(), str2, str1.Length() < len ? str1.Length() : len);
	if ((ret > 0) || ((ret == 0) && (str1.Length() >= len)))
		return true;
	return false;
}

inline bool operator>= (const char *str1, const cTBString &str2) {
	uint len = strlen(str1);
	int ret = memcmp(str1, str2.Buffer(), len < str2.Length() ? len : str2.Length());
	if ((ret > 0) || ((ret == 0) && (len >= str2.Length())))
		return true;
	return false;
}
		
#endif // TOOLBOX_STRING_H
