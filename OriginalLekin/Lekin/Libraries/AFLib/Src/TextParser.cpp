#include "StdAfx.h"

#include "AFLibTextParser.h"

#include "AFLibStrings.h"

using namespace AFLib;
using namespace AFLibIO;

TTextParser::TTextParser(CFile& file) : super(file)
{
    StartReader();
}

TTextParser::~TTextParser()
{
    FinishAndDestroy(true);
}

bool TTextParser::FindDelimiter(const TStringBuffer& line, int& position, int& skip)
{
    position = line.FindOneOf(strEol);
    skip = -1;

    // if the last character in line, do not process yet
    if (position < 0 || position >= line.GetLength() - 1) return false;

    // if next char is also a CR or LF, but not the same as this char,
    // treat both of them as one EOL
    skip = strEol.Find(line[position + 1]) >= 0 && line[position + 1] != line[position] ? 2 : 1;

    return true;
}

int TTextParser::FindLastSegment(const TStringBuffer& line)
{
    int position = line.FindOneOf(strEol);
    return position < 0 ? line.GetLength() : position;
}
