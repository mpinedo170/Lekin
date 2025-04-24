#pragma once

#include "DlgA.h"

class TRoute;

/////////////////////////////////////////////////////////////////////////////
// CDlgOppy dialog

class CDlgOppy : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_OPPY
    };

    CScrollBar m_wndScroll;
    CFont m_font;
    CRect m_colRect[4];
    int m_py;
    int m_dy;
    int m_height;
    int m_width;
    int m_sel;
    int m_good;
    CSmartArray<CWnd> m_controls;

    void AddWkcCtrl();
    CWnd* GetControl(int row, int col);
    void CreateRow();
    void Init(int row);
    int GetExCount();
    int GetPos();
    void ResetScrollInfo();
    void Reposition();

public:
    CDlgOppy(CWnd* pParent, int bottom, TRoute& route);

    bool Conflict(LPCTSTR idWkc, int row);
    bool Conflict();
    bool IsValid();
    int GetCount();
    int GetGoodCount();

    int GetSel();
    void SetSel(int sel);

    CString GetRowAt(int row);
    void SetRowAt(int row, LPCTSTR data);

    void InsertRow(int row, LPCTSTR data);
    void InsertNewRow();
    void DeleteRow(int row);
    void MoveUp();
    void MoveDown();

    void RouteToDlg(TRoute& route);
    void DlgToRoute(TRoute& route);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    afx_msg void OnPaint();
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnDestroy();
    afx_msg void OnCtSetFocus(UINT id);
    afx_msg void OnWkcChange(UINT id);

    DECLARE_MESSAGE_MAP()
};
