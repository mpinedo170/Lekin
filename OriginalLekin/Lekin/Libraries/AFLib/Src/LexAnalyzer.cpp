#include "StdAfx.h"

#include "AFLibLexAnalyzer.h"

#include "AFLibExcBreak.h"
#include "AFLibExcMessage.h"
#include "AFLibGlobal.h"
#include "AFLibLexParser.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;
using AFLibCalc::TLexAnalyzer;

namespace {
int compareLexem(LPCVOID a1, LPCVOID a2)
{
    const CString& s1 = *reinterpret_cast<const CString*>(a1);
    const CString& s2 = *reinterpret_cast<const CString*>(a2);

    int dL = s2.GetLength() - s1.GetLength();
    return dL == 0 ? s1.Compare(s2) : dL;
}
}  // namespace

TLexAnalyzer::TLexAnalyzer()
{
    m_bIgnoreKeywordCase = false;
    m_pSp = NULL;
    ClearCurrent();
}

TLexAnalyzer::~TLexAnalyzer()
{
    delete m_pSp;
}

void TLexAnalyzer::AddLexem(LPCTSTR sLex, int IDLex, bool bThrowOnDup)
{
    CString key = sLex;
    bool bFound = false;
    int index =
        BinarySearch(&key, m_arrSLex.GetData(), m_arrSLex.GetSize(), sizeof(CString), compareLexem, bFound);

    if (bFound)
    {
        if (bThrowOnDup) ThrowMessage(LocalAfxString(IDS_DUP_LEXEM, sLex));
        m_arrIDLex[index] = IDLex;
    }
    else
    {
        m_arrSLex.InsertAt(index, sLex);
        m_arrIDLex.InsertAt(index, IDLex);
    }
}

void TLexAnalyzer::DelLexem(LPCTSTR sLex, bool bThrowOnNotFound)
{
    CString key = sLex;
    bool bFound = false;
    int index =
        BinarySearch(&key, m_arrSLex.GetData(), m_arrSLex.GetSize(), sizeof(CString), compareLexem, bFound);

    if (bFound)
    {
        m_arrSLex.RemoveAt(index);
        m_arrIDLex.RemoveAt(index);
    }
    else if (bThrowOnNotFound)
        ThrowMessage(LocalAfxString(IDS_NO_LEXEM, sLex));
}

void TLexAnalyzer::ClearLexems()
{
    m_arrSLex.RemoveAll();
    m_arrIDLex.RemoveAll();
}

void TLexAnalyzer::SetKeywordIC(bool bIgnoreKeywordCase)
{
    ASSERT(m_mapKeyword.GetCount() == 0);
    m_bIgnoreKeywordCase = bIgnoreKeywordCase;
}

void TLexAnalyzer::AddKeyword(LPCTSTR sKey, int IDLex, bool bThrowOnDup)
{
    CString sKey2 = sKey;
    if (m_bIgnoreKeywordCase) sKey2.MakeLower();

    if (bThrowOnDup && m_mapKeyword.Exists(sKey2)) ThrowMessage(LocalAfxString(IDS_DUP_KEYWORD, sKey));

    m_mapKeyword.SetAt(sKey2, IDLex);
}

void TLexAnalyzer::DelKeyword(LPCTSTR sKey, bool bThrowOnNotFound)
{
    CString sKey2 = sKey;
    if (m_bIgnoreKeywordCase) sKey2.MakeLower();

    if (bThrowOnNotFound && !m_mapKeyword.Exists(sKey2)) ThrowMessage(LocalAfxString(IDS_NO_KEYWORD, sKey));

    m_mapKeyword.RemoveKey(sKey2);
}

void TLexAnalyzer::ClearKeywords()
{
    m_mapKeyword.RemoveAll();
}

void TLexAnalyzer::ClearCurrent()
{
    m_pos = 0;
    m_iLine = m_iCol = 1;
    m_IDLex = lexEOF;
    m_str.Empty();
    m_i = 0;
    m_d = 0;
}

void TLexAnalyzer::CreateParser(LPCTSTR buffer)
{
    m_pSp = new TLexParser(buffer);
}

bool TLexAnalyzer::SkipComments()
{
    return false;
}

void TLexAnalyzer::ConvertKeyword()
{
    if (m_bIgnoreKeywordCase)
    {
        CString s = m_str;
        s.MakeLower();
        m_IDLex = m_mapKeyword.GetAt(s, m_IDLex);
    }
    else
        m_IDLex = m_mapKeyword.GetAt(m_str, m_IDLex);
}

bool TLexAnalyzer::ReadStandardLexem()
{
    int i = m_pSp->TestSymbolArray(m_arrSLex);
    if (i < 0) return false;

    m_IDLex = m_arrIDLex[i];
    m_str = m_arrSLex[i];
    return true;
}

bool TLexAnalyzer::ReadOtherLexem()
{
    static const double intMax = double(INT_MAX);

    if (m_pSp->GetCurrent() == chrDQuote)
    {
        m_IDLex = lexString;
        m_str = m_pSp->ReadCStr();
        return true;
    }

    if (m_pSp->LexReadID(m_str))
    {
        m_IDLex = lexID;
        ConvertKeyword();
        return true;
    }

    if (m_pSp->LexReadNumber(m_str, m_d))
    {
        if (IsDigit(m_str) && m_d >= 0 && m_d <= intMax)
        {
            m_IDLex = lexInt;
            m_i = AFLibMath::Rint(m_d);
        }
        else
            m_IDLex = lexDouble;
        return true;
    }

    if (m_pSp->TestSymbol(strDot))
    {
        m_IDLex = chrDot;
        m_str = strDot;
    }

    return false;
}

CString TLexAnalyzer::GetLexemText(int IDLex)
{
    switch (IDLex)
    {
        case lexID:
            return LocalAfxString(IDS_ID);
        case lexInt:
            return LocalAfxString(IDS_INT);
        case lexDouble:
            return LocalAfxString(IDS_NUM);
        case lexString:
            return LocalAfxString(IDS_QUOTE);
    }

    for (int i = 0; i < m_arrIDLex.GetSize(); ++i)
        if (m_arrIDLex[i] == IDLex) return m_arrSLex[i];

    int IDLex2 = 0;
    CString sLex;

    for (POSITION pos = m_mapKeyword.GetStartPosition(); pos != NULL;
         m_mapKeyword.GetNextAssoc(pos, sLex, IDLex2))
        if (IDLex2 == IDLex) return sLex;

    return LocalAfxString(IDS_LEXEM, IntToStr(IDLex));
}

void TLexAnalyzer::Start(LPCTSTR buffer, int pos)
{
    delete m_pSp;
    m_pSp = NULL;
    ClearCurrent();
    CreateParser(buffer);
    m_pSp->SetPos(pos);
}

void TLexAnalyzer::PrivateNext(bool bStopAtComment)
{
    if (m_pSp == NULL) ThrowMessage(LocalAfxString(IDS_LEX_EOF));

    ClearCurrent();

    while (true)
    {
        m_pos = m_pSp->GetPos();
        m_iLine = m_pSp->GetLineNum();
        m_iCol = m_pSp->GetColNum();
        if (!SkipComments()) break;

        if (bStopAtComment)
        {
            m_IDLex = lexComment;
            return;
        }
    }

    // check if EOF is reached
    if (m_pSp->IsEof())
    {
        m_IDLex = lexEOF;
        delete m_pSp;
        m_pSp = NULL;
        return;
    }

    // try a standard lexem, like "+", "+=", etc.
    if (ReadStandardLexem()) return;

    // try to read an ID (and convert to keyword), or a number
    if (ReadOtherLexem()) return;

    ThrowMessage(LocalAfxString(IDS_LEX_BAD_SYM, CString(m_pSp->GetCurrent(), 1)));
}

void TLexAnalyzer::SlackyNext()
{
    if (m_pSp == NULL) ThrowMessage(LocalAfxString(IDS_LEX_EOF));

    int pos = m_pSp->GetPos();

    try
    {
        PrivateNext(true);
        return;
    }
    catch (AFLibThread::CExcBreak*)
    {
        throw;
    }
    catch (CExcMessage* pExc)
    {
        pExc->Delete();
    }

    if (m_pSp == NULL) return;

    m_IDLex = lexError;
    if (m_pSp->GetPos() <= pos) m_pSp->Next();
}

void TLexAnalyzer::Test(int IDLex)
{
    if (IDLex == lexEOF)
    {
        if (m_IDLex == lexEOF) return;
        ThrowMessage(LocalAfxString(IDS_EXTRA_CHAR));
    }
    else
    {
        if (m_IDLex == IDLex)
        {
            Next();
            return;
        }
        ThrowMessageExpected(GetLexemText(IDLex));
    }
}
