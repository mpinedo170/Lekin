#include "StdAfx.h"

#include "AFLibFileParser.h"

using namespace AFLibIO;

TFileParser::TFileParser(CFile& file, LPCTSTR delimiter, int blockSize) :
    super(file, blockSize),
    m_delimiter(delimiter)
{
    StartReader();
}

TFileParser::~TFileParser()
{
    FinishAndDestroy(true);
}

bool TFileParser::FindDelimiter(const TStringBuffer& line, int& position, int& skip)
{
    position = line.Find(m_delimiter);
    skip = -1;

    if (position < 0) return false;
    skip = m_delimiter.GetLength();
    return true;
}
