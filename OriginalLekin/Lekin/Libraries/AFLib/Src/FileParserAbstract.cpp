#include "StdAfx.h"

#include "AFLibFileParserAbstract.h"

#include "AFLibGlobal.h"
#include "AFLibThread.h"
#include "Local.h"
#include "WorcerRead.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibThread;
using namespace AFLibPrivate;

TFileParserAbstract::TFileParserAbstract(CFile& file, int blockSize) :
    m_file(file),
    m_line(blockSize),
    m_remainder(blockSize)
{
    m_length = file.GetLength();
    m_lenSig = 0;
    m_lenChar = 1;
    m_ttf = ttfN;
    file.SeekToBegin();

    BYTE s[lenTextPrefixMax] = {0};
    file.Read(s, lenTextPrefixMax);

    for (int i = 0; i < ttfN; ++i)
    {
        int len = arrLenTextPrefix[i];
        if (memcmp(s, arrStrTextPrefix[i], len) == 0)
        {
            m_ttf = TTypeTextFile(i);
            m_lenSig = len;
            break;
        }
    }

    switch (m_ttf)
    {
        case ttfN:
            m_ttf = ttfAscii;
            break;

        case ttfAscii:
            // in some files, the UTF-8 mark is repeated; check for that!
            while (file.Read(s, lenTextPrefixMax) == lenTextPrefixMax &&
                   memcmp(s, arrStrTextPrefix[ttfAscii], lenTextPrefixMax) == 0)
                m_lenSig += lenTextPrefixMax;
            break;

        default:
            m_lenChar = 2;
            break;
    }

    file.Seek(m_lenSig, CFile::begin);
    m_position = m_lenSig;
    m_nEol = m_nSkip = 0;
    m_bEof = m_bReachedEof = m_position > m_length - m_lenChar;
}

TWorcer* TFileParserAbstract::CreateWorcer(int index, int nWorcers)
{
    return new TWorcerRead(this);
}

void TFileParserAbstract::StartReader()
{
    if (m_bReachedEof) return;
    StartWorc(1);
    StartStage();
}

void TFileParserAbstract::SetPosition(INT64 pos)
{
    FinishAndDestroy(false);

    if (pos < m_lenSig)
        pos = m_lenSig;
    else if (pos > m_length)
        pos = m_length;

    // make sure we are reading at the right word boundary
    if (m_ttf != ttfAscii) pos &= ~INT64(1);

    m_file.Seek(pos, CFile::begin);
    m_line.Empty();
    m_remainder.Empty();
    m_position = pos;
    m_bEof = m_bReachedEof = pos > m_length - m_lenChar;
    m_nEol = m_nSkip = 0;
    StartReader();
}

const TStringBuffer& TFileParserAbstract::GetNext()
{
    m_line.Empty();
    if (m_bEof) ThrowMessage(LocalAfxString(IDS_PAST_EOF));

    WaitStage();

    int nTotalSeg = m_nEol + m_nSkip;

    m_position += nTotalSeg * m_lenChar;
    m_line.Append(LPCTSTR(m_remainder), m_nEol);
    m_remainder.Delete(0, nTotalSeg);

    if (m_bReachedEof)
        m_bEof = true;
    else
        StartStage();
    return m_line;
}

int TFileParserAbstract::FindLastSegment(const TStringBuffer& line)
{
    return line.GetLength();
}

CString TFileParserAbstract::GetFilePath()
{
    return m_file.GetFilePath();
}

CString TFileParserAbstract::GetFileName()
{
    return m_file.GetFileName();
}
