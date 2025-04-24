#pragma once

#include "AFLibDataMass.h"
#include "AFLibDlgSDSupport.h"

/////////////////////////////////////////////////////////////////////////////
// CDlg
// Implements data i/o, check-box/edit-box correspondence, tooltips.
// Provides helpful DlgItem access functions.

namespace AFLibGui {
class AFLIB CDlg : public CDlgSDSupport, public TDataMass
{
private:
    typedef CDlgSDSupport super;
    DEFINE_COPY_AND_ASSIGN(CDlg);

    // some edit fields are enabled only if a certain check box is selected
    // this structure is used for automatic handling of such cases
    struct TTranslationUnit
    {
        UINT m_idcFrom;  // Translate from
        UINT m_idcTo;    // Translate to
        UINT m_idcTo2;   // Additional range of "to" items (e.g., spin buttons)
        int m_count;     // number of elements in the set

        // idc is in the "translate from" range
        bool ContainsIdcFrom(UINT idc) const
        {
            return idc >= m_idcFrom && idc < m_idcFrom + m_count;
        }

        // idc is in the "translate to" range
        bool ContainsIdcTo(UINT idc) const
        {
            return (idc >= m_idcTo && idc < m_idcTo + m_count) ||
                   (m_idcTo2 != 0 && idc >= m_idcTo2 && idc < m_idcTo2 + m_count);
        }
    };

    class TTUArray : public CArray<TTranslationUnit>
    {};

    // array of "active check handlers"
    TTUArray m_arrCheck;

    // enable/disable corresponding edit boxes
    void PrivateProcessCheck(const TTranslationUnit& tu, UINT idc);

    // array of buttons like "Set" or "Browse" (for tool tip translation)
    TTUArray m_arrSetButton;

    // searches for an appropriate tool tip
    CString PrivateGetToolTip(UINT idc, bool bTranslate);

    // enumerator -- adjusts margins in edit boxes
    static BOOL CALLBACK AdjustMarginProc(HWND hWnd, LPARAM lParam);

protected:
    // add "automatic check handler" with 2 dependables
    void AddCheck(UINT idcCheck, UINT idcEdit, UINT idcOther, int count);

    // add "automatic check handler" with 1 dependable
    void AddCheck(UINT idcCheck, UINT idcEdit, int count)
    {
        AddCheck(idcCheck, idcEdit, 0, count);
    }

    // process check manually
    void ProcessCheck(UINT idcCheck, bool bActivate);

    // enable/disable edit boxes for all check sets
    void ProcessAllChecks();

    // add "Set" or "Browse" button
    void AddSetButton(UINT idcButton, UINT idcDest, int count);

    // CDlg::IsDUNeeded() tests "Check Sets"
    virtual bool IsDUNeeded(const TDataUnit* pDu);

    // CDlg::GetToolTip() returns edit box's tip if called for a check box
    virtual CString GetToolTip(UINT idc);

public:
    CDlg();
    explicit CDlg(UINT nIDTemplate, CWnd* pWndParent = NULL);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CDlg)
};
}  // namespace AFLibGui
