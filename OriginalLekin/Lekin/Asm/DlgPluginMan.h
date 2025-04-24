#pragma once

#include "DlgA.h"
#include "TreePlug.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPluginMan dialog

class CDlgPluginMan : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_PLUGIN_MAN
    };

    CBtnImg m_btnRollback;
    CTreePlug m_treeHeu;
    CRect m_objRect;

    HTREEITEM m_hCurrent, m_hDragged, m_hDrop;
    bool m_bDragCopy;
    void SetDragCursor();

public:
    CDlgPluginMan();

    TPlugin m_plug;

    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangedHeuristics(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNewFolder();
    afx_msg void OnRemove();
    afx_msg void OnNewItem();
    afx_msg void OnRollback();
    afx_msg void OnCopy();
    afx_msg void OnBegindragHeuristics(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnPaint();
    afx_msg void OnAlter();
    afx_msg void OnExport();
    afx_msg void OnImport();
    afx_msg void OnChangeConsole(UINT nIDC);

    void UnchangeConsole();
    void LoadConsole();
    bool SaveConsole();
    void FindPlace(HTREEITEM hTarget, HTREEITEM& hParent, HTREEITEM& hAfter);
    HTREEITEM AddItem(LPCTSTR title, HTREEITEM hTarget);
    HTREEITEM AddNewItem(LPCTSTR title);
    void AlterName(HTREEITEM hItem);
    void Drop();

    DECLARE_MESSAGE_MAP()
};
