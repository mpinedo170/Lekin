#include "StdAfx.h"

#include "AFLibStringWriter.h"

#include "AFLibGlobal.h"
#include "AFLibStringTokenizerStr.h"
#include "AFLibThreadSafeConst.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;

// slightly longer default buffer for "string writer"
namespace {
const int DefGrowBy = 1 << 16;
}

TStringWriter::TStringWriter(int lineLength, LPCTSTR delimiter) : m_buffer(DefGrowBy)
{
    m_lineLength = lineLength;
    m_currentLength = 0;
    SetIndent(0);
    m_indenter = chrSpace;
    m_bNewLine = true;
    SetDelimiter(delimiter);
}

void TStringWriter::ResetNextDelim()
{
    m_nextDelim.Empty();

    if (m_currentLength > 0)
        m_nextDelim += m_delimiter;
    else
        m_nextDelim.AppendCh(m_indenter, m_bNewLine ? m_indentReg : m_indentRoll);
}

void TStringWriter::SetDelimiter(LPCTSTR delimiter)
{
    m_delimiter = delimiter;
    ResetNextDelim();
}

void TStringWriter::SetIndenter(TCHAR indenter)
{
    m_indenter = indenter;
    ResetNextDelim();
}

void TStringWriter::SetIndent(int indentReg, int indentRoll)
{
    ResetWord();
    m_indentReg = indentReg;
    m_indentRoll = indentRoll == -1 ? indentReg + 2 : indentRoll;
    ResetNextDelim();
}

void TStringWriter::AddEol()
{
    m_buffer += strEol;
    m_currentLength = 0;
}

void TStringWriter::AddNextDelim()
{
    m_buffer += m_nextDelim;
    m_currentLength += m_nextDelim.GetLength();
}

bool TStringWriter::Fits(int length)
{
    return m_currentLength + m_nextDelim.GetLength() + m_word.GetLength() + length <= m_lineLength;
}

void TStringWriter::ResetWord()
{
    if (m_word.IsEmpty()) return;

    if (m_currentLength == 0 || Fits(0))
        AddNextDelim();
    else
    {
        AddEol();
        ResetNextDelim();
        AddNextDelim();
    }

    m_buffer += m_word;
    m_currentLength += m_word.GetLength();
    m_bNewLine = false;
    m_word.Empty();
    ResetNextDelim();
}

void TStringWriter::Rollover()
{
    ResetWord();
    AddEol();
    ResetNextDelim();
}

void TStringWriter::NewLine()
{
    ResetWord();
    AddEol();
    m_bNewLine = true;
    ResetNextDelim();
}

void TStringWriter::WriteSplit(LPCTSTR s)
{
    TStringTokenizerStr sp(s, m_delimiter);
    while (!sp.IsEof())
    {
        CString token = sp.ReadStr();
        Write(token);
    }
}

void TStringWriter::ConvertToC(LPCBYTE& s, int size, TStringBuffer& letter)
{
    SAFE_CONST_STR(strBackslashN, _T("\\n"));
    SAFE_CONST_STR(strBackslashT, _T("\\t"));
    SAFE_CONST_STR(strBackslashF, _T("\\f"));
    SAFE_CONST_STR(strBackslashX, _T("\\x"));
    SAFE_CONST_STR(strBackslashCapX, _T("\\X"));

    WCHAR ch = size == 1 ? *s : *reinterpret_cast<const WCHAR*>(s);
    s += size;

    switch (ch)
    {
        case 0:
            s = NULL;
            return;

        case chrDQuote:
        case chrBSlash:
            letter += chrBSlash;
            letter += TCHAR(ch);
            return;

        case chrTab:
            letter += strBackslashT;
            return;

        case chrFormFeed:
            letter += strBackslashF;
            return;

        case chrNewLine:
            letter += strBackslashN;
            return;

        case chrCarRet:
            return;
    }

    BYTE cAscii = BYTE(ch);

    if (cAscii != ch)
    {
        letter += strBackslashCapX;
        letter.AppendHex(ch, 4);
    }
    else if (cAscii < ' ' || cAscii > 126)
    {
        letter += strBackslashX;
        letter.AppendHex(ch, 2);
    }
    else
        letter += TCHAR(ch);
}

void TStringWriter::PrivateWriteCStr(LPCBYTE s, int size)
{
    if (!GetConcat())
    {
        m_word += chrDQuote;

        while (s != NULL) ConvertToC(s, size, m_word);

        m_word += chrDQuote;
        ResetWord();
        return;
    }

    SAFE_STATIC_SB(strLetter);
    ConvertToC(s, size, strLetter);
    if (s == NULL)
    {
        m_word += str2Quote;
        return;
    }

    if (m_currentLength > 0 && !Fits(strLetter.GetLength() + 2)) Rollover();

    m_word += chrDQuote;
    m_word += strLetter;

    while (true)
    {
        strLetter.Empty();
        ConvertToC(s, size, strLetter);
        if (s == NULL) break;

        if (Fits(strLetter.GetLength() + 1))
            m_word += strLetter;
        else
        {
            m_word += chrDQuote;
            Rollover();
            m_word.Empty();
            m_word += chrDQuote;
            m_word += strLetter;
        }
    }

    m_word += chrDQuote;
}

bool TStringWriter::IsStrSafe(LPCTSTR s)
{
    if (*s == 0 || int(_tcsnlen(s, m_lineLength + 1)) > m_lineLength) return false;

    for (; *s != 0; ++s)
        if (!IsChrSafe(*s)) return false;

    return true;
}

void TStringWriter::WriteSafeStr(LPCTSTR s)
{
    if (IsStrSafe(s))
    {
        Write(s);
        return;
    }

    TConcatManipulator cm(*this, false);
    WriteCStr(s);
}

void TStringWriter::WriteSafeStrNoDelim(LPCTSTR s)
{
    if (IsStrSafe(s))
    {
        WriteNoDelim(s);
        return;
    }

    TConcatManipulator cm(*this, false);
    WriteCStrNoDelim(s);
}

const AFLibIO::TStringBuffer& TStringWriter::GetBuffer()
{
    ResetWord();
    return m_buffer;
}
