#include "StdAfx.h"

#include "AFLibDlgPrint.h"

#include "AFLibGlobal.h"
#include "AFLibMargin.h"
#include "AFLibResource.h"
#include "AFLibViewEx.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using AFLibGui::CDlgPrint;

// 1026 is the "Printer" button in a regular Page Setup dialog box
// We simulate a click on that button in order to read the list of printers,
//   change printer, or allow the user to change print "properties"
//   (e.g., select gray-only on a color printer)
namespace {
const int IDC_PRINTER_P = 1026;
const CString keyPrintToFile = _T("Print to file");
}  // namespace

CDlgPrint* CDlgPrint::m_pDlgHook;

IMPLEMENT_DYNAMIC(CDlgPrint, super)

CDlgPrint::CDlgPrint(bool bSetupOnly, CWnd* pParentWnd) : super(bSetupOnly, pParentWnd)
{
    if (!bSetupOnly) m_pd.Flags |= PD_RETURNDC;

    m_pMargin = m_pMarginCur = NULL;
    m_pViewEx = NULL;
    m_pSldPage = new CSliderCtrl;
    m_rectFull.SetRectEmpty();

    m_bSaveMargin = true;
    m_bShowApply = !bSetupOnly;

    m_bPrintToFile = false;
    m_bDoModalCalled = false;

    m_bApply = bSetupOnly;
    m_bValid = true;
    m_bPageDrawnReady = true;
    m_bChanged = true;

    SetPageMax(0);
    SetPages(0, 0);
    m_pageDrawn = 0;
    m_action = mpaNon;
    m_hHook = NULL;

    memset(&m_psd, 0, sizeof(m_psd));
    m_psd.lStructSize = sizeof(m_psd);
    m_psd.Flags = (PSD_MARGINS | PSD_INWININIINTLMEASURE | PSD_ENABLEPAGESETUPTEMPLATE |
                   PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGEPAINTHOOK);
    m_psd.lpPageSetupTemplateName = LPCWSTR(MAKEINTRESOURCE(bSetupOnly ? IDD_PAGE_SETUP_EX : IDD_PRINT_EX));
    m_psd.hInstance = AFLibDLL.hModule;
}

CDlgPrint::~CDlgPrint()
{
    delete m_pMarginCur;
    delete m_pSldPage;

    if (m_psd.hDevMode != NULL)
    {
        GlobalFree(m_psd.hDevMode);
        m_psd.hDevMode = NULL;
    }

    if (m_psd.hDevNames != NULL)
    {
        GlobalFree(m_psd.hDevNames);
        m_psd.hDevNames = NULL;
    }
}

CString CDlgPrint::GetToolTip(UINT idc)
{
    bool bOk = idc == IDOK;

    static const struct
    {
        UINT m_idcFrom;
        UINT m_idcTo;
    } arrConversion[] = {{IDC_PRINTER_MAR_TOP, IDC_PRINTER_MAR_LEFT},
        {IDC_PRINTER_MAR_RIGHT, IDC_PRINTER_MAR_LEFT}, {IDC_PRINTER_MAR_BOTTOM, IDC_PRINTER_MAR_LEFT},
        {IDC_PRINTER_LANDSCAPE, IDC_PRINTER_PORTRAIT}, {IDC_PRINTER_PAGE_FROM, IDC_PRINTER_PAGE0 + selFrom},
        {IDC_PRINTER_PAGE_TO, IDC_PRINTER_PAGE0 + selFrom}, {IDOK, IDC_PRINTER_APPLY},
        {IDCANCEL, IDS_PRINTER_CANCEL}, {0, 0}};

    for (int i = 0; arrConversion[i].m_idcFrom != 0; ++i)
        if (arrConversion[i].m_idcFrom == idc)
        {
            idc = arrConversion[i].m_idcTo;
            break;
        }

    switch (idc)
    {
        case IDC_PRINTER_PREVIEW:
            return LocalAfxString(idc, IntToStr(m_pageDrawn));

        case IDC_PRINTER_APPLY:
        {
            if (IsSetupOnly()) break;
            CString s = LocalAfxString(bOk ? IDS_PRINTER_OK : IDS_PRINTER_APPLY_ONLY);
            return LocalAfxString(idc, s);
        }
    }

    return LocalAfxString(idc);
}

void CDlgPrint::Action()
{
    CComboBox* pCmb2 = (CComboBox*)m_wndSpec.GetDlgItem(1139);
    int count = pCmb2->GetCount();

    for (int i = 0; i < count; ++i)
    {
        CString s;
        pCmb2->GetLBText(i, s);
        m_miniPrintData.m_arrAllPrint.Add(s);
    }

    switch (m_action)
    {
        case mpaSet:
        {
            int i = count;

            while (--i >= 0)
                if (m_miniPrintData.m_arrAllPrint[i] == m_miniPrintData.m_printer) break;

            if (i >= 0)
            {
                pCmb2->SetCurSel(i);
                m_wndSpec.SendMessage(WM_COMMAND, MAKEWPARAM(1139, CBN_SELCHANGE));
            }
            break;
        }

        case mpaPro:
            m_wndSpec.SendMessage(WM_COMMAND, MAKEWPARAM(1025, BN_CLICKED));
            break;
    }

    m_miniPrintData.m_curIndex = pCmb2->GetCurSel();
    m_wndSpec.GetDlgItemText(1099, m_miniPrintData.m_status);
    m_wndSpec.GetDlgItemText(1098, m_miniPrintData.m_type);
    m_wndSpec.GetDlgItemText(1101, m_miniPrintData.m_location);
    m_wndSpec.SendMessage(WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED));
}

void CDlgPrint::ProcFilter(CWPRETSTRUCT& cwp)
{
    if (m_wndSpec.m_hWnd == NULL)
    {
        if (cwp.message != WM_CREATE) return;
        CREATESTRUCT& cs = *reinterpret_cast<CREATESTRUCT*>(cwp.lParam);

        if (cs.hwndParent != m_hWnd) return;
        if (int(cs.lpszClass) != 0x8002) return;
        m_wndSpec.Attach(cwp.hwnd);
    }

    if (cwp.hwnd == m_wndSpec.m_hWnd && cwp.message == WM_INITDIALOG) Action();
}

LRESULT CALLBACK CDlgPrint::ProcFilter2(int code, WPARAM wParam, LPARAM lParam)
{
    HHOOK hHook = NULL;

    if (m_pDlgHook != NULL)
    {
        hHook = m_pDlgHook->m_hHook;
        if (code == HC_ACTION)
        {
            CWPRETSTRUCT& cwp = *reinterpret_cast<CWPRETSTRUCT*>(lParam);
            m_pDlgHook->ProcFilter(cwp);
        }
    }
    return ::CallNextHookEx(hHook, code, wParam, lParam);
}

void CDlgPrint::GoP(CMiniPrintAction action)
{
    m_action = action;

    m_miniPrintData.m_printer.Empty();
    m_miniPrintData.m_arrAllPrint.RemoveAll();
    m_miniPrintData.m_curIndex = 0;
    m_miniPrintData.m_status.Empty();
    m_miniPrintData.m_type.Empty();
    m_miniPrintData.m_location.Empty();

    if (m_action == mpaSet)
    {
        int iSel = m_cmbPrinter.GetCurSel();
        if (iSel == -1) iSel = 0;
        m_cmbPrinter.GetLBText(iSel, m_miniPrintData.m_printer);
    }

    CWnd* pWnd = GetFocus();
    SendMessage(WM_COMMAND, MAKEWPARAM(IDC_PRINTER_P, BN_CLICKED));

    if (action == mpaGet)
    {
        m_cmbPrinter.ResetContent();
        for (int i = 0; i < m_miniPrintData.m_arrAllPrint.GetSize(); ++i)
            m_cmbPrinter.AddString(m_miniPrintData.m_arrAllPrint[i]);
        m_cmbPrinter.SetCurSel(m_miniPrintData.m_curIndex);
    }

    SetDlgItemText(IDC_PRINTER_STATUS, m_miniPrintData.m_status);
    SetDlgItemText(IDC_PRINTER_TYPE, m_miniPrintData.m_type);
    SetDlgItemText(IDC_PRINTER_WHERE, m_miniPrintData.m_location);

    if (pWnd != NULL) pWnd->SetFocus();
    m_action = mpaNon;
}

void CDlgPrint::SelectSel(TSel sel)
{
    SetDlgRadioButton(IDC_PRINTER_PAGE0, selN, sel);
}

bool CDlgPrint::IsSelected(TSel sel)
{
    return IsDlgButtonChecked(IDC_PRINTER_PAGE0 + sel) == 1;
}

void CDlgPrint::EnablePages()
{
    static const UINT arrIDC[] = {IDC_PRINTER_PAGE_FROM, IDC_PRINTER_LAB0, IDC_PRINTER_PAGE_TO, 0};

    if (IsSetupOnly()) return;
    EnableDlgItems(arrIDC, IsSelected(selFrom));
}

void CDlgPrint::LoadPages()
{
    if (IsSetupOnly())
    {
        SetPages(0, 0);
        OnChangePage();
        return;
    }

    if (!m_bValid)
    {
        OnChangePage();
        return;
    }

    bool bOne = m_pageMax == 1;

    SetDlgItemText(IDC_PRINTER_PAGE0 + selAll,
        LocalAfxString(
            bOne || m_pageMax == TViewEx::pageMaxHigh || IsSelected(selSel) ? IDS_ALL : IDS_ALL_PAGES,
            IntToStr(m_pageMax)));

    EnableDlgItem(IDC_PRINTER_PAGE0 + selFrom, !bOne);

    if (bOne || !IsSelected(selFrom))
        SetPages(0, 0);
    else
        SetPages(m_pageFrom, m_pageTo);

    if (bOne)
    {
        if (IsSelected(selFrom)) SelectSel(selAll);
        EnablePages();
        SetDlgItemText(IDC_PRINTER_PAGE_FROM, strEmpty);
        SetDlgItemText(IDC_PRINTER_PAGE_TO, strEmpty);
        return;
    }

    SetDlgItemInt(IDC_PRINTER_PAGE_FROM, m_pageFrom);
    if (m_pageTo == TViewEx::pageMaxHigh)
        SetDlgItemText(IDC_PRINTER_PAGE_TO, strEmpty);
    else
        SetDlgItemInt(IDC_PRINTER_PAGE_TO, m_pageTo);

    EnablePages();
}

UINT CDlgPrint::SavePages()
{
    if (IsSetupOnly() || !IsSelected(selFrom))
    {
        SetPages(0, 0);
        return 0;
    }

    BOOL bTemp = false;
    CString s;
    GetDlgItemText(IDC_PRINTER_PAGE_TO, s);

    int pageTo = s.IsEmpty() ? m_pageMax : GetDlgItemInt(IDC_PRINTER_PAGE_TO, &bTemp);
    return SetPages(GetDlgItemInt(IDC_PRINTER_PAGE_FROM, &bTemp), pageTo) ? 0 : IDC_PRINTER_PAGE_FROM;
}

bool CDlgPrint::IsSetupOnly()
{
    return m_bApply;
}

void CDlgPrint::LoadAdditionalData()
{
    CheckDlgButton(IDC_PRINTER_FILE, m_bPrintToFile);
}

UINT CDlgPrint::SaveAdditionalData()
{
    m_bPrintToFile = IsDlgButtonChecked(IDC_PRINTER_FILE) == 1;
    m_pMarginCur->m_bSelection = IsSelected(selSel);
    return 0;
}

void CDlgPrint::SetChangedFlag()
{
    m_bChanged = true;
}

bool CDlgPrint::SetPageMax(int pageMax)
{
    if (pageMax > 0)
    {
        m_pageMax = pageMax;
        return true;
    }
    else
    {
        m_pageMax = TViewEx::pageMaxHigh;
        return false;
    }
}

bool CDlgPrint::SetPages(int pageFrom, int pageTo)
{
    bool bOk = true;
    m_pageFrom = pageFrom;
    m_pageTo = pageTo;

    if (m_pageFrom < 1 || m_pageFrom > m_pageMax)
    {
        bOk = false;
        m_pageFrom = 1;
    }

    if (m_pageTo < m_pageFrom || m_pageTo > m_pageMax)
    {
        bOk = false;
        m_pageTo = m_pageMax;
    }

    return bOk;
}

int CDlgPrint::GetPageMax()
{
    return m_pageMax;
}

void CDlgPrint::GetPages(int& pageFrom, int& pageTo)
{
    pageFrom = m_pageFrom;
    pageTo = m_pageTo;
}

bool CDlgPrint::CheckValid()
{
    if (m_bValid) return true;
    LocalAfxMB(IDS_BAD_PRINTER_SETTINGS);
    return false;
}

void CDlgPrint::RedrawPage(bool bRecompute)
{
    if (m_pViewEx == NULL) return;

    if (bRecompute)
        SetChangedFlag();
    else if (!m_bValid || m_pSldPage->GetPos() == m_pageDrawn)
        return;

    GetDlgItem(IDC_PRINTER_PREVIEW)->RedrawWindow();
}

HDC CDlgPrint::CreateTempDC()
{
    if (m_psd.hDevNames == NULL) return NULL;

    LPDEVNAMES lpDevNames = reinterpret_cast<LPDEVNAMES>(GlobalLock(m_psd.hDevNames));

    LPDEVMODEW pDevMode = m_psd.hDevMode == NULL ? NULL : LPDEVMODEW(GlobalLock(m_psd.hDevMode));

    HDC hDC = CreateDCW(LPCWSTR(lpDevNames) + lpDevNames->wDriverOffset,
        LPCWSTR(lpDevNames) + lpDevNames->wDeviceOffset, LPCWSTR(lpDevNames) + lpDevNames->wOutputOffset,
        pDevMode);

    GlobalUnlock(m_psd.hDevNames);
    if (m_psd.hDevMode != NULL) GlobalUnlock(m_psd.hDevMode);
    return hDC;
}

void CDlgPrint::ReplaceByFileHelper(LPDEVNAMES lpDevNames, int bufSize)
{
    static const LPCTSTR strFile = _T("FILE:");
    int len = _tcslen(strFile) + 1;
    int offset = lpDevNames->wOutputOffset;

    // we just hope that there is space to fit strFile.
    // if not, just silently return and hope the user won't notice

    if (StringStartsWithIC(LPCTSTR(lpDevNames) + offset, strFile)) return;
    if ((offset + len) * int(sizeof(TCHAR)) > bufSize) return;

    memcpy(LPTSTR(lpDevNames) + offset, LPCTSTR(strFile), len * sizeof(TCHAR));
}

void CDlgPrint::ReplaceByFile()
{
    LPDEVNAMES lpDevNames = LPDEVNAMES(GlobalLock(m_pd.hDevNames));
    int bufSize = GlobalSize(m_pd.hDevNames);

    ReplaceByFileHelper(lpDevNames, bufSize);
    GlobalUnlock(m_pd.hDevNames);
}

void CDlgPrint::DrawPage(HDC hDC)
{
    HDC hDC2 = CreateTempDC();
    if (hDC2 == NULL) return;

    int nSavedDC = SaveDC(hDC);
    CSize sz(GetDeviceCaps(hDC2, HORZRES), GetDeviceCaps(hDC2, VERTRES));
    CDC* pDC = m_pViewEx->ViewExCreateDC(hDC, hDC2, m_rectFull, sz);

    if (m_bChanged)
    {
        CWaitCursor wc;
        m_bChanged = false;
        SavePages();

        m_bValid = SaveAdditionalData() == 0;
        int nPages = 0;

        if (m_bValid)
        {
            m_pMarginCur->Load(&m_psd);
            m_pViewEx->ViewExUpdate(pDC, m_pMarginCur);

            if (!m_pViewEx->ViewExHasUpdateFinished()) SetTimer(timerUpdate, 50, NULL);

            nPages = m_pViewEx->ViewExGetPageMax();
            m_bValid = !m_pViewEx->ViewExHasUpdateFailed();
        }

        if (m_bValid) SetPageMax(nPages);

        PostMessage(WM_TIMER, timerLoad);
    }

    bool bValid2 = m_bValid;
    if (bValid2)
    {
        int pageDrawn = m_pSldPage->GetPos();
        if (pageDrawn <= 0)
            bValid2 = false;
        else
        {
            m_bPageDrawnReady = m_pViewEx->ViewExIsPageReady(pageDrawn);
            if (m_bPageDrawnReady) m_pViewEx->ViewExPrintPage(pDC, pageDrawn);
            m_pageDrawn = pageDrawn;
        }
    }

    if (!bValid2) m_pageDrawn = -1;
    m_pViewEx->ViewExDeleteDC(pDC);
    RestoreDC(hDC, nSavedDC);
    DeleteDC(hDC2);
}

void CDlgPrint::PD2PSD()
{
    m_psd.lpfnPageSetupHook = m_pd.lpfnPrintHook;
    m_psd.lpfnPagePaintHook = PaintHookProc;

    SetPageMax(m_pd.nMaxPage);
    SetPages(m_pd.nFromPage, m_pd.nToPage);

    m_pMarginCur->Save(&m_psd);
    m_bPrintToFile = (m_pd.Flags & PD_PRINTTOFILE) != 0;
}

void CDlgPrint::PrivatePD2PSD()
{
    if (m_pMargin == NULL)
    {
        m_pMarginCur = new TMargin(strEmpty);
        m_pMarginCur->m_bSelection = (m_pd.Flags & PD_SELECTION) != 0;
    }
    else
        m_pMarginCur = m_pMargin->Clone();

    if (m_pViewEx != NULL && m_pViewEx->ViewExSelectionState() != TViewEx::sbsEnable)
        m_pMarginCur->m_bSelection = false;

    m_psd.hDevMode = DevMode2W(m_pd.hDevMode);
    m_psd.hDevNames = DevNames2W(m_pd.hDevNames);
    PD2PSD();
}

void CDlgPrint::PSD2PD()
{
    m_pd.nMinPage = 1;
    m_pd.nMaxPage = m_pageMax;
    m_pd.nFromPage = m_pageFrom;
    m_pd.nToPage = m_pageTo;

    m_pMarginCur->Load(&m_psd);
    m_pd.Flags &= ~(PD_PRINTTOFILE | PD_SELECTION | PD_PAGENUMS);

    if (m_pMarginCur->m_bSelection)
        m_pd.Flags |= PD_SELECTION;
    else if (m_pageFrom > 1 || m_pageTo < m_pageMax)
        m_pd.Flags |= PD_PAGENUMS;

    if (m_bPrintToFile)
    {
        m_pd.Flags |= PD_PRINTTOFILE;
        ReplaceByFile();
    }
}

void CDlgPrint::PrivatePSD2PD(bool bNeedDC)
{
    PSD2PD();

#ifdef _UNICODE
    m_pd.hDevMode = m_psd.hDevMode;
    m_pd.hDevNames = m_psd.hDevNames;
    m_psd.hDevMode = NULL;
    m_psd.hDevNames = NULL;
#else
    m_pd.hDevMode = DevMode2A(m_psd.hDevMode);
    m_pd.hDevNames = DevNames2A(m_psd.hDevNames);
#endif

    if (bNeedDC) m_pd.hDC = CreatePrinterDC();

    if (m_pMargin != NULL) m_pMargin->Copy(m_pMarginCur);
}

void CDlgPrint::FinalizeDC()
{
    if (m_bDoModalCalled || m_pd.hDC == NULL) return;
    DeleteDC(m_pd.hDC);
    m_pd.hDC = NULL;
    PrivatePD2PSD();
    PrivatePSD2PD(true);
}

void CDlgPrint::LoadPrintToFile(LPCTSTR section)
{
    m_sectionPrintToFile = section;
    bool bPrintToFile = DynamicGetProfileInt(m_sectionPrintToFile, keyPrintToFile, false) != 0;

    if (bPrintToFile)
        m_pd.Flags |= PD_PRINTTOFILE;
    else
        m_pd.Flags |= ~PD_PRINTTOFILE;
}

int CDlgPrint::DoModal()
{  // no call to super::DoModal(), so we count modals in here.
    TModalCounter modalCounter;

    if (m_pMargin == NULL) m_bSaveMargin = false;
    PrivatePD2PSD();

    if (m_psd.lpPageSetupTemplateName == NULL || m_psd.hInstance == NULL)
    {
        m_psd.lpPageSetupTemplateName =
            LPCWSTR(MAKEINTRESOURCE(IsSetupOnly() ? IDD_PAGE_SETUP_EX : IDD_PRINT_EX));
        m_psd.hInstance = AfxGetResourceHandle();
    }

    m_psd.hwndOwner = PreModal();
    bool bOk = ::PageSetupDlgW(&m_psd) == IDOK;
    PostModal();

    if (bOk)
    {
        PrivatePSD2PD((m_pd.Flags & PD_RETURNDC) != 0);
        if (m_bSaveMargin) m_pMargin->SaveReg();
        if (!m_sectionPrintToFile.IsEmpty())
            DynamicWriteProfileInt(m_sectionPrintToFile, keyPrintToFile, m_bPrintToFile);
    }

    m_bDoModalCalled = true;
    return bOk && !IsSetupOnly() ? IDOK : IDCANCEL;
}

BEGIN_MESSAGE_MAP(CDlgPrint, super)
ON_CBN_SELCHANGE(IDC_PRINTER_NAME, OnCbnSelChangePrinterName)
ON_BN_CLICKED(IDC_PRINTER_PROPERTIES, OnBnClickedProperties)
ON_CONTROL_RANGE(BN_CLICKED, IDC_PRINTER_PAGE0, IDC_PRINTER_PAGE0 + selN - 1, OnBnClickedPage)

ON_EN_CHANGE(IDC_PRINTER_PAGE_FROM, OnChangePage)
ON_EN_CHANGE(IDC_PRINTER_PAGE_TO, OnChangePage)
ON_WM_CONTEXTMENU()
ON_WM_SETCURSOR()
ON_WM_TIMER()

ON_CBN_SELCHANGE(IDC_PRINTER_SIZE, OnChange)
ON_EN_CHANGE(IDC_PRINTER_MAR_LEFT, OnChange)
ON_EN_CHANGE(IDC_PRINTER_MAR_TOP, OnChange)
ON_EN_CHANGE(IDC_PRINTER_MAR_RIGHT, OnChange)
ON_EN_CHANGE(IDC_PRINTER_MAR_BOTTOM, OnChange)
ON_BN_CLICKED(IDC_PRINTER_PORTRAIT, OnChange)
ON_BN_CLICKED(IDC_PRINTER_LANDSCAPE, OnChange)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_PRINTER_SLIDER, OnChangeSlider)

ON_BN_CLICKED(IDC_PRINTER_P, OnP)
ON_BN_CLICKED(IDC_PRINTER_APPLY, OnApply)
END_MESSAGE_MAP()

BOOL CDlgPrint::OnInitDialog()
{
    if (m_btnProps.m_hWnd == NULL)
    {
        m_btnProps.SubclassDlgItem(IDC_PRINTER_PROPERTIES, this);
        m_btnProps.SetArrowTitle(CBtnTxt::arRight);
    }

    super::OnInitDialog();

    if (!m_title.IsEmpty())
    {
        CString title;
        GetWindowText(title);
        title += strPrefixer + m_title;
        SetWindowText(title);
    }

    EnableToolTips();

    m_cmbPrinter.SubclassDlgItem(IDC_PRINTER_NAME, this);
    m_pSldPage->SubclassDlgItem(IDC_PRINTER_SLIDER, this);

    GoP(mpaGet);

    if (m_pd.Flags & PD_HIDEPRINTTOFILE)
        ShowDlgItem(IDC_PRINTER_FILE, false);
    else if (m_pd.Flags & PD_DISABLEPRINTTOFILE)
        EnableDlgItem(IDC_PRINTER_FILE, false);

    if (!m_bShowApply) ShowDlgItem(IDC_PRINTER_APPLY, false);

    if (m_psd.Flags & PSD_DISABLEPRINTER)
    {
        static const UINT arrIDC[] = {IDC_PRINTER_NAME, IDC_PRINTER_PROPERTIES, 0};
        EnableDlgItems(arrIDC, false);
    }

    LoadAdditionalData();
    if (IsSetupOnly()) return true;

    SelectSel(selAll);

    TViewEx::TSelButtonState sbs = m_pViewEx != NULL             ? m_pViewEx->ViewExSelectionState()
                                   : m_pd.Flags & PD_NOSELECTION ? TViewEx::sbsHide
                                                                 : TViewEx::sbsEnable;

    ShowDlgItem(IDC_PRINTER_PAGE0 + selSel, sbs != TViewEx::sbsHide);
    EnableDlgItem(IDC_PRINTER_PAGE0 + selSel, sbs != TViewEx::sbsDisable);

    EnableDlgItem(IDC_PRINTER_PORTRAIT, true);
    EnableDlgItem(IDC_PRINTER_LANDSCAPE, true);

    if (m_pMarginCur->m_bSelection && sbs == TViewEx::sbsEnable) SelectSel(selSel);
    LoadPages();
    return true;
}

void CDlgPrint::OnCbnSelChangePrinterName()
{
    GoP(mpaSet);
}

void CDlgPrint::OnBnClickedProperties()
{
    GoP(mpaPro);
}

void CDlgPrint::OnP()
{
    if (m_hHook != NULL || m_action == mpaNon) return;
    m_pDlgHook = this;
    m_hHook = ::SetWindowsHookEx(WH_CALLWNDPROCRET, ProcFilter2, NULL, ::GetCurrentThreadId());
    Default();
    ::UnhookWindowsHookEx(m_hHook);
    m_wndSpec.Detach();
    m_hHook = NULL;
    m_pDlgHook = NULL;
}

void CDlgPrint::OnBnClickedPage(UINT idc)
{
    if (IsSelected(selSel) != m_pMarginCur->m_bSelection)
        RedrawPage(true);
    else
    {
        EnablePages();
        OnChangePage();
    }
}

void CDlgPrint::OnOK()
{
    if (m_pViewEx != NULL) m_pViewEx->ViewExAbortUpdate();

    if (!CheckValid()) return;

    UINT idc = SavePages();
    if (idc != 0)
    {
        LocalAfxMB(IDS_INVALID_PAGE);
        GetDlgItem(idc)->SetFocus();
        return;
    }

    idc = SaveAdditionalData();
    if (idc != 0)
    {
        LocalAfxMB(IDS_INVALID_INPUT);
        GetDlgItem(idc)->SetFocus();
        return;
    }

    Default();
}

void CDlgPrint::OnApply()
{
    if (m_pViewEx != NULL) m_pViewEx->ViewExAbortUpdate();

    if (!CheckValid()) return;
    m_bApply = true;
    PostMessage(WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED));
}

void CDlgPrint::OnCancel()
{
    if (m_pViewEx != NULL) m_pViewEx->ViewExAbortUpdate();

    m_pageFrom = m_pageTo = 0;
    Default();
}

void CDlgPrint::OnChange()
{
    SetChangedFlag();
    Default();
}

void CDlgPrint::OnChangePage()
{
    if (!m_bValid || SavePages() != 0)
    {
        int pageDrawn = m_pSldPage->GetPos();
        m_pSldPage->EnableWindow(false);
        m_pSldPage->SetRange(pageDrawn, pageDrawn, true);
    }
    else
    {
        m_pSldPage->EnableWindow(m_pageTo > m_pageFrom);
        m_pSldPage->SetRange(m_pageFrom, m_pageTo, true);
    }
    RedrawPage(false);
}

void CDlgPrint::OnContextMenu(CWnd* pWnd, CPoint point)
{}

BOOL CDlgPrint::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    CPoint pt;
    GetCursorPos(&pt);
    MSG msg;
    msg.hwnd = m_hWnd;
    msg.message = message;
    msg.wParam = 0;
    msg.lParam = MAKELPARAM(pt.x, pt.y);
    msg.time = GetCurrentMessage()->time;
    msg.pt = pt;
    FilterToolTipMessage(&msg);

    return super::OnSetCursor(pWnd, nHitTest, message);
}

void CDlgPrint::OnTimer(UINT nIDEvent)
{
    switch (nIDEvent)
    {
        case timerLoad:
            LoadPages();
            return;

        case timerUpdate:
            if (m_bValid && m_pageDrawn > 0 && !m_bPageDrawnReady &&
                m_pViewEx->ViewExIsPageReady(m_pageDrawn))
                GetDlgItem(IDC_PRINTER_PREVIEW)->RedrawWindow();

            if (m_pViewEx->ViewExHasUpdateFinished())
            {
                KillTimer(timerUpdate);
                int nPages = m_pViewEx->ViewExGetPageMax();
                m_bValid = nPages > 0;

                if (m_bValid)
                    SetPageMax(nPages);
                else
                    GetDlgItem(IDC_PRINTER_PREVIEW)->RedrawWindow();

                PostMessage(WM_TIMER, timerLoad);
            }
            return;
    }

    super::OnTimer(nIDEvent);
}

void CDlgPrint::OnChangeSlider(NMHDR* pNMHDR, LRESULT* pResult)
{
    RedrawPage(false);
    *pResult = 0;
}

UINT CALLBACK CDlgPrint::PaintHookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (hWnd == NULL) return 0;

    CDlgPrint* pDlg = dynamic_cast<CDlgPrint*>(CWnd::FromHandlePermanent(::GetParent(hWnd)));
    if (pDlg == NULL) return 0;

    switch (message)
    {
        case WM_PSD_PAGESETUPDLG:
            return pDlg->PreDrawPage(LOWORD(wParam), HIWORD(wParam), LPPAGESETUPDLGW(lParam));

        case WM_PSD_FULLPAGERECT:
        case WM_PSD_MINMARGINRECT:
        case WM_PSD_MARGINRECT:
        case WM_PSD_GREEKTEXTRECT:
        case WM_PSD_ENVSTAMPRECT:
        case WM_PSD_YAFULLPAGERECT:
            return pDlg->OnDrawPage(CDC::FromHandle(HDC(wParam)), message, LPRECT(lParam));
    }

    return 0;
}

UINT CDlgPrint::OnDrawPage(CDC* pDC, UINT nMessage, LPRECT lpRect)
{
    if (m_pViewEx != NULL)
    {
        const CRect rect(*lpRect);

        switch (nMessage)
        {
            case WM_PSD_FULLPAGERECT:
                m_rectFull = rect;
                break;

            case WM_PSD_MARGINRECT:
                return 1;

            case WM_PSD_GREEKTEXTRECT:
                DrawPage(pDC->m_hDC);
                if (!m_bValid)
                    m_pViewEx->ViewExDrawCross(pDC, rect);
                else if (!m_bPageDrawnReady)
                    m_pViewEx->ViewExDrawQuestion(pDC, rect);
                return 1;
        }
    }

    return 0;
}

UINT CDlgPrint::PreDrawPage(WORD wPaperType, WORD wFlags, LPPAGESETUPDLGW pPSD)
{
    ASSERT(pPSD == &m_psd);
    return 0;
    // return 1 to prevent any more drawing
}
