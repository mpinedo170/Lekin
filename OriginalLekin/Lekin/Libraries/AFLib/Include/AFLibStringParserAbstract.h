#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TStringParserAbstract
// Parses a string.
// Parent for TStringTokenizerAbstract and TStringParserGeneral.

namespace AFLibIO {
class AFLIB TStringParserAbstract
{
private:
    DEFINE_COPY_AND_ASSIGN(TStringParserAbstract);

    LPCTSTR m_sInitial;  // string to be parsed
    LPCTSTR m_sPtr;      // current pointer
    int m_length;        // length of the input string
    bool m_bIgnoreCase;  // ignore character case in PrivateTestSymbol

    // test if m_sPtr has reached the end; set it to NULL if yes
    void TestEof();

    // read unsigned int or hex
    UINT64 PrivateReadUint(UINT idsType, int mul, UINT64 iMax);

    // build a string for case-insensitive FindOneOf
    void BuildCaseInsensitive(LPCTSTR sSrc, TStringBuffer& sDest);

protected:
    // "Protected" functions can be called from PostRead()

    // reads a "plus" or "minus" sign; returns true if negative
    bool ProtectedReadNegative();

    // read signed 64-bit integer
    INT64 ProtectedReadInt64();

    // read unsigned 64-bit integer
    UINT64 ProtectedReadUint64();

    // read hexadecimal 64-bit integer
    UINT64 ProtectedReadHex64();

    // read a floating-point number; optionally, may allow commas in integer part
    double ProtectedReadDouble(bool bAllowCommas);

    // read the exact number of characters
    void ProtectedReadSegment(int count, TStringBuffer& sDest);

    // return the segment from m_sPtr to ssEnd
    void ProtectedReadSegment(LPCTSTR ssEnd, int nSkip, TStringBuffer& sDest);

    // read string into buffer(MUST be quoted)
    void ProtectedReadQuotedStr(TStringBuffer& sDest);

    // extract up to a delimiter character
    void ProtectedFindFirstCh(bool bSkip, TCHAR ch, TStringBuffer& sDest);

    // extract up to any of the delimiter strings
    void ProtectedFindFirst(bool bSkip, int count, const LPCTSTR arrStr[], TStringBuffer& sDest);

    // read until a char from strDelim
    void ProtectedFindFirstDelim(LPCTSTR strDelim, TStringBuffer& sDest);

    // find first symbol NOT from strColl
    void ProtectedFindFirstNot(LPCTSTR strColl, TStringBuffer& sDest);

    // test if sPtr starts with str, move pointer if yes
    bool ProtectedTestSymbol(LPCTSTR str);

    // same, but do not move the pointer
    bool ProtectedTestSymbolNoMove(LPCTSTR str);

    // test symbol, throw an exception if not matched
    void ProtectedTestSymbolHard(LPCTSTR str);

    // test if sPtr starts with given character, move pointer if yes
    bool ProtectedTestSymbol(TCHAR ch);

    // same, but do not move the pointer
    bool ProtectedTestSymbolNoMove(TCHAR ch);

    // test character, throw an exception if not matched
    void ProtectedTestSymbolHard(TCHAR ch);

    // go to next symbol
    void ProtectedNext();

    // set current pointer position
    void ProtectedSetPos(int pos);

    // what to do after reading something; e.g., skip whitespace
    virtual void PostRead() = 0;

    // return the current pointer
    LPCTSTR ProtectedGetPtr()
    {
        return m_sPtr;
    }

    // set the parser to the end
    void ProtectedSetEof()
    {
        m_sPtr = NULL;
    }

public:
    explicit TStringParserAbstract(LPCTSTR sPtr);
    virtual ~TStringParserAbstract();

    // set the ignore case flag
    virtual void SetIgnoreCase(bool bIgnoreCase);

    // get the start of the parsed string
    LPCTSTR GetInitPtr() const
    {
        return m_sInitial;
    }

    // total length of the input
    int GetLength()
    {
        return m_length;
    }

    // get the ignore case flag
    bool GetIgnoreCase()
    {
        return m_bIgnoreCase;
    }

    // are we in the end yet?
    bool IsEof()
    {
        return m_sPtr == NULL;
    }

    // get current symbol
    TCHAR GetCurrent()
    {
        return IsEof() ? 0 : *m_sPtr;
    }

    // current pointer position
    int GetPos()
    {
        return IsEof() ? m_length : m_sPtr - m_sInitial;
    }

    // set current pointer position
    virtual void SetPos(int pos);

    // read the rest of the buffer
    virtual CString ReadFinal();

    // read a string (MUST be quoted)
    virtual CString ReadQuotedStr();

    // read a double number
    virtual double ReadDouble();

    // read a double number; translate '?' to Nan
    virtual double ReadDoubleNan();

    // read double, divide by 100, skip or force a % sign
    virtual double ReadPercentage(bool bForcePrecent = false);

    // read double; translate '?' to Nan; divide by 100
    virtual double ReadPercentageNan(bool bForcePrecent = false);

    // if not eof, throw exception
    virtual void TestEofHard();

    // read decimal integer
    virtual int ReadInt();

    // read decimal unsigned integer
    virtual UINT ReadUint();

    // read hexadecimal integer
    virtual UINT ReadHex();

    // read decimal integer
    virtual INT64 ReadInt64();

    // read decimal unsigned integer
    virtual UINT64 ReadUint64();

    // read hexadecimal integer
    virtual UINT64 ReadHex64();

    // read bool -- plus or minus
    virtual bool ReadPM();

    // read alphanumeric token
    virtual CString ReadAlnum();

    // class for manipulating the ignore case flag:
    // constructor sets it to new value,
    // destructor restores the old value

    class AFLIB TCaseManipulator
    {
    private:
        // object to which the manipulator is being applied
        TStringParserAbstract& m_host;

        // old value of the ignore case flag
        bool m_bIgnoreCaseSaved;

    public:
        TCaseManipulator(TStringParserAbstract& host, bool bIgnoreCase);
        ~TCaseManipulator();
    };
};
}  // namespace AFLibIO
