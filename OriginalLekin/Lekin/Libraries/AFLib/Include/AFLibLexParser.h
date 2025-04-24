#pragma once

#include "AFLibStringSpacer.h"

/////////////////////////////////////////////////////////////////////////////
// TLexParser class
// String parser for the lexical analyzer

namespace AFLibCalc {
class AFLIB TLexParser : public AFLibIO::TStringSpacer
{
private:
    typedef AFLibIO::TStringSpacer super;
    DEFINE_COPY_AND_ASSIGN(TLexParser);

    int m_iLine;               // current line number
    LPCTSTR m_sLineStart;      // pointer to the start of the current line
    LPCTSTR m_sNextLineStart;  // pointer to the start of the next line

    // find the start of the next line of the code
    void InitNextLineStart();

    // roll m_sLineStart and m_sNextLineStart until current position is reached
    void RollLineCount();

public:
    explicit TLexParser(LPCTSTR sPtr);

    virtual void SetPos(int pos);
    virtual void EatSpace();

    // read identifier
    virtual bool LexReadID(AFLibIO::TStringBuffer& str);

    // read a double number
    virtual bool LexReadNumber(AFLibIO::TStringBuffer& str, double& number);

    // move to the next line
    void SkipToEol();

    // find end of the comment
    void SkipTo(LPCTSTR strEnd);

    // get the current line (row) number in the parsed string
    int GetLineNum()
    {
        return m_iLine;
    }

    // get the current column number in the parsed string
    int GetColNum()
    {
        return IsEof() ? 1 : ProtectedGetPtr() - m_sLineStart + 1;
    }
};
}  // namespace AFLibCalc
