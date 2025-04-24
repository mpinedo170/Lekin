#pragma once

#include "AFLibDefine.h"
#include "AFLibResource.h"
#include "AFLibViewTempEx.h"

/////////////////////////////////////////////////////////////////////////////
// CFormView2 form view
// A form view with a default constructor

namespace AFLibPrivate {
class CFormView2 : public CFormView
{
private:
    typedef CFormView super;

public:
    explicit CFormView2(UINT nIDTemplate = IDD_EMPTY) : super(nIDTemplate)
    {}
};
}  // namespace AFLibPrivate

/////////////////////////////////////////////////////////////////////////////
// CViewAF form view
// Supports "update" and "resize" functionality

namespace AFLibGui {
class AFLIB CViewAF : public CViewTempEx<AFLibPrivate::CFormView2>
{
private:
    typedef CViewTempEx<AFLibPrivate::CFormView2> super;
    DEFINE_COPY_AND_ASSIGN(CViewAF);

    bool m_bInitialized;  // was view initialized?

    // initialize the view
    void PrivateInit();

    // for updating all child windows
    enum TAction
    {
        actHide,
        actShow,
        actRedraw
    };
    TAction m_act;
    static BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);

    // mandatory part of Resize()
    void PrivateResize();

protected:
    explicit CViewAF(UINT nIDTemplate = IDD_EMPTY);

public:
    // calls all virtual updating routines
    void Update();

    // calls all virtual resizing routines
    void Resize();

    // calls both resizing and updating routines
    void ResizeAndUpdate();

    // redraws everything
    void RedrawAll();

    // calls OnUpdateSelection(), redraws
    void UpdateSelection();

    // has view been initialized?
    bool IsInitialized()
    {
        return m_bInitialized;
    }

    // called from CFrmAF::OnGetMinMaxInfo(); return false if cannot determine yet
    virtual bool GetMinMaxInfo(MINMAXINFO& MMI);

    // overridden from CView
    virtual void OnInitialUpdate();

protected:
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);

    // initializes the controls
    virtual void OnInit();

    // updates the view (OnUpdate() is a standard function...)
    virtual void OnUpdateAF();

    // does custom resizing
    virtual void OnResize();

    // called BOTH when the view is updated and when it is resized
    virtual void OnUpdateResize();

    // called when selection needs updating
    virtual void OnUpdateSelection();

    // called when showing child windows after a resize
    virtual bool IsChildVisible(HWND hWnd);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CViewAF)
};
}  // namespace AFLibGui
