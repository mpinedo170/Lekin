#pragma once

#include "DlgA.h"
#include "TreePlug.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPlugin dialog

class CDlgPlugin : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_PLUGIN
    };

    CListCtrl m_lstFiles;
    CBtnImg m_btnAdd;
    CBtnImg m_btnDel;
    CImageList m_images;

    bool Add(LPCTSTR file, bool bSelect);
    bool SetExeFile(LPCTSTR file);
    bool AddFiles(LPCTSTR files, bool bSelect);
    void CheckSelection();

    int GetFCount();
    CString GetDir();

public:
    CDlgPlugin();

    bool m_bCopy;

    TPlugin m_plug;
    CTreePlug* m_pHeu;
    HTREEITEM m_hItem;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnClickFiles(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnAddFile();
    afx_msg void OnDelFile();
    afx_msg void OnBrowse();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()
};
