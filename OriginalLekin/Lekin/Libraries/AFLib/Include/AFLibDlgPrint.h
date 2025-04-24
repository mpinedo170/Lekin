#pragma once

#include "AFLibBtnTxt.h"
#include "AFLibTempTT.h"

class CSliderCtrl;

/////////////////////////////////////////////////////////////////////////////
// CPrintDialog2 dialog
// CPrintDialog with a default constructor (needed for the CTempTT template)

namespace AFLibPrivate {
class CPrintDialog2 : public CPrintDialog
{
private:
    typedef CPrintDialog super;

public:
    explicit CPrintDialog2(bool bSetupOnly = false, CWnd* pParentWnd = NULL) :
        super(bSetupOnly, PD_ALLPAGES, pParentWnd)
    {}
};
}  // namespace AFLibPrivate

/////////////////////////////////////////////////////////////////////////////
// CDlgPrint dialog
// A mixture of CPrintDialog and CPageSetupDialog.
// CPrintDialog interface is for standard MFC handling of print commands.
// However, PageSetupDlg is called in DoModal instead of PrintDlg.

namespace AFLibGui {
class TMargin;
class TViewEx;

class AFLIB CDlgPrint : public CTempTT<AFLibPrivate::CPrintDialog2>
{
private:
    typedef CTempTT<AFLibPrivate::CPrintDialog2> super;
    DEFINE_COPY_AND_ASSIGN(CDlgPrint);

// for MFC Wizard
#if 0
        enum { IDD = IDD_PRINT_EX };
#endif

    // translate PRINTDLG structure into PAGESETUPDLGW
    void PrivatePD2PSD();

    // translate PAGESETUPDLGW into PRINTDLG; create a DC if necessary
    void PrivatePSD2PD(bool bNeedDC);

    // hook procedure
    static LRESULT CALLBACK ProcFilter2(int code, WPARAM wParam, LPARAM lParam);

    // hook for paint messages
    static UINT CALLBACK PaintHookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // need to implicitly open Print Dialog (mini-dialog) and close it right away
    // to check and select printers; hook is used to catch that dialog window opening.
    static CDlgPrint* m_pDlgHook;  // currently modal CDlgPrint object
    HHOOK m_hHook;                 // hook handle
    CWnd m_wndSpec;                // mini-dialog window

    // hook procedure, can use member variables and functions
    void ProcFilter(CWPRETSTRUCT& cwp);

    // functions to load mini-dialog for:
    //   none, get list of printers, set printer, printer properties
    enum CMiniPrintAction
    {
        mpaNon,
        mpaGet,
        mpaSet,
        mpaPro
    };
    CMiniPrintAction m_action;  // action

    // input and output data for Action()
    struct TMiniPrintData
    {
        CString m_printer;           // current printer (in)
        CStringArray m_arrAllPrint;  // array with all printers (out)
        int m_curIndex;              // current printer index
        CString m_status;            // printer status
        CString m_type;              // printer type
        CString m_location;          // where is the printer located
    };
    TMiniPrintData m_miniPrintData;

    // execute action
    void GoP(CMiniPrintAction action);

    // do something with mini-dialog window and close it
    void Action();

    // helper for ReplaceByFile()
    void ReplaceByFileHelper(LPDEVNAMES lpDevNames, int bufSize);

protected:
    CBtnTxt m_btnProps;      // "Printer Properties" button
    bool m_bApply;           // just page setup? set to true if "Apply" is pressed
    bool m_bChanged;         // user changed something in page/print setup -- recompute view
    bool m_bValid;           // view recomputation was OK?
    bool m_bPageDrawnReady;  // current page ready for drawing?

    // m_pd is not updated while dialog is running, so gotta keep current page numbers handy
    int m_pageMax;    // maximum page (# of pages)
    int m_pageFrom;   // first page to print
    int m_pageTo;     // last page to print
    int m_pageDrawn;  // page that was last drawn

    // redraw page if needed
    void RedrawPage(bool bRecompute);

    // replace output port in m_pd by "FILE:"
    void ReplaceByFile();

    // was DoModal called? if not, may need to reload margins and re-create HDC
    bool m_bDoModalCalled;

public:
    explicit CDlgPrint(bool bSetupOnly, CWnd* pParentWnd = NULL);
    ~CDlgPrint();

    // set of margins that gets loaded into the dialog and saved afterwards
    TMargin* m_pMargin;

    // view used for page redrawing
    TViewEx* m_pViewEx;

    // dialog window title
    CString m_title;

    // save margin in the registry in case of success?
    bool m_bSaveMargin;

    // show "Apply" button?
    bool m_bShowApply;

    // where to save "Print to file" in the registry
    CString m_sectionPrintToFile;

    // load m_bPrintToFile from registry
    void LoadPrintToFile(LPCTSTR section);

    // "Setup only" dialog?
    bool IsSetupOnly();

    // recreates HDC in case DoModal was not called
    void FinalizeDC();

    virtual int DoModal();

protected:
    CComboBox m_cmbPrinter;   // "Printer" combo box
    CSliderCtrl* m_pSldPage;  // Slider that changes the displayed page

    // types of selection
    enum TSel
    {
        selAll,
        selSel,
        selFrom,
        selN
    };

    // structure to initialize PageSetupDlgW.  Filled by PD2PSD()
    PAGESETUPDLGW m_psd;

    // a copy of m_pMargin that is changed dynamically
    TMargin* m_pMarginCur;

    // "print to file" selected by the user
    bool m_bPrintToFile;

    // rectangle of the small preview window
    CRect m_rectFull;

    // create printer DC from m_psd
    HDC CreateTempDC();

    // check pages; if invalid, show error message
    bool CheckValid();

    // is the button selected?
    bool IsSelected(TSel sel);

    // select button
    void SelectSel(TSel sel);

    // the user changed something, gotta recompute the preview page
    void SetChangedFlag();

    // change max page
    bool SetPageMax(int pageMax);

    // change page interval; SetPages(0,0) is the same as SetPages(1,max)
    bool SetPages(int pageFrom, int pageTo);

    // return max page
    int GetPageMax();

    // return "from" and "to" pages
    void GetPages(int& pageFrom, int& pageTo);

    // overridable part of PrivatePD2PSD
    virtual void PD2PSD();

    // overridable part of PrivatePSD2PD
    virtual void PSD2PD();

    // enable or disable the "pages...from...to" line
    virtual void EnablePages();

    // save data from "all...from...to" controls into variables
    virtual UINT SavePages();

    // load pages data into controls
    virtual void LoadPages();

    // the following functions may be overloaded if new controls are introduced
    virtual UINT SaveAdditionalData();  // save more data into dialog variables
    virtual void LoadAdditionalData();  // load more data

    // draw a preview page through m_pViewEx
    void DrawPage(HDC hDC);

    // get a tool tip for control
    virtual CString GetToolTip(UINT idc);

    // see CPageSetupDialog::PreDrawPage
    virtual UINT PreDrawPage(WORD wPaperType, WORD wFlags, LPPAGESETUPDLGW pPSD);

    // see CPageSetupDialog::OnDrawPage
    virtual UINT OnDrawPage(CDC* pDC, UINT nMessage, LPRECT lpRect);

    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelChangePrinterName();
    afx_msg void OnBnClickedProperties();
    afx_msg void OnP();
    afx_msg void OnApply();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnChange();
    afx_msg void OnChangePage();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnChangeSlider(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedPage(UINT idc);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CDlgPrint)
};
}  // namespace AFLibGui
