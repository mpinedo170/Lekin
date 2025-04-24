#pragma once

#include "AFLibFileParserAbstract.h"

/////////////////////////////////////////////////////////////////////////////
// TTextParser
// Splits the text file into lines.
// File may be UNIX, MAC or DOS/WIN.

namespace AFLibIO {
class AFLIB TTextParser : public TFileParserAbstract
{
private:
    typedef TFileParserAbstract super;
    DEFINE_COPY_AND_ASSIGN(TTextParser);

protected:
    virtual bool FindDelimiter(const TStringBuffer& line, int& position, int& skip);
    virtual int FindLastSegment(const TStringBuffer& line);

public:
    explicit TTextParser(CFile& file);
    ~TTextParser();
};
}  // namespace AFLibIO
