#pragma once

#include "AFLibGuiGlobal.h"
#include "AFLibViewEx.h"

namespace AFLibPrivate {
AFLIB extern CRuntimeClass* const pClassPrintPreview;
}

/////////////////////////////////////////////////////////////////////////////
// CViewTempEx
// Template view (can be derived from any view class, e.g., CFormView).
// Handles printing and print preview.

namespace AFLibGui {
template <class V>
class CViewTempEx : public V, public TViewEx
{
private:
    typedef V super;

    // OnEndPrinting() was called while processing
    bool m_bEndPrintingCalled;

protected:
    // saved margins for printing
    TMargin* m_pMargin;

    // print preview class; by default -- CPreviewViewAF
    CRuntimeClass* m_pClassPrintPreview;

private:
    // the PrivateInit() function is needed for AFLIB_TEMP_CONSTRUCTORS to work
    void PrivateInit()
    {
        m_pMargin = NULL;
        m_pClassPrintPreview = AFLibPrivate::pClassPrintPreview;
        m_bEndPrintingCalled = true;
    }

public:
    AFLIB_TEMP_CONSTRUCTORS(CViewTempEx)

    // OnEndPrinting() is not always called from OnFilePrint()!
    // We make sure that OnEndPrintingAF() is always called.
    virtual void OnEndPrintingAF()
    {}

    // OnPreparePrinting has to be overridden for a custom dlg box
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo)
    {
        return ViewExPreparePrinting(pInfo, m_pMargin);
    }

    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
    {
        super::OnBeginPrinting(pDC, pInfo);
        ViewExUpdate(pDC, m_pMargin);
        pInfo->SetMaxPage(ViewExGetPageMax());
    }

    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
    {
        m_bEndPrintingCalled = true;
        ViewExAbortUpdate();
        OnEndPrintingAF();
    }

    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo)
    {
        ViewExOnPrint(pDC, pInfo);
    }

    // In CView, OnFilePrint() and OnFilePrintPreview() are not virtual.
    // Here, they are made virtual for user's convenience.
    // OnCmdMsg() is redefined for this purpose.

    virtual void OnFilePrint();
    virtual void OnFilePrintPreview();
    virtual BOOL OnCmdMsg(UINT nID, int nCode, LPVOID pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
};

template <class V>
void CViewTempEx<V>::OnFilePrint()
{
    m_bEndPrintingCalled = false;
    super::OnFilePrint();
    if (m_bEndPrintingCalled) return;
    OnEndPrintingAF();
    m_bEndPrintingCalled = true;
}

template <class V>
void CViewTempEx<V>::OnFilePrintPreview()
{
    CPrintPreviewState* pState = new CPrintPreviewState;
    if (!DoPrintPreview(AFX_IDD_PREVIEW_TOOLBAR, this, m_pClassPrintPreview, pState))
    {
        AfxMB(AFX_IDP_COMMAND_FAILURE);
        delete pState;
    }
}

template <class V>
BOOL CViewTempEx<V>::OnCmdMsg(UINT nID, int nCode, LPVOID pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    if (nCode == CN_COMMAND)
    {
        switch (nID)
        {
            case ID_FILE_PRINT_DIRECT:
            case ID_FILE_PRINT:
                if (pHandlerInfo == NULL) OnFilePrint();
                return true;

            case ID_FILE_PRINT_PREVIEW:
                if (pHandlerInfo == NULL) OnFilePrintPreview();
                return true;
        }
    }

    return super::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
}  // namespace AFLibGui
