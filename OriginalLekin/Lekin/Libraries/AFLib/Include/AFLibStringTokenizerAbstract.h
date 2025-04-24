#pragma once

#include "AFLibStringParserAbstract.h"

/////////////////////////////////////////////////////////////////////////////
// TStringTokenizerAbstract
// Parent for TStringTokenizer, TStringTokenizerStr, TStringTokenizerAlt,
//   and TStringTokenizerText

namespace AFLibIO {
class AFLIB TStringTokenizerAbstract : public TStringParserAbstract
{
private:
    typedef TStringParserAbstract super;
    DEFINE_COPY_AND_ASSIGN(TStringTokenizerAbstract);

protected:
    bool m_bCommaSafe;  // comma is not a part of a delimiter

    // read the next token (fast version)
    virtual void ProtectedReadStr(TStringBuffer& sDest) = 0;

    // read a string from Excel's clipboard (fast version)
    void ProtectedReadExcelStr(TStringBuffer& sDest);

public:
    explicit TStringTokenizerAbstract(LPCTSTR sPtr);

    // skip the next token if it is empty; returns true if skipped
    virtual bool SkipIfEmpty() = 0;

    // read the next token (public version)
    CString ReadStr();

    // skip the next token
    void SkipStr();

    // read a string from Excel's clipboard (public version)
    CString ReadExcelStr();

    // read a double number from an Excel string (skip commas)
    double NoCommasDouble();
};
}  // namespace AFLibIO
