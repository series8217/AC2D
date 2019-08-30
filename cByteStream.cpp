#include "StdAfx.h"
#include ".\cbytestream.h"

cByteStream::cByteStream(void)
{
	m_pbDataStart = NULL;
	m_pbDataPtr = NULL;
	m_dwLength = NULL;
}

cByteStream::~cByteStream(void)
{
}

cByteStream::cByteStream(BYTE *Data, DWORD Length)
{
	SetStream(Data, Length);
}

void cByteStream::SetStream(BYTE *Data, DWORD Length)
{
	m_pbDataPtr = m_pbDataStart = Data;
    m_dwLength = Length;
}

DWORD cByteStream::GetOffset()
{
    return (DWORD) (m_pbDataPtr - m_pbDataStart);
}

bool cByteStream::AtEOF()
{
    return (GetOffset() >= m_dwLength);
}

void cByteStream::ReadBegin( void )
{
	m_pbDataPtr = m_pbDataStart;
}

// Align to next DWORD boundary?
void cByteStream::ReadAlign( void )
{
	long iOffset = m_pbDataPtr - m_pbDataStart;
	long alignDelta = iOffset % 4;
	if (alignDelta != 0) {
		m_pbDataPtr += (int)(4 - alignDelta);
	}
}

void cByteStream::ReadSkip(int iAmount)
{
	m_pbDataPtr += iAmount;
}

//Ew, =)
BYTE	cByteStream::ReadByte()		{
	BYTE Result = *((BYTE *)m_pbDataPtr);
	m_pbDataPtr += sizeof(BYTE);
	return Result;
}
WORD	cByteStream::ReadWORD()		{
	WORD Result = *((WORD *)m_pbDataPtr);
	m_pbDataPtr += sizeof(WORD);
	return Result;
}
DWORD	cByteStream::ReadDWORD()	{
	DWORD Result = *((DWORD *)m_pbDataPtr);
	m_pbDataPtr += sizeof(DWORD);
	return Result;
}
QWORD	cByteStream::ReadQWORD()	{
	QWORD Result = *((QWORD *)m_pbDataPtr);
	m_pbDataPtr += sizeof(QWORD);
	return Result;
}
WORD	cByteStream::ReadPackedWORD()	{
	WORD Result = *((BYTE *)m_pbDataPtr);
	m_pbDataPtr += sizeof(BYTE);
	if (Result & 0x80)
	{
		Result <<= 8;
		Result &= 0x7FFF;
		Result |= *((BYTE *)m_pbDataPtr);
		m_pbDataPtr += sizeof(BYTE);
	}
	return Result;
}
DWORD	cByteStream::ReadPackedDWORD()	{
	DWORD Result = *((WORD *)m_pbDataPtr);
	m_pbDataPtr += sizeof(WORD);
	if (Result & 0x8000)
	{
		Result <<= 16;
		Result &= 0x7FFFFFFF;
		Result |= *((WORD *)m_pbDataPtr);
		m_pbDataPtr += sizeof(WORD);
	}
	return Result;
}
float	cByteStream::ReadFloat()		{
	float Result = *((float *)m_pbDataPtr);
	m_pbDataPtr += sizeof(float);
	return Result;
}
double	cByteStream::ReadDouble()	{
	double Result = *((double *)m_pbDataPtr);
	m_pbDataPtr += sizeof(double);
	return Result;
}
char*	cByteStream::ReadString()	{
	WORD wStrLen = ReadWORD();
	char* Result = new char[wStrLen+1];
	Result[wStrLen] = 0;

	memcpy(Result, m_pbDataPtr, wStrLen);
	m_pbDataPtr += wStrLen;

	ReadAlign();
	return Result;
}

wchar_t* cByteStream::ReadWString()	{
	WORD wStrLen = ReadWORD();
	wchar_t* Result = new wchar_t[(2*wStrLen)+1];
	Result[wStrLen] = 0;

	memcpy(Result, m_pbDataPtr, 2*wStrLen);
	m_pbDataPtr += wStrLen*2;

	ReadAlign();
	return Result;
}

char * cByteStream::ReadEncodedString()
{
	WORD wLength = ReadWORD();
	WORD wCount = 0;

	char *szTemp = new char[ wLength + 4 ];
	memset( szTemp, 0, wLength + 4 );

	long lTell = (long) (m_pbDataPtr - m_pbDataStart);

	if( lTell % 4 )
	{
		memcpy(szTemp, ReadGroup(2), 2);
		wCount += 2;
	}

	for(; wCount < wLength; wCount += 4)
		memcpy(szTemp + wCount, ReadGroup(4), 4);

	//if( bNibbled )
	if (true)
	{
		char *temp;
		for( temp = szTemp; *temp; ++temp )
		{
			WORD c = *(BYTE *) temp;
			WORD c2 = c >> 4;
			c <<= 4;
			*temp = (c | c2);
		}
	}

	return szTemp;
}


BYTE *	cByteStream::ReadGroup(int iAmount)
{
	BYTE * Result = m_pbDataPtr;
	m_pbDataPtr += iAmount;
	return Result;
}
