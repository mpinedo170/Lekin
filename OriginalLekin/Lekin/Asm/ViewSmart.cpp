#include "StdAfx.h"

#include "AppAsm.h"
#include "ViewSmart.h"

#include "DocSmart.h"
#include "DocTmpSmart.h"
#include "FrmSmart.h"

#include "Draws.h"

/////////////////////////////////////////////////////////////////////////////
// CViewSmart

IMPLEMENT_DYNCREATE(CViewSmart, CScrollView)

CViewSmart::CViewSmart()
{
    m_bBarX = m_bBarY = false;
    m_pWndParent = NULL;
    m_yTop = m_yBottom = 0;
    m_pageCount = 0;
    m_page = -1;
    m_pDDPaint = m_pDDPrint = NULL;
    m_pFontTitle = NULL;
}

CViewSmart::~CViewSmart()
{
    delete m_pDDPaint;
    m_pDDPaint = NULL;
    delete m_pDDPrint;
    m_pDDPrint = NULL;
    delete m_pFontTitle;
    m_pFontTitle = NULL;
}

BEGIN_MESSAGE_MAP(CViewSmart, super)
ON_WM_SIZE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONDBLCLK()
ON_WM_ERASEBKGND()
ON_COMMAND(ID_FILE_NEW, OnFileNew)
ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
ON_COMMAND(ID_FILE_APP, OnFileApp)
ON_COMMAND(ID_FILE_SAVE, OnFileSave)
ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFile)
ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFile)
ON_UPDATE_COMMAND_UI(ID_FILE_APP, OnUpdateFile)
ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSave)
ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_DIRECT, OnUpdateFilePrint)
ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrint)
ON_WM_PAINT()
ON_UPDATE_COMMAND_UI_RANGE(ID_WIN0, ID_WIN5, OnUpdateWin)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSmart drawing

void CViewSmart::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    Update();
}

void CViewSmart::OnPaint()
{
    CPaintDC dc(this);

    CRect rect;
    dc.GetClipBox(&rect);
    if (rect.IsRectEmpty()) return;

    CSize sz = rect.Size();
    CImageAF img;
    img.CreateScreen(sz);
    img.Fill(colorNormal());
    CDC* pDCImg = img.GetCDC();

    pDCImg->SetTextColor(colorText());
    pDCImg->SetBkMode(TRANSPARENT);

    pDCImg->SetViewportOrg(-(rect.TopLeft() + GetDeviceScrollPosition()));
    CRect rectCli;
    GetClientRect(rectCli);
    DrawView(pDCImg, rectCli.Size(), m_pDDPaint);
    pDCImg->SetViewportOrg(0, 0);

    img.ReleaseCDC(pDCImg);
    img.BitBlt(dc, rect.TopLeft());
}

void CViewSmart::OnDraw(CDC* pDC)
{}

BOOL CViewSmart::OnEraseBkgnd(CDC* pDC)
{
    return true;
}

void CViewSmart::Update()
{
    ScreenUpdate();
}

CDocTmpSmart* CViewSmart::GetTmp()
{
    return (CDocTmpSmart*)GetDocument()->GetDocTemplate();
}

CDocTmpSmart* CViewSmart::GetFileTmp()
{
    return GetTmp();
}

CViewSmart::TDDSmart* CViewSmart::CreateDD(bool bPrinting)
{
    return new TDDSmart(bPrinting);
}

bool CViewSmart::IsVisible()
{
    return !m_pWndParent->IsIconic() && m_pWndParent->IsWindowVisible();
}

/////////////////////////////////////////////////////////////////////////////
// CViewSmart message handlers

BOOL CViewSmart::Create(LPCTSTR lpszClassName,
    LPCTSTR lpszWindowName,
    DWORD dwStyle,
    const RECT& rect,
    CWnd* pParentWnd,
    UINT nID,
    CCreateContext* pContext)
{
    m_pWndParent = (CFrmSmart*)pParentWnd;
    if (!CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext)) return false;
    m_pWndParent->m_pView = this;
    return true;
}

CRect CViewSmart::GetViewRect(CDC* pDC, const CRect& rectFrame, TDDSmart* pDD)
{
    return rectFrame;
}

void CViewSmart::DrawHeader(THeader header, CDC* pDC, CSize sz, TDDSmart* pDD)
{}

void CViewSmart::DrawView(CDC* pDC, CSize sz, TDDSmart* pDD)
{}

void CViewSmart::DrawMisc(CDC* pDC, TDDSmart* pDD)
{}

CRect CViewSmart::GetHeaderRect(const CRect& rectFrame, const CRect& rectView, THeader header, bool bPrinting)
{
    if (header == Center) return rectView;

    int dx, dy, sbx, sby, rg, bt;
    if (bPrinting)
        dx = dy = sbx = sby = rg = bt = 0;
    else
    {
        CRect rect;
        GetWindowRect(rect);
        ScreenToClient(rect);
        dx = -rect.left;
        dy = -rect.top;
        sbx = GetSystemMetrics(SM_CXHTHUMB);
        sby = GetSystemMetrics(SM_CYVTHUMB);
        rg = m_bBarY ? sbx : 0;
        bt = m_bBarX ? sby : 0;
    }

    switch (header)
    {
        case Top:
            return CRect(rectView.left + dx, rectFrame.top + 1, rectView.right - dx, rectView.top);
        case Bottom:
            return CRect(rectView.left + dx + sbx, rectView.bottom, rectView.right - dx - rg - sbx,
                rectFrame.bottom - 1);
        case Left:
            return CRect(rectFrame.left + 1, rectView.top + dx, rectView.left, rectView.bottom - dx);
        case Right:
            return CRect(
                rectView.right, rectView.top + sby, rectFrame.right - 1, rectView.bottom - dx - bt - sbx);
    }
    return CRect(0, 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CViewSmart printing

T3State CViewSmart::ViewExUpdateFg(CDC* pDC, const TMargin* pMargin)
{
    if (m_pDDPrint == NULL) m_pDDPrint = CreateDD(true);
    if (m_pFontTitle != NULL) delete m_pFontTitle;

    m_pageCount = 0;
    m_pFontTitle = new CFont;
    CreateCourierFont(*m_pFontTitle, pDC);
    m_rectPage = pMargin->ComputeRect(pDC);
    if (m_rectPage.IsRectEmpty()) return false;

    m_yTop = GetFontHeight(pDC, m_pFontTitle) * 3;
    m_yBottom = GetPrintYBottom(pDC, m_pDDPrint, pMargin);

    m_rectPage.top += m_yTop;
    m_rectPage.bottom -= m_yBottom;
    m_rectCenter = GetViewRect(pDC, m_rectPage, m_pDDPrint);
    m_pageCount = OnPrintUpdate(pDC, m_pDDPrint, pMargin);
    ViewExSetPageMax(m_pageCount);
    return m_pageCount > 0;
}

void CViewSmart::ViewExPrintPage(CDC* pDC, int page)
{
    if (m_pageCount <= 0) return;

    CFont* pFontOld = pDC->SelectObject(m_pFontTitle);
    pDC->SetTextColor(colorBlack);
    pDC->SetBkMode(TRANSPARENT);

    m_page = page;
    pDC->SetTextAlign(TA_CENTER | TA_TOP);
    CString s;
    GetParent()->GetWindowText(s);
    pDC->TextOut(m_rectPage.CenterPoint().x, m_rectPage.top - m_yTop, s);
    PrintBottom(pDC, m_pDDPrint);
    pDC->SelectObject(pFontOld);

    for (int i = 0; i < 4; ++i)
    {
        CRect rect = GetHeaderRect(m_rectPage, m_rectCenter, THeader(i), true);
        if (rect.IsRectEmpty()) continue;
        pDC->SetViewportOrg(rect.TopLeft());
        DrawHeader(CViewSmart::THeader(i), pDC, rect.Size(), m_pDDPrint);
    }

    pDC->SetViewportOrg(m_rectCenter.TopLeft());
    DrawView(pDC, m_rectCenter.Size(), m_pDDPrint);
}

BOOL CViewSmart::OnPreparePrinting(CPrintInfo* pInfo)
{
    m_pMargin = GetTmp()->m_pMargin;
    return ViewExPreparePrinting(pInfo, m_pMargin, GetTmp()->CreateDlgPrint(false));
}

int CViewSmart::OnPrintUpdate(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin)
{
    return 1;
}

int CViewSmart::GetPrintYBottom(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin)
{
    return 0;
}

void CViewSmart::PrintBottom(CDC* pDC, TDDSmart* pDD)
{}

void CViewSmart::PrintPageNo(CDC* pDC)
{
    pDC->SetTextAlign(TA_CENTER | TA_TOP);
    pDC->TextOut(m_rectPage.CenterPoint().x,
        m_rectPage.bottom + (m_yBottom - pDC->GetTextExtent(_T("W")).cy) / 2, IntToStr(m_page));
}

/////////////////////////////////////////////////////////////////////////////
// Resizing

CSize CViewSmart::OnScreenUpdate(CDC* pDC, TDDSmart* pDD)
{
    return CSize(1, 1);
}

void CViewSmart::ScreenUpdate()
{
    if (m_pDDPaint == NULL) m_pDDPaint = CreateDD(false);

    CRect rect;
    GetClientRect(rect);
    CSize szCli = rect.Size();
    CSize szTotal(1, 1);

    if (szCli.cx > 0 && szCli.cy > 0)
    {
        CDC* pDC = GetDC();
        szTotal = OnScreenUpdate(pDC, m_pDDPaint);
        ReleaseDC(pDC);
    }

    SetScrollSizes(MM_TEXT, szTotal, szCli);
    m_bBarX = szCli.cx < szTotal.cx;
    m_bBarY = szCli.cy < szTotal.cy;

    m_pWndParent->RedrawWindow();
    RedrawWindow();
}

void CViewSmart::OnSize(UINT nType, int cx, int cy)
{
    super::OnSize(nType, cx, cy);
    ScreenUpdate();
}

/////////////////////////////////////////////////////////////////////////////
// Mouse

void CViewSmart::OnLButtonDown(UINT nFlags, CPoint point)
{
    Click(nFlags, point + GetDeviceScrollPosition());
}

void CViewSmart::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    DblClick(nFlags, point + GetDeviceScrollPosition());
}

void CViewSmart::Click(UINT nFlags, CPoint point)
{}

void CViewSmart::DblClick(UINT nFlags, CPoint point)
{}

BOOL CViewSmart::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
    CPoint p1 = GetScrollPosition();
    BOOL bOk = super::OnScroll(nScrollCode, nPos, bDoScroll);
    CPoint p2 = GetScrollPosition();
    m_pWndParent->Redraw(p1.x != p2.x, p1.y != p2.y);
    return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// Commands

void CViewSmart::OnFileNew()
{
    CDocTmpSmart* pTmp = GetFileTmp();
    if (!pTmp) return;
    pTmp->OpenDocumentFile(NULL);
}

void CViewSmart::OnFileOpen()
{
    CDocTmpSmart* pTmp = GetFileTmp();
    if (!pTmp) return;
    pTmp->PromptOpen();
}

void CViewSmart::OnFileApp()
{
    CDocTmpSmart* pTmp = GetFileTmp();
    if (!pTmp) return;

    CString lpszPathName = pTmp->Prompt();
    if (lpszPathName.IsEmpty()) return;
    if (pTmp->GetID() != tvwSeq)
        if (!theApp.CheckSequences(true)) return;
    pTmp->AppendFile(lpszPathName);
    pTmp->Modify();
}

void CViewSmart::OnFileSave()
{
    CDocTmpSmart* pTmp = GetFileTmp();
    if (!pTmp) return;
    pTmp->GetDoc()->Save();
}

void CViewSmart::OnFileSaveAs()
{
    CDocTmpSmart* pTmp = GetFileTmp();
    if (!pTmp) return;
    pTmp->GetDoc()->SaveAs();
}

void CViewSmart::OnUpdateFile(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetFileTmp() != NULL);
}

void CViewSmart::OnUpdateFileSave(CCmdUI* pCmdUI)
{
    CDocTmpSmart* pTmp = GetFileTmp();
    pCmdUI->Enable(pTmp && !pTmp->GetDoc()->IsEmpty());
}

void CViewSmart::OnUpdateFilePrint(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!GetTmp()->GetDoc()->IsEmpty());
}

void CViewSmart::OnUpdateWin(CCmdUI* pCmdUI)
{
    int id = pCmdUI->m_nID - ID_WIN0;
    pCmdUI->SetCheck(id == GetTmp()->GetID());
}
