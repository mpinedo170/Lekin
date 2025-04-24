#include "StdAfx.h"

#include "AFLibSuperText.h"

#include "AFLibFontDescriptor.h"
#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibRichEditCtrlW.h"
#include "AFLibStringSpacer.h"
#include "AFLibStringWriter.h"
#include "AFLibThread.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibGui;
using namespace AFLibPrivate;
using AFLibMath::Rint;
using AFLibThread::CThread;

const CPoint TSuperText::m_ptSizeOnly(0, 0);
const double TSuperText::DefaultPadding = 0.5;

namespace {
const CString strMTW = _T("Tabs");
const CString strPad = _T("Padding");
const CString strLSC = _T("Skips");
const CString strHea = _T("Heading");
const CString strTxt = _T("Text");
}  // namespace

void TSuperText::Load(TStringSpacer& sp)
{
    int headerIndex = 0;

    enum
    {
        iiiMTW,
        iiiPad,
        iiiLSC,
        iiiHea
    };
    static const LPCTSTR arrKey[] = {strMTW, strPad, strLSC, strHea, NULL};

    Clear();
    sp.TestSymbolHard(_T("["));

    while (!sp.TestSymbol(strTxt))
    {
        int iii = sp.TestSymbolArrayHard(arrKey);
        sp.TestSymbolHard(strEqual);

        switch (iii)
        {
            case iiiMTW:
                sp.TestSymbolHard(_T("("));
                while (!sp.TestSymbol(_T(")"))) m_arrMinTabWidth.Add(sp.ReadDouble());
                break;

            case iiiPad:
                m_padding = sp.ReadDouble();
                break;

            case iiiLSC:
                m_lineSkipCount = sp.ReadInt();
                break;

            case iiiHea:
                headerIndex = sp.ReadInt();
                break;
        }
    }

    int lastTab = 0;

    while (!sp.TestSymbol(_T("]")))
    {
        CThread::Break();

        if (sp.TestSymbolNoMove(strDQuote))
        {
            CStringW str = sp.ReadCStrW();
            if (str.IsEmpty()) continue;

            int n = GetLength();
            m_realLength = n + str.GetLength();
            m_arrContent.SetSize(m_realLength);
            wmemcpy(LPWSTR(m_arrContent.GetData()) + n, LPCWSTR(str), str.GetLength());
        }
        else
        {
            DWORD attribute = DWORD(sp.ReadHex());
            AddA(attribute);
            m_attributeOld = attribute & atSaved;
            TBrk brk = GetBrk(attribute);
            if (brk == brkNone) continue;

            if (brk == brkTab) m_realLength = GetLength();

            m_tab = GetBitField(attribute, atTab);
            if (m_tab < lastTab) ThrowMessage(LocalAfxString(IDS_BAD_SUPER_TEXT));
            lastTab = brk == brkTab ? m_tab : 0;
        }
    }

    PrivateMarkHeader(headerIndex);
}

void TSuperText::Save(TStringWriter& sw) const
{
    sw.Write(_T("["));

    if (!m_arrMinTabWidth.IsEmpty())
    {
        TStringBuffer s;
        s += strMTW;
        s += _T("=(");

        for (int i = 0; i < m_arrMinTabWidth.GetSize(); ++i)
        {
            s.AppendDouble(m_arrMinTabWidth[i]);
            sw.Write(s);
            s.Empty();
        }
        sw.WriteNoDelim(_T(")"));
    }

    if (m_padding != DefaultPadding) sw.Write(strPad + strEqual + DoubleToStr(m_padding));

    if (m_lineSkipCount != DefaultLineSkipCount) sw.Write(strLSC + strEqual + IntToStr(m_lineSkipCount));

    if (m_pageHeader.m_index != 0) sw.Write(strHea + strEqual + IntToStr(m_pageHeader.m_index));

    sw.Write(strTxt);
    int index = 0;
    TStringBufferW str;

    while (index < GetLength())
    {
        CThread::Break();
        DWORD attribute = BadAttribute;

        if (ReadNextToken(index, GetLength(), attribute, str))
            sw.Write(HexToStr(attribute, 8));
        else if (!str.IsEmpty())
            sw.WriteCStr(str);
    }

    sw.Write(_T("]"));
}

DWORD CALLBACK TSuperText::StreamOutFunc(DWORD dwCookie, LPBYTE pbBuff, long cb, LPLONG pcb)
{
    CByteArray& buffer = *reinterpret_cast<CByteArray*>(dwCookie);
    int length = buffer.GetSize();
    buffer.SetSize(length + cb);
    memcpy(buffer.GetData() + length, pbBuff, cb);
    *pcb = cb;
    return 0;
}

CStringW TSuperText::ToUnicode() const
{
    TStringBufferW buffer(GetLength());
    TNextText xx(this, NULL, m_ptSizeOnly, 0, GetLength());

    while (!xx.IsEof())
    {
        xx.Parse(NULL);
        if (xx.IsSpecial())
        {
            for (int i = 0; i < xx.m_str.GetLength(); ++i) SpecialToUnicode(xx.m_str[i], buffer);
        }
        else
            buffer += xx.m_str;
        xx.Move();
    }

    return LPCWSTR(buffer);
}

CString TSuperText::ToRtf() const
{
    CRichEditCtrlW wndRich;
    wndRich.Create(WS_CHILD | ES_READONLY | ES_MULTILINE | ES_LEFT | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
        CRect(0, 0, 10, 10), AfxGetApp()->GetMainWnd(), 0xABFF);
    UploadRich(wndRich);

    CByteArray buffer;
    EDITSTREAM es = {0};
    es.dwCookie = DWORD(&buffer);
    es.pfnCallback = StreamOutFunc;
    wndRich.StreamOut(SF_RTF, es);
    wndRich.DestroyWindow();

    return CString(LPCSTR(buffer.GetData()), buffer.GetSize());
}

//***************** TSuperText::TNextText *****************

TSuperText::TNextText::TNextText(const TSuperText* pHost,
    const TDrawCache* pDrawCache,
    const CPoint& ptOrigin,
    int indexStart,
    int indexEnd)
{
    m_pHost = pHost;
    m_pDrawCache = pDrawCache;
    m_ptOrigin = ptOrigin;
    m_bSizeOnly = IsSizeOnly(ptOrigin);
    m_index = indexStart;
    m_indexEnd = indexEnd;

    m_brk = brkNone;
    m_attribute = 0;
    m_colorText = m_colorBack = colorAuto;
    m_width = 0;
    m_tab = m_tabNew = 0;
    m_wCurTab = 0;
    m_ptCurrent = m_ptOrigin;
}

void TSuperText::TNextText::ResetColorText(CDC* pDC)
{
    pDC->SetTextColor(m_colorText == colorAuto ? m_pDrawCache->m_colorTextDefault : m_colorText);
}

void TSuperText::TNextText::DrawBackground(CDC* pDC)
{
    if (m_colorBack == colorAuto) return;
    pDC->FillSolidRect(m_ptCurrent.x, m_ptCurrent.y, m_width, m_pDrawCache->m_hFont, m_colorBack);
}

void TSuperText::TNextText::Underline(CDC* pDC)
{
    if (IsUnderline()) m_pDrawCache->Underline(pDC, m_ptCurrent, m_width);
}

void TSuperText::TNextText::Parse(CDC* pDC)
{
    const CWordArray& arrContent = m_pHost->m_arrContent;
    ASSERT(!IsEof());

    // while we are reading an attribute, loop
    while (!IsEof() && m_pHost->ReadNextToken(m_index, m_indexEnd, m_attribute, m_str))
    {
        m_brk = GetBrk(m_attribute);

        if (IsBrk())
        {
            m_tabNew = GetBitField(m_attribute, atTab);
            ASSERT(m_tabNew > m_tab);
            m_attribute &= atSaved;

            if (m_brk >= brkEol)
            {
                for (int i = m_pHost->GetSkipCount(m_brk); --i >= 0;) m_str += strEolW;
            }
            else
            {
                m_str.AppendCh(chrTabW, m_tabNew - m_tab);
            }

            // quit the loop if this is a "break" (tab, eol, eof)
            break;
        }
    }

    // decode colors

    if (pDC == NULL)
        m_colorText = m_colorBack = colorAuto;
    else
    {
        BYTE ciText = BYTE(GetBitField(m_attribute, atColorText));
        BYTE ciBack = BYTE(GetBitField(m_attribute, atColorBack));

        m_colorText = m_pHost->GetRealColor(pDC, ciText);
        m_colorBack = m_pHost->GetRealColor(pDC, ciBack);
    }
}

void TSuperText::TNextText::Draw(CDC* pDC)
{
    ASSERT(m_pDrawCache != NULL);

    ResetColorText(pDC);

    switch (m_brk)
    {
        case brkNone:
            m_width = 0;

            if (IsSpecial())
            {
                if (m_bSizeOnly || m_colorBack != colorAuto)
                {
                    m_width = m_pDrawCache->DrawSpecialStr(
                        pDC, m_str, m_ptSizeOnly, IsBold(), IsItalic(), IsUnderline());
                    if (m_bSizeOnly) break;
                    DrawBackground(pDC);
                }

                m_width = m_pDrawCache->DrawSpecialStr(
                    pDC, m_str, m_ptCurrent, IsBold(), IsItalic(), IsUnderline());
                break;
            }
            else
            {
                CFont* pFont = m_pDrawCache->GetFont(0, pDC, IsBold(), IsItalic());
                pDC->SelectObject(pFont);
                m_width = GetTextWidthW(pDC, m_str);

                if (!m_bSizeOnly)
                {
                    DrawBackground(pDC);
                    TextOutW(pDC->m_hDC, m_ptCurrent.x, m_ptCurrent.y, m_str, m_str.GetLength());
                    Underline(pDC);
                }
                break;
            }

        case brkTab:
            if (m_bSizeOnly || m_pDrawCache->m_arrTab.IsEmpty()) break;

            m_width = m_ptOrigin.x + m_pDrawCache->m_arrTab[m_tabNew] - m_ptCurrent.x;
            DrawBackground(pDC);
            Underline(pDC);
            break;
    }
}

void TSuperText::TNextText::Move()
{
    CThread::Break();

    if (m_brk == brkTab)
        m_tab = m_tabNew;
    else if (m_brk > brkTab)
        m_tab = 0;

    if (m_pDrawCache != NULL)
    {
        switch (m_brk)
        {
            case brkNone:
                m_wCurTab += m_width;
                m_ptCurrent.x += m_width;
                m_width = 0;
                break;

            case brkTab:
                m_wCurTab = 0;
                if (m_tabNew < m_pDrawCache->m_arrTab.GetSize())
                    m_ptCurrent.x = m_ptOrigin.x + m_pDrawCache->m_arrTab[m_tabNew];
                break;

            default:
                m_wCurTab = 0;
                m_ptCurrent.y += m_pHost->GetSkipCount(m_brk) * m_pDrawCache->m_hFont;
                m_ptCurrent.x = m_ptOrigin.x;
                break;
        }
    }

    m_str.Empty();
    m_brk = brkNone;
    m_tabNew = m_tab;
}

bool TSuperText::TNextText::ComputeSimpleTab(CIntArray& arrTabWidth)
{
    ASSERT(m_tabNew > m_tab);

    // add at least 2 pixels -- just in case
    m_wCurTab += max(Rint(m_pHost->m_padding * m_pDrawCache->m_hFont), 2);

    if (arrTabWidth.GetSize() < m_tabNew) arrTabWidth.SetSize(m_tabNew);
    if (m_tabNew > m_tab + 1) return false;

    arrTabWidth[m_tab] = max(arrTabWidth[m_tab], m_wCurTab);
    return true;
}

void TSuperText::TNextText::FillCharacterFormat(CHARFORMAT2& cf)
{
    m_pHost->m_pFdc->FillCharacterFormat(cf, IsBold(), IsItalic(), IsUnderline(), m_colorText, m_colorBack);
}

//***************** TSuperText::TFontFamily *****************

TSuperText::TFontFamily::TFontFamily(const TFontDescriptor* pFdc, int pixelSize)
{
    m_pFdc = pFdc;
    m_pixelSize = pixelSize;
    for (int i = 0; i < nFonts; ++i) m_arrFont[i] = NULL;
}

TSuperText::TFontFamily::~TFontFamily()
{
    for (int i = 0; i < nFonts; ++i) delete m_arrFont[i];
}

bool TSuperText::TFontFamily::IsSame(const TFontDescriptor* pFdc, int pixelSize) const
{
    return m_pFdc == pFdc && m_pixelSize == pixelSize;
}

CFont* TSuperText::TFontFamily::GetFont(CDC* pDC, bool bBold, bool bItalic) const
{
    int index = (bBold ? 1 : 0) + (bItalic ? 2 : 0);
    CFont*& pFont = m_arrFont[index];
    if (pFont != NULL) return pFont;

    pFont = new CFont;
    m_pFdc->CreateFont(*pFont, pDC, bBold, bItalic, m_pixelSize);
    return pFont;
}

//***************** TSuperText::TBoldManipulator *****************

TSuperText::TBoldManipulator::TBoldManipulator(TSuperText& host, bool bBold) : m_host(host)
{
    m_bBoldSaved = m_host.GetBold();
    m_host.SetBold(bBold);
}

TSuperText::TBoldManipulator::~TBoldManipulator()
{
    m_host.SetBold(m_bBoldSaved);
}

//***************** TSuperText::TItalicManipulator *****************

TSuperText::TItalicManipulator::TItalicManipulator(TSuperText& host, bool bItalic) : m_host(host)
{
    m_bItalicSaved = m_host.GetItalic();
    m_host.SetItalic(bItalic);
}

TSuperText::TItalicManipulator::~TItalicManipulator()
{
    m_host.SetItalic(m_bItalicSaved);
}
