#include "StdAfx.h"

#include "AFLibSuperText.h"

#include "AFLibDCStateSaver.h"
#include "AFLibFontDescriptor.h"
#include "AFLibGlobal.h"
#include "AFLibImageAF.h"
#include "AFLibMathGlobal.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibGui;
using namespace AFLibIO;
using namespace AFLibPrivate;
using AFLibMath::Rint;

const TSuperText::TPage TSuperText::TPage::m_pageZero(0, 0);

TSuperText::TSuperText(const TFontDescriptor* pFdc)
{
    m_pFdc = pFdc;
    m_growBy = 1 << 10;
    m_pageHeader = TPage::m_pageZero;
    m_pSync = new CCriticalSection;
    PrivateClear();
}

TSuperText::TSuperText(const TSuperText& superText)
{
    m_pSync = new CCriticalSection;
    *this = superText;
}

TSuperText& TSuperText::operator=(const TSuperText& superText)
{
    if (this == &superText) return *this;

    ClearCache();
    *static_cast<super*>(this) = superText;
    m_pageHeader = superText.m_pageHeader;
    m_arrContent.Copy(superText.m_arrContent);
    m_arrMinTabWidth.Copy(superText.m_arrMinTabWidth);
    return *this;
}

TSuperText::~TSuperText()
{
    ClearCache();
    PrivateClear();
    delete m_pSync;
}

void TSuperText::SetFontDescriptor(const TFontDescriptor* pFdc)
{
    ClearCache();
    m_pFdc = pFdc;
}

void TSuperText::SetGrowBy(int growBy)
{
    m_growBy = growBy;
    m_arrContent.SetSize(m_arrContent.GetSize(), m_growBy);
}

void TSuperText::PrivateClear()
{
    m_attributeNew = 0;
    m_attributeOld = BadAttribute;
    m_tab = 0;
    m_lineSkipCount = DefaultLineSkipCount;
    m_realLength = 0;
    m_padding = DefaultPadding;
    m_arrContent.SetSize(0, m_growBy);
    m_arrMinTabWidth.RemoveAll();
}

void TSuperText::PrivateClearCache() const
{
    CSingleLock lock(m_pSync, true);
    m_arrDrawCache.DestroyAll();
}

void TSuperText::Clear()
{
    PrivateClear();
}

void TSuperText::ClearCache() const
{
    PrivateClearCache();
}

void TSuperText::AddA(DWORD attribute)
{
    AddC(0);
    AddC(LOWORD(attribute));
    AddC(HIWORD(attribute));
}

void TSuperText::AddBrk(TBrk brk)
{
    ASSERT(brk != brkNone);
    ClearCache();
    ++m_tab;

    DWORD attribute = m_attributeNew;
    attribute = UpdateBitField(attribute, atBrk, brk);
    attribute = UpdateBitField(attribute, atTab, m_tab);
    AddA(attribute);

    m_attributeOld = m_attributeNew;
    if (brk > brkTab) m_tab = 0;
}

void TSuperText::AddChar(WCHAR ch)
{
    ClearCache();

    if (m_attributeNew != m_attributeOld)
    {
        AddA(m_attributeNew);
        m_attributeOld = m_attributeNew;
    }

    AddC(ch);
    m_realLength = GetLength();
}

void TSuperText::AddSpecial(WCHAR ch)
{
    ClearCache();
    SetAttrBit(true, atSpecial);
    AddChar(ch);
    SetAttrBit(false, atSpecial);
}

int TSuperText::DrawSpecial(
    CDC* pDC, const TDrawCache* pDrawCache, WCHAR ch, const CPoint& point, bool bBold, bool bItalic) const
{
    return pDrawCache->m_hFont;
}

void TSuperText::InitFontFamilies(TDrawCache* pDrawCache) const
{}

void TSuperText::SpecialToUnicode(WCHAR ch, TStringBufferW& buffer) const
{}

COLORREF TSuperText::GetRealColor(CDC* pDC, BYTE ci) const
{
    return colorAuto;
}

void TSuperText::Copy() const
{
    WriteClipboardString(ToRtf(), true, RegisterClipboardFormat(_T("Rich Text Format")));
    CStringW s = ToUnicode();
    WriteClipboardStringW(s, false);

#ifdef _UNICODE
    WriteClipboardString(s, false);
#else
    CString s2(s);
    WriteClipboardString(s2, false);
#endif
}

void TSuperText::Add(CHAR ch)
{
    switch (ch)
    {
        case chrFormFeedA:
            AddBrk(brkPage);
            break;

        case chrNewLineA:
            AddBrk(brkEol);
            break;

        case chrCarRetA:
            break;

        case chrTabA:
            SetTab(m_tab + 1);
            break;

        default:
            if (BYTE(ch) < BYTE(chrSpace))
                AddSpecial(ch);
            else
                AddChar(ch);
    }
}

void TSuperText::Add(WCHAR ch)
{  // non-Ascii characters go as-is;
    // Ascii have to be probed for special characters

    if ((ch & 0xFF00) != 0)
        AddChar(ch);
    else
        Add(CHAR(ch));
}

void TSuperText::Add(LPCWSTR s)
{
    for (; *s; ++s) Add(*s);
}

void TSuperText::Add(LPCSTR s)
{
    for (; *s; ++s) Add(*s);
}

void TSuperText::Add(const TSuperText& superText)
{
    if (superText.GetLength() == 0) return;
    ClearCache();

    SAFE_STATIC_SB_W(bufTemp);
    BYTE ciText = GetCIText();
    BYTE ciBack = GetCIBack();

    if (superText.m_realLength > 0) m_realLength = GetLength() + superText.m_realLength;

    int index = GetLength();
    m_arrContent.Append(superText.m_arrContent);

    // superText.m_arrContent[0] is 0;
    // superText.m_arrContent[1,2] is initial attribute

    if (superText.m_arrContent[0] == 0 && superText.GetAttribute(1) == m_attributeOld)
    {
        m_arrContent.RemoveAt(index, 3);
        m_realLength -= 3;
    }

    while (index < GetLength())
    {
        DWORD attribute = BadAttribute;
        if (!ReadNextToken(index, GetLength(), attribute, bufTemp)) continue;

        TranslateAttribute(attribute, ciText, ciBack);
        if (m_tab > 0)
        {
            TBrk brk = GetBrk(attribute);

            if (brk >= brkTab)
            {
                int newTab = GetBitField(attribute, atTab) + m_tab;
                int maxTab = MaxTabIndex;

                if (brk >= brkEol)
                {
                    ++maxTab;
                    m_tab = 0;
                }

                attribute = UpdateBitField(attribute, atTab, min(newTab, maxTab));
            }
        }
        GetAttribute(index - 2) = attribute;
    }

    m_tab = min(m_tab + superText.m_tab, MaxTabIndex);
    m_attributeOld = superText.m_attributeOld;
    m_attributeNew = superText.m_attributeNew;

    TranslateAttribute(m_attributeOld, ciText, ciBack);
    TranslateAttribute(m_attributeNew, ciText, ciBack);
}

void TSuperText::TranslateAttribute(DWORD& attribute, BYTE ciText, BYTE ciBack)
{
    if (attribute == BadAttribute) return;

    if (ciText != 0)
    {
        BYTE ciTextOld = BYTE(GetBitField(attribute, atColorText));
        if (ciTextOld == 0) attribute = UpdateBitField(attribute, atColorText, ciText);
    }

    if (ciBack != 0)
    {
        BYTE ciBackOld = BYTE(GetBitField(attribute, atColorBack));
        if (ciBackOld == 0) attribute = UpdateBitField(attribute, atColorBack, ciBack);
    }
}

BYTE TSuperText::GetCIText() const
{
    return BYTE(GetBitField(m_attributeNew, atColorText));
}

BYTE TSuperText::GetCIBack() const
{
    return BYTE(GetBitField(m_attributeNew, atColorBack));
}

void TSuperText::SetAttrBit(bool bSet, DWORD mask)
{
    m_attributeNew = UpdateBitField(m_attributeNew, mask, bSet ? 1 : 0);
}

void TSuperText::SetCIText(BYTE ci)
{
    m_attributeNew = UpdateBitField(m_attributeNew, atColorText, ci);
}

void TSuperText::SetCIBack(BYTE ci)
{
    m_attributeNew = UpdateBitField(m_attributeNew, atColorBack, ci);
}

void TSuperText::SetTab(int tab)
{
    tab = min(tab, MaxTabIndex);
    if (tab <= m_tab) return;

    m_tab = tab - 1;
    AddBrk(brkTab);
    m_realLength = GetLength();
}

void TSuperText::SetLineSkipCount(int lineSkipCount)
{
    ASSERT(lineSkipCount > 0);
    m_lineSkipCount = lineSkipCount;
}

void TSuperText::SetPadding(double padding)
{
    ASSERT(padding >= 0);
    m_padding = padding;
}

void TSuperText::SetMinTabWidth(int tab, double width)
{
    m_arrMinTabWidth.SetAtGrow(tab, width);
}

double TSuperText::GetMinTabWidth(int tab) const
{
    return tab >= m_arrMinTabWidth.GetSize() ? 0 : m_arrMinTabWidth[tab];
}

TSuperText::TDrawCache* TSuperText::Compute(CDC* pDC) const
{
    ASSERT(m_pFdc != NULL);

    CSingleLock lock(m_pSync, true);
    for (int i = 0; i < m_arrDrawCache.GetSize(); ++i)
        if (m_arrDrawCache[i]->Fits(pDC)) return m_arrDrawCache[i];

    const_cast<CWordArray*>(&m_arrContent)->FreeExtra();
    lock.Unlock();

    CAutoPtr<TDrawCache> pDrawCache(new TDrawCache(this, pDC));
    TDCStateSaver dcss(pDC);

    pDrawCache->AddFontFamily(m_pFdc);
    pDC->SelectObject(pDrawCache->GetFont(0, pDC, false, false));
    pDrawCache->m_hFont = pDC->GetTextExtent(strFontTest).cy;
    InitFontFamilies(pDrawCache);
    pDrawCache->Init(pDC);

    lock.Lock();
    int count = m_arrDrawCache.GetSize();

    for (int i = 0; i < count; ++i)
        if (m_arrDrawCache[i]->Fits(pDC)) return m_arrDrawCache[i];

    m_arrDrawCache.Add(pDrawCache.Detach());
    return m_arrDrawCache[count];
}

int TSuperText::GetExtent(CDC* pDC, CSize& sz) const
{
    TDrawCache* pDrawCache = Compute(pDC);
    sz = pDrawCache->m_szTotal;
    return pDrawCache->m_hFont;
}

bool TSuperText::ReadNextToken(int& index, int indexEnd, DWORD& attribute, TStringBufferW& str) const
{
    ASSERT(index < indexEnd);
    str.Empty();

    if (m_arrContent[index] == 0)
    {
        ASSERT(index <= indexEnd - 3);
        attribute = GetAttribute(index + 1);
        index += 3;
        return true;
    }

    int len = indexEnd - index;
    LPCWSTR s = LPCWSTR(m_arrContent.GetData() + index);
    LPCWSTR sEnd = wmemchr(s, 0, len);
    if (sEnd != NULL) len = sEnd - s;

    str.Append(s, len);
    index += len;
    return false;
}

void TSuperText::DrawSegment(CDC* pDC, const CPoint& point, const TPage& pageStart, int indexEnd) const
{
    TDrawCache* pDrawCache = Compute(pDC);
    pDrawCache->DrawSegment(pDC, point, pageStart, indexEnd);
}

void TSuperText::Draw(CDC* pDC, const CPoint& point) const
{
    DrawSegment(pDC, point, TPage::m_pageZero, GetLength());
}

void TSuperText::DrawPage(CDC* pDC, const CPoint& point, int page) const
{
    TDrawCache* pDrawCache = Compute(pDC);
    ASSERT(page < pDrawCache->m_arrPage.GetSize() - 1);

    pDrawCache->DrawSegment(pDC, point, TPage::m_pageZero, m_pageHeader.m_index);
    pDrawCache->DrawSegment(pDC, CPoint(point.x, point.y + pDrawCache->m_hHeader),
        pDrawCache->m_arrPage[page], pDrawCache->m_arrPage[page + 1].m_index);
}

void TSuperText::PrivateUploadRich(CRichEditCtrl& wndRich, int length) const
{
    if (length == 0) return;

    bool bVisible = wndRich.IsWindowVisible() != 0;
    if (bVisible) wndRich.ShowWindow(SW_HIDE);
    COLORREF colorBackDefault = wndRich.SetBackgroundColor(true, 0);
    wndRich.SetBackgroundColor(false, colorBackDefault);

    CDC* pDC = wndRich.GetDC();
    pDC->SetTextColor(colorText());
    TDrawCache* pDrawCache = Compute(pDC);
    pDrawCache->UploadRich(pDC, colorBackDefault, wndRich, length);
    if (bVisible) wndRich.ShowWindow(SW_SHOW);
}

void TSuperText::UploadRich(CRichEditCtrl& wndRich) const
{
    PrivateUploadRich(wndRich, GetLength());
}

void TSuperText::UploadClearRich(CRichEditCtrl& wndRich, bool bReturnCursor) const
{
    wndRich.SetWindowText(strSpace);  // CString(chrSpace, GetLength() * 4));
    wndRich.SetWindowText(strEmpty);
    PrivateUploadRich(wndRich, GetRealLength());
    if (bReturnCursor) wndRich.SetSel(0, 0);
}

void TSuperText::CreateImage(CImageAF& img) const
{
    CDC* pDC = NULL;

    if (img.IsNull())
    {
        pDC = CWnd::GetDesktopWindow()->GetDC();

        CSize sz;
        GetExtent(pDC, sz);
        img.CreateCompatible(sz, pDC);
        CWnd::GetDesktopWindow()->ReleaseDC(pDC);
        img.Fill(colorNormal());
    }

    pDC = img.GetCDC();
    pDC->SetTextColor(colorText());
    Draw(pDC, CPoint(0, 0));
    CImageAF::ReleaseCDC(pDC);
}

void TSuperText::PrivateMarkHeader(int index)
{
    DWORD attribute = 0;
    ClearCache();

    index = min(index, GetLength());

    // header must be either 0 or right after Eol or end of page
    for (index -= 3; index >= 0; --index)
    {
        if (m_arrContent[index] != 0) continue;
        attribute = GetAttribute(index + 1);
        if (GetBrk(attribute) >= brkEol) break;
    }

    m_pageHeader = index < 0 ? TPage::m_pageZero : TPage(index + 3, attribute);
}

void TSuperText::MarkHeader()
{
    PrivateMarkHeader(GetLength());
}

void TSuperText::StartPaging(CDC* pDC) const
{
    TDrawCache* pDrawCache = Compute(pDC);
    pDrawCache->m_arrPage.RemoveAll();
    pDrawCache->m_pageNice = m_pageHeader;
    pDrawCache->m_pageHard = m_pageHeader;
}

bool TSuperText::ProcessNextPage(CDC* pDC, int pageHeight) const
{
    TDrawCache* pDrawCache = Compute(pDC);
    return pDrawCache->ProcessNextPage(pageHeight);
}

int TSuperText::PrivateGetPageCount(CDC* pDC) const
{
    TDrawCache* pDrawCache = Compute(pDC);
    const CArray<TPage>& arrPage = pDrawCache->m_arrPage;
    int pageCount = arrPage.GetSize() - 1;

    if (pageCount < 1) return 0;

    return arrPage[0].m_index == m_pageHeader.m_index && arrPage[pageCount].m_index >= GetRealLength()
               ? pageCount
               : 0;
}

bool TSuperText::IsPaged(CDC* pDC) const
{
    return PrivateGetPageCount(pDC) > 0;
}

int TSuperText::GetPageCount(CDC* pDC) const
{
    int pageCount = PrivateGetPageCount(pDC);
    ASSERT(pageCount > 0);
    return pageCount;
}
