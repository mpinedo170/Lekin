#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgAbout.h"

#include "Draws.h"
#include "Misc.h"

namespace {
const CString keyCredits = _T("Credits");
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAbout dialog

CDlgAbout::CDlgAbout() : super(IDD)
{
    m_section = secSettings;
    AddDU(new TChkDataUnit(keyCredits, IDC_SHOWAGAIN, m_bShowAgain, true, true));
    m_bShowAnyway = false;
}

void CDlgAbout::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SHOWAGAIN, m_btnShowAgain);
    DDX_Control(pDX, IDOK, m_btnOk);
}

BEGIN_MESSAGE_MAP(CDlgAbout, super)
ON_WM_PAINT()
END_MESSAGE_MAP()

static const LPCTSTR Credits[][3] = {{strDash, strEmpty},
    {_T("Developed for:"), _T("Scheduling: Theory, Algorithms, and Systems."), strEmpty},
    {NULL, _T("Michael L. Pinedo")}, {NULL, _T("Prentice Hall, 1995")},
    {strEmpty, _T("Operations Scheduling with Applications"), strEmpty},
    {NULL, _T("in Manufacturing and Services."), strEmpty}, {NULL, _T("Michael L. Pinedo, Xiuli Chao")},
    {NULL, _T("Irwin/McGraw-Hill, 1998")}, {strEmpty, _T("Operations Management."), strEmpty},
    {NULL, _T("J. Heizer and B. Render")}, {NULL, _T("Prentice-Hall,  2006")}, {strDash, strEmpty},
    {_T("Project supervision:"), _T("Michael Pinedo (mpinedo@stern.nyu.edu)")},
    {_T("System Development:"), _T("Andrew Feldman (jd850@hotmail.com)")},
    {_T("Plug-in Algorithms:"), _T("Nutthapol Assadathorn (lekin@loxinfo.co.th)")},
    {NULL, _T("Stephan Kreipl (stephan.kreipl@sap-ag.de)")},
    {NULL, _T("Marcos Singer (singer@volcan.facea.puc.cl)")},
    {NULL, _T("Alkis Vazacopoulos (alkis1256@aol.com)")}, {NULL, _T("Ya Yang")}, {strDash, strEmpty},
    {_T("Features:"), _T("Various types of production environments.")},
    {NULL, _T("Gantt chart with drag-and-drop support.")}, {NULL, _T("Comparative analysis of heuristics.")},
    {NULL, _T("User heuristic attachment.")}, {NULL, _T("Complete graphic printouts.")}, {strDash, strEmpty},
    {NULL, NULL}};

CSize CDlgAbout::Draw(CDC* pDC)
{
    m_marginY = GetSystemMetrics(SM_CYCAPTION);
    m_marginX = m_marginY * 2;

    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextAlign(TA_TOP | TA_LEFT);
    CImageAF imgHeadline;
    imgHeadline.LoadNeatBitmap(IDB_INTRO, CImageAF::bmcWindow);
    CSize head_sz = imgHeadline.GetSize();

    CFont font0;
    CreateLogicalFont(font0, pDC, strFntTimes, 8, false, false);
    CFont font1;
    CreateLogicalFont(font1, pDC, strFntTimes, 9, false, false);
    CFont font2;
    CreateLogicalFont(font2, pDC, strFntTimes, 9, false, true);
    CFont font3;
    CreateLogicalFont(font3, pDC, strFntTimes, 9, true, true);
    CFont* pFontOld = pDC->SelectObject(&font0);

    int dy = int(GetFontHeight(pDC, &font0) * 1.2);
    int y = m_marginY;

    int w1 = 0, w2 = 0;
    for (int i = 0; Credits[i][1] != NULL; ++i)
    {
        LPCTSTR s = Credits[i][0];
        if (s) w1 = max(w1, int(GetFontSize(pDC, &font1, s).cx));
        s = Credits[i][1];
        if (s) w2 = max(w2, int(GetFontSize(pDC, &font3, s).cx));
    }
    w1 += w1 / 5;

    CSize total_sz(w1 + w2 + 2 * m_marginX, 0);

    imgHeadline.Draw(*pDC, total_sz.cx / 4 - head_sz.cx / 2, m_marginY);
    pDC->SelectObject(&font0);

    CString tops[4];
    tops[0] = _T("Version ") + GetVersionInfo(2);
    tops[2].LoadString(IDS_COPYRIGHT);

    CString s;
    if (License != _T("Demo")) s.Format(_T(" (%dx%dx%d)"), MaxJob, MaxWkc, MaxMch);
    tops[1] = _T("License: ") + License + s;

    for (int i = 0;; ++i)
    {
        s = tops[i];
        if (s.IsEmpty()) break;
        CSize sz = pDC->GetTextExtent(s);
        pDC->TextOut(total_sz.cx * 3 / 4 - sz.cx / 2, y, s);
        y += sz.cy;
    }

    y = max(y, int(m_marginY + head_sz.cy));

    for (int i = 0; Credits[i][1] != 0; ++i)
    {
        LPCTSTR s = Credits[i][0];
        if (s)
            if (!_tcscmp(s, strDash))
                Draw3dLine(pDC, 0, total_sz.cx, y + dy / 2);
            else
            {
                y += dy / 3;
                pDC->SelectObject(&font1);
                pDC->TextOut(m_marginX, y, s);
            }

        pDC->SelectObject(Credits[i][2] ? &font3 : &font2);
        pDC->TextOut(m_marginX + w1, y, Credits[i][1]);
        y += dy;
    }

    CRect rect;
    GetDlgItem(IDOK)->GetWindowRect(rect);
    total_sz.cy = y - dy / 2 + rect.Height() + 2 * m_marginY;

    pDC->SelectObject(pFontOld);
    return total_sz;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAbout message handlers

void CDlgAbout::OnPaint()
{
    CPaintDC dc(this);
    Draw(&dc);
}

BOOL CDlgAbout::OnInitDialog()
{
    super::OnInitDialog();
    if (!m_bShowAnyway && !m_bShowAgain)
    {
        EndDialog(IDOK);
        return false;
    }

    CDC dc;
    dc.CreateCompatibleDC(NULL);
    CSize sz = Draw(&dc);
    int cx = sz.cx + 2 * GetSystemMetrics(SM_CXDLGFRAME);
    int cy = sz.cy + 2 * GetSystemMetrics(SM_CYDLGFRAME);
    MoveWindow(
        (GetSystemMetrics(SM_CXSCREEN) - cx) / 2, (GetSystemMetrics(SM_CYSCREEN) - cy) / 2, cx, cy, false);

    CRect rect;
    m_btnOk.GetWindowRect(rect);
    VERIFY(m_btnOk.SetWindowPos(NULL, sz.cx - m_marginX - rect.Width(), sz.cy - m_marginY - rect.Height(), -1,
        -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE));

    m_btnShowAgain.GetWindowRect(rect);
    VERIFY(m_btnShowAgain.SetWindowPos(NULL, m_marginX, sz.cy - m_marginY - rect.Height(), -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE));

    return true;
}
