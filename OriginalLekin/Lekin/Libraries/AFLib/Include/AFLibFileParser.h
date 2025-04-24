#pragma once

#include "AFLibFileParserAbstract.h"

/////////////////////////////////////////////////////////////////////////////
// TFileParser structure
// Splits the file into tokens delimited by a given string

namespace AFLibIO {
class AFLIB TFileParser : public TFileParserAbstract
{
private:
    typedef TFileParserAbstract super;
    DEFINE_COPY_AND_ASSIGN(TFileParser);

    CString m_delimiter;  // token separator

protected:
    virtual bool FindDelimiter(const TStringBuffer& line, int& position, int& skip);

public:
    TFileParser(CFile& file, LPCTSTR delimiter, int blockSize = DefBlockSize);
    ~TFileParser();
};
}  // namespace AFLibIO
