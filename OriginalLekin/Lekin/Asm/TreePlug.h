#pragma once

#include "Plugin.h"

/////////////////////////////////////////////////////////////////////////////
// CTreePlug: Tree View of Plug-in algorithms

class CTreePlug : public CTreeCtrl
{
private:
    typedef CTreeCtrl super;

    CStringArray m_storage;
    int m_saveID;

    CString m_portDir;
    CStringArray m_portFiles;
    CStringArray m_portPlugs;

    HTREEITEM SearchTree(HTREEITEM hParent, LPCTSTR line, HTREEITEM hSelf);

    void EnumSave(HTREEITEM hRoot);
    bool EnumExport(HTREEITEM hRoot);
    HTREEITEM EnumCopy(HTREEITEM hSource, HTREEITEM hDestParent, HTREEITEM hDestAfter);

public:
    CTreePlug();
    void CreateDummy();
    void Init(bool bLoadFromReg);

    HTREEITEM FindItem(HTREEITEM hParent, LPCTSTR line);
    bool FindConflict(HTREEITEM hItem, LPCTSTR line = NULL);
    void AlterName(HTREEITEM hItem);

    void SetSubmenu(HTREEITEM hItem);
    void SetPlugin(HTREEITEM hItem, TPlugin& plug);
    bool IsPlugin(HTREEITEM hItem);

    void Save();
    HTREEITEM Copy(HTREEITEM hSource, HTREEITEM hDestParent, HTREEITEM hDestAfter);

    bool Export(LPCTSTR fileName, HTREEITEM hSource);
    bool Import(LPCTSTR fileName);

protected:
    afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()
};
