#pragma once

#include "AFLibIOGlobal.h"

class COleVariant;

/////////////////////////////////////////////////////////////////////////////
// TStringBufferGeneric
// String that only allows +=; does fewer reallocations.
// May be either Ascii or Unicode.

namespace AFLibIO {
class AFLIB TStringBufferGeneric
{
private:
    LPBYTE m_pBuffer;  // the actual buffer
    int m_charShift;   // character length log-base-2 (0 or 1)
    int m_nLength;     // length of the string
    int m_nAlloc;      // allocated length
    int m_nGrow;       // buffer growth increments

    // compute m_nAlloc from thelength and m_nGrow
    void ComputeAllocSize(int nLength);

    // get address of index-th character
    LPVOID GetData(int index)
    {
        return m_pBuffer + (index << m_charShift);
    }

    // get address of index-th character (const version)
    LPCVOID GetData(int index) const
    {
        return const_cast<TStringBufferGeneric*>(this)->GetData(index);
    }

    // convert pointer to index
    int GetPosition(LPCVOID ss) const;

    // put the character at m_nLength
    void SetChar(int pos, WCHAR ch);

    // zero out one character at m_nLength
    void Terminate()
    {
        SetChar(m_nLength, 0);
    }

    // put the character at m_nLength, increment m_nLength
    void PutChar(WCHAR ch)
    {
        SetChar(m_nLength, ch);
        ++m_nLength;
    }

    // allocate memory for m_pBuffer
    void Allocate(LPCVOID str);

    // check if additional data fits the buffer; reallocate if necessary
    void Reallocate(int incLen);

    // copy the structure
    void PrivateCopy(const TStringBufferGeneric& sb);

    // append the buffer
    void PrivateAppend(LPCVOID str, int charShift, int len);

    // helper for AppendDouble() and AppendDoubleStrict()
    void PrivateAppendDouble(double d, int precision, bool bForceSign, bool bStrict, bool bPercent);

    // private and undefined operators to prohibit simple addition
    TStringBufferGeneric operator+(LPCSTR str) const;
    TStringBufferGeneric operator+(LPCWSTR str) const;
    TStringBufferGeneric operator+(const CStringA& s) const;
    TStringBufferGeneric operator+(const CStringW& s) const;
    TStringBufferGeneric operator+(const TStringBufferGeneric& sb) const;

protected:
    explicit TStringBufferGeneric(int charShift);
    TStringBufferGeneric(const TStringBufferGeneric& sb);
    TStringBufferGeneric(int charShift, int nGrow);

    // access to the buffer
    LPCVOID ProtectedGetBuffer() const
    {
        return m_pBuffer;
    }

    // "safe" version of PutChar()
    void ProtectedPutChar(WCHAR ch)
    {
        Reallocate(1);
        PutChar(ch);
        Terminate();
    }

    // "safe" version of SetChar()
    void ProtectedSetChar(int pos, WCHAR ch)
    {
        ASSERT(pos < m_nLength);
        SetChar(pos, ch);
    }

    // delete existing data, copy new
    void ProtectedCopy(const TStringBufferGeneric& sb);

    // append/fill the buffer with the character
    void ProtectedFill(int ch, int count);

    // find character in the buffer
    int ProtectedFind(int ch, int pos) const;

    // find substring in the buffer
    int ProtectedFind(LPCVOID str, int pos) const;

    // find first occurrence of one of the characters
    int ProtectedFindOneOf(LPCVOID str, int pos) const;

    // compare to a string
    int ProtectedCompare(LPCVOID str) const;

public:
    // suggested buffer size for file output
    static const int FileBufSize = 1 << 16;

    ~TStringBufferGeneric();

    // clear the buffer, initialize buffer length
    void Clear();

    // set length to null, leave the buffer allocated
    void Empty();

    // set grow-by parameter for the buffer
    void SetGrowBy(int growBy);

    // delete one or more characters
    void Delete(int pos, int count);

    // append another buffer, convert A-W if necessary
    void Append(const TStringBufferGeneric& sb)
    {
        PrivateAppend(sb.m_pBuffer, sb.m_charShift, sb.m_nLength);
    }

    // add Ascii string with given number of characters
    void Append(LPCSTR str, int length)
    {
        PrivateAppend(str, 0, length);
    }

    // add Unicode string with given number of characters
    void Append(LPCWSTR str, int length)
    {
        PrivateAppend(str, 1, length);
    }

    // add Ascii string with given number of characters
    void Append(const CStringA& s)
    {
        PrivateAppend(LPCSTR(s), 0, s.GetLength());
    }

    // add Unicode string with given number of characters
    void Append(const CStringW& s)
    {
        PrivateAppend(LPCWSTR(s), 1, s.GetLength());
    }

    // add Ascii string
    void Append(LPCSTR str);

    // add Unicode string
    void Append(LPCWSTR str);

    // append an OLE String
    void AppendBstr(BSTR str);

    // append an integer
    void AppendInt(INT64 a, int digits = 0);

    // append an unsigned integer
    void AppendUint(UINT64 a, int digits = 0);

    // append an unsigned integer in hexadecimal form
    void AppendHex(UINT64 a, int digits);

    // append a double; cut trailing zeros
    void AppendDouble(double d, int precision = 6, bool bForceSign = false)
    {
        PrivateAppendDouble(d, precision, bForceSign, false, false);
    }

    // append a double; do NOT cut trailing zeros
    void AppendDoubleStrict(double d, int precision = 6, bool bForceSign = false)
    {
        PrivateAppendDouble(d, precision, bForceSign, true, false);
    }

    // append a percentage; cut trailing zeros
    void AppendPercent(double d, int precision = 2, bool bForceSign = false)
    {
        PrivateAppendDouble(d * 100, precision, bForceSign, false, true);
    }

    // append a percentage; do NOT cut trailing zeros
    void AppendPercentStrict(double d, int precision = 2, bool bForceSign = false)
    {
        PrivateAppendDouble(d * 100, precision, bForceSign, true, true);
    }

    // append an OLE variant
    void AppendVariant(const COleVariant& var);

    // read from file (append)
    int Read(CFile& Fin, int length, TTypeTextFile ttf = ttfAscii);

    // write to a file
    void Write(CFile& Fout, TTypeTextFile ttf = ttfAscii) const;

    // if length over m_nGrow, write to file and empty
    void WriteAndEmpty(CFile& Fout, TTypeTextFile ttf = ttfAscii);

    // is this buffer Unicode (as opposed to Ascii)?
    bool IsUnicode() const
    {
        return m_charShift == 1;
    }

    // is the buffer empty?
    bool IsEmpty() const
    {
        return m_nLength <= 0;
    }

    // get the length of the object
    int GetLength() const
    {
        return m_nLength;
    }

    // get the buffer growth parameter
    int GetGrowBy() const
    {
        return m_nGrow;
    }
};
}  // namespace AFLibIO
