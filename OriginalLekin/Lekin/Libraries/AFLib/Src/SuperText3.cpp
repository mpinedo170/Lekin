#include "StdAfx.h"

#include "AFLibSuperText.h"

#include "AFLibDCStateSaver.h"
#include "AFLibGuiGlobal.h"
#include "AFLibImageAF.h"
#include "AFLibMathGlobal.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibGui;
using namespace AFLibIO;
using namespace AFLibPrivate;
using AFLibMath::Rint;

//***************** TSuperText::TDrawCache *****************

TSuperText::TDrawCache::TDrawCache(const TSuperText* pHost, CDC* pDC)
{
    m_pHost = pHost;
    m_xLogPix = pDC->GetDeviceCaps(LOGPIXELSX);
    m_yLogPix = pDC->GetDeviceCaps(LOGPIXELSY);
    m_maxTabWidth = 0;
    m_bTooManyTabs = false;
    m_hHeader = -1;
    m_hFont = 0;
    m_szTotal.SetSize(0, 0);
    m_colorTextDefault = pDC->GetTextColor();
    m_hPageExtra = 0;
    m_pExtraData = NULL;
}

TSuperText::TDrawCache::~TDrawCache()
{
    delete[] reinterpret_cast<LPBYTE>(m_pExtraData);
    m_arrFontFamily.DestroyAll();
}

LPVOID TSuperText::TDrawCache::AllocateExtraData(int amount)
{
    if (m_pExtraData == NULL)
    {
        m_pExtraData = new BYTE[amount];
        memset(m_pExtraData, 0, amount);
    }

    return m_pExtraData;
}

void TSuperText::TDrawCache::ClearPaging()
{
    m_pageNice.Clear();
    m_pageHard.Clear();
    m_hPageExtra = 0;
}

bool TSuperText::TDrawCache::Fits(CDC* pDC) const
{
    return m_xLogPix == pDC->GetDeviceCaps(LOGPIXELSX) && m_yLogPix == pDC->GetDeviceCaps(LOGPIXELSY);
}

int TSuperText::TDrawCache::AddFontFamily(const TFontDescriptor* pFdc, int pixelSize)
{
    for (int i = 0; i < m_arrFontFamily.GetSize(); ++i)
        if (m_arrFontFamily[i]->IsSame(pFdc, pixelSize)) return i;
    m_arrFontFamily.Add(new TFontFamily(pFdc, pixelSize));
    return m_arrFontFamily.GetSize() - 1;
}

CFont* TSuperText::TDrawCache::GetFont(int family, CDC* pDC, bool bBold, bool bItalic) const
{
    return m_arrFontFamily[family]->GetFont(pDC, bBold, bItalic);
}

void TSuperText::TDrawCache::Underline(CDC* pDC, const CPoint& point, int width) const
{
    int dy = Rint(m_hFont / 16.);
    pDC->FillSolidRect(point.x, point.y + m_hFont - dy, width, dy, pDC->GetTextColor());
}

void TSuperText::TDrawCache::Init(CDC* pDC)
{
    CIntArray arrTabWidth;
    CSortedUintArray<TExtraTab> arrExtra;

    try
    {
        for (int i = 0; i < m_pHost->m_arrMinTabWidth.GetSize(); ++i)
            arrTabWidth.Add(Rint(m_pHost->m_arrMinTabWidth[i] * m_hFont));

        TNextText xx(m_pHost, this, m_ptSizeOnly, 0, m_pHost->GetRealLength());

        while (true)
        {
            if (m_hHeader == -1 && xx.GetIndex() >= m_pHost->m_pageHeader.m_index)
                m_hHeader = xx.m_ptCurrent.y;

            if (xx.IsEof()) break;
            xx.Parse(pDC);
            xx.Draw(pDC);

            if (xx.IsBrk())
            {
                if (!xx.ComputeSimpleTab(arrTabWidth))
                {
                    TExtraTab etNew = {xx.m_tab, xx.m_tabNew, xx.m_wCurTab};
                    TExtraTab* pEt = arrExtra.FindKey(etNew.GetGLKey());

                    if (pEt == NULL)
                        arrExtra.Insert(new TExtraTab(etNew));
                    else
                        pEt->m_width = max(pEt->m_width, etNew.m_width);
                }
            }

            xx.Move();
        }

        if (xx.m_wCurTab > 0)
        {
            xx.m_tabNew = xx.m_tab + 1;
            xx.ComputeSimpleTab(arrTabWidth);
        }

        if (xx.m_ptCurrent.x > 0) xx.m_ptCurrent.y += m_hFont;

        m_bTooManyTabs = arrTabWidth.GetSize() > MAX_TAB_STOPS + 1;
        m_maxTabWidth = 0;

        for (int i = 0; i < arrTabWidth.GetSize(); ++i) m_maxTabWidth = max(m_maxTabWidth, arrTabWidth[i]);

        for (int i = 0; i < arrExtra.GetSize(); ++i)
        {
            const TExtraTab* pEt = arrExtra[i];
            int nTabs = pEt->m_tabTo - pEt->m_tabFrom;
            int avgTab = (pEt->m_width + nTabs - 1) / nTabs;
            int wTab = 0;

            for (int j = pEt->m_tabFrom; j < pEt->m_tabTo - 1; ++j) wTab += arrTabWidth[j];

            int iTabChange = pEt->m_tabTo - 1;
            arrTabWidth[iTabChange] = max(arrTabWidth[iTabChange], pEt->m_width - wTab);
            m_maxTabWidth = max(m_maxTabWidth, avgTab);
        }

        int wTotal = 0;
        for (int i = 0; i < arrTabWidth.GetSize(); ++i)
        {
            m_arrTab.Add(wTotal);
            wTotal += arrTabWidth[i];
        }

        wTotal -= int(m_pHost->m_padding * m_hFont);
        m_arrTab.Add(wTotal);
        m_szTotal.cx = wTotal;
        m_szTotal.cy = xx.m_ptCurrent.y;
    }
    catch (...)
    {
        arrExtra.DestroyAll();
        throw;
    }

    arrExtra.DestroyAll();
}

int TSuperText::TDrawCache::DrawSpecialStr(
    CDC* pDC, LPCWSTR str, const CPoint& point, bool bBold, bool bItalic, bool bUnderline) const
{
    TDCStateSaver dcss(pDC);
    int width = 0;
    bool bSizeOnly = IsSizeOnly(point);

    for (; *str != 0; ++str)
    {
        int w = m_pHost->DrawSpecial(
            pDC, this, *str, bSizeOnly ? point : CPoint(point.x + width, point.y), bBold, bItalic);
        width += w;
    }
    dcss.Close();

    if (!bSizeOnly && bUnderline) Underline(pDC, point, width);
    return width;
}

void TSuperText::TDrawCache::DrawSegment(
    CDC* pDC, const CPoint& point, const TPage& pageStart, int indexEnd) const
{
    TDCStateSaver dcss(pDC);
    TNextText xx(m_pHost, this, point, pageStart.m_index, indexEnd);
    xx.m_attribute = pageStart.m_attribute;

    while (!xx.IsEof())
    {
        xx.Parse(pDC);
        xx.Draw(pDC);
        xx.Move();
    }
}

void TSuperText::TDrawCache::FlushRich(CRichEditCtrl& wndRich, TStringBufferW& sAdd, CHARFORMAT2& cf)
{
    if (sAdd.IsEmpty()) return;

    wndRich.SetSel(-1, -1);
    CHARRANGE cr;
    wndRich.GetSel(cr);
    int start = cr.cpMax;

    LPARAM lParam = LPARAM(LPCWSTR(sAdd));
    ::SendMessageW(wndRich.m_hWnd, EM_REPLACESEL, WPARAM(false), lParam);
    sAdd.Empty();
    if (cf.cbSize == 0) return;

    wndRich.SetSel(start, -1);
    wndRich.SetSelectionCharFormat(cf);
}

void TSuperText::TDrawCache::UploadRich(
    CDC* pDC, COLORREF colorBackDefault, CRichEditCtrl& wndRich, int length) const
{
    wndRich.SetSel(-1, -1);
    CHARRANGE cr;
    wndRich.GetSel(cr);
    int start = cr.cpMax;

    TDCStateSaver dcss(pDC);
    TNextText xx(m_pHost, this, m_ptSizeOnly, 0, length);

    DWORD attribute = BadAttribute;
    TStringBufferW sAdd;
    CHARFORMAT2 cf;
    xx.FillCharacterFormat(cf);

    while (!xx.IsEof())
    {
        xx.Parse(pDC);
        xx.Draw(pDC);

        if (xx.IsSpecial())
        {
            FlushRich(wndRich, sAdd, cf);
            attribute = BadAttribute;
            wndRich.SetSel(-1, -1);

            CImageAF img;
            img.CreateScreen(xx.m_width, m_hFont);
            img.Fill(xx.m_colorBack == colorAuto ? colorBackDefault : xx.m_colorBack);

            CDC* pDCImg = img.GetCDC();
            xx.ResetColorText(pDCImg);

            pDCImg->SetMapMode(MM_TEXT);
            pDCImg->SetTextAlign(TA_TOP | TA_LEFT);
            pDCImg->SetBkMode(TRANSPARENT);
            DrawSpecialStr(pDCImg, xx.m_str, CPoint(0, 0), xx.IsBold(), xx.IsItalic(), xx.IsUnderline());

            CImageAF::ReleaseCDC(pDCImg);
            CSize sz(xx.m_width * 2540 / m_xLogPix, m_hFont * 2540 / m_yLogPix);

            try
            {
                AddBitmapToRichEdit(wndRich, HBITMAP(img.Detach()), sz);
            }
            catch (CException* pExc)
            {
                pExc->Delete();
            }

            xx.m_str.Empty();
        }
        else
        {
            DWORD newAttribute = xx.m_attribute & atSaved;

            if (newAttribute != attribute)
            {
                FlushRich(wndRich, sAdd, cf);
                attribute = newAttribute;
                xx.FillCharacterFormat(cf);
            }

            if (xx.IsBrk())
            {  // adjust tabs, if more than one
                if (xx.m_brk == brkTab && xx.m_str.GetLength() > 1)
                {
                    int i;
                    for (i = xx.m_tab; i <= xx.m_tabNew; ++i)
                        if (m_arrTab[i] > xx.m_ptCurrent.x) break;
                    xx.m_str.Empty();
                    xx.m_str.AppendCh(chrTabW, xx.m_tabNew - i + 1);
                }
            }
            sAdd += xx.m_str;
        }

        xx.Move();
    }

    FlushRich(wndRich, sAdd, cf);

    if (m_arrTab.GetSize() > 1)
    {
        PARAFORMAT2 pf;
        memset(&pf, 0, sizeof(PARAFORMAT2));
        pf.cbSize = sizeof(PARAFORMAT2);
        pf.dwMask = PFM_LINESPACING | PFM_SPACEAFTER | PFM_SPACEBEFORE | PFM_TABSTOPS;
        pf.dyLineSpacing = m_hFont * 1440 / m_yLogPix;
        pf.bLineSpacingRule = 4;
        pf.cTabCount = min(m_arrTab.GetSize() - 2, MAX_TAB_STOPS);

        for (int i = 0; i < pf.cTabCount; ++i) pf.rgxTabs[i] = m_arrTab[i + 1] * 1440 / m_xLogPix;

        wndRich.SetSel(start, -1);
        wndRich.SetParaFormat(pf);
    }

    wndRich.ReleaseDC(pDC);
    wndRich.SetSel(-1, -1);
}

bool TSuperText::TDrawCache::ProcessNextPage(int pageHeight)
{
    ASSERT(m_pageHard.IsStarted());

    int textLength = m_pHost->GetLength();
    int y = 0;
    int index = m_pageHard.m_index;
    TPage newPageNice;
    TPage newPageHard;
    TStringBufferW str;
    int newHPageExtra = 0;
    bool bTryingNice = m_hPageExtra > 0;

    pageHeight = max(pageHeight - m_hHeader, m_hFont);

    // reached the end already (will be true if the text is empty)
    if (index >= textLength)
    {
        if (m_arrPage.IsEmpty()) m_arrPage.Add(m_pageHard);
        m_arrPage.Add(TPage(textLength, 0));
        ClearPaging();
        return false;
    }

    while (true)
    {
        DWORD attribute = BadAttribute;
        TBrk brk = brkNone;
        bool bEof = index >= textLength;

        if (bEof)
            brk = brkPage;
        else
        {
            if (!m_pHost->ReadNextToken(index, textLength, attribute, str)) continue;

            // Got an attribute
            brk = GetBrk(attribute);
            if (brk < brkEol) continue;
        }

        // EOL or end of page!
        TPage page(index, attribute);
        int ySkip = m_pHost->GetSkipCount(brk) * m_hFont;
        y += ySkip;
        newPageHard = page;

        // new page end -- nice?
        if (brk >= brkPage)
        {
            newPageNice = newPageHard;
            newHPageExtra = 0;
        }
        else
        {
            if (newPageNice.IsStarted()) newHPageExtra += ySkip;
        }

        if (bTryingNice)
        {
            if (y <= pageHeight - m_hPageExtra - m_hFont) continue;
            if (newPageNice.IsStarted()) break;
            bTryingNice = false;
        }

        if (bEof || y > pageHeight - m_hFont) break;
    }

    if (bTryingNice)
        m_arrPage.Add(m_pageNice);
    else
        m_arrPage.Add(m_pageHard);

    m_pageNice = newPageNice.IsStarted() ? newPageNice : newPageHard;
    m_pageHard = newPageHard;
    m_hPageExtra = newHPageExtra;

    // more real text available?
    if (m_pageHard.m_index < m_pHost->GetRealLength()) return true;

    // no!
    m_arrPage.Add(TPage(textLength, 0));
    ClearPaging();
    return false;
}
