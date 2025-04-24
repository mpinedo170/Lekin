#include "StdAfx.h"

#include "AppAsm.h"
#include "TreePlug.h"

#include "../ControlSum.h"
#include "DlgRunComp.h"

/////////////////////////////////////////////////////////////////////////////
// CTreePlug

CTreePlug::CTreePlug()
{}

void CTreePlug::CreateDummy()
{
    Create(0, CRect(0, 0, 200, 200), theApp.GetMainWnd(), 1);
}

HTREEITEM CTreePlug::SearchTree(HTREEITEM hParent, LPCTSTR line, HTREEITEM hSelf)
{
    for (HTREEITEM hItem = GetChildItem(hParent); hItem; hItem = GetNextSiblingItem(hItem))
        if (hItem != hSelf && !_tcsicmp(GetItemText(hItem), line)) return hItem;
    return NULL;
}

HTREEITEM CTreePlug::FindItem(HTREEITEM hParent, LPCTSTR line)
{
    return SearchTree(hParent, line, NULL);
}

bool CTreePlug::FindConflict(HTREEITEM hItem, LPCTSTR line)
{
    CString s = line != NULL ? CString(line) : GetItemText(hItem);
    return SearchTree(GetParentItem(hItem), s, hItem) != NULL;
}

void CTreePlug::SetSubmenu(HTREEITEM hItem)
{
    SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
}

void CTreePlug::SetPlugin(HTREEITEM hItem, TPlugin& plug)
{
    CString s = plug.Save();
    int index = m_storage.Add(s);
    SetItemData(hItem, DWORD(LPCTSTR(m_storage[index])));
}

bool CTreePlug::IsPlugin(HTREEITEM hItem)
{
    return GetItemData(hItem) != 0;
}

void CTreePlug::Save()
{
    TPlugin::DeleteAll();
    HTREEITEM root = GetRootItem();
    ASSERT(root);
    m_saveID = 0;
    EnumSave(root);
}

HTREEITEM CTreePlug::Copy(HTREEITEM hSource, HTREEITEM hDestParent, HTREEITEM hDestAfter)
{
    return EnumCopy(hSource, hDestParent, hDestAfter);
}

void CTreePlug::AlterName(HTREEITEM hItem)
{
    if (FindConflict(hItem))
    {
        CString name = GetItemText(hItem);
        while (FindConflict(hItem, name)) ::AlterName(name);
        SetItemText(hItem, name);
    }
}

// Enumeration

void CTreePlug::EnumSave(HTREEITEM hRoot)
{
    if (GetItemData(hRoot) != 0)
    {
        TPlugin plug;
        plug.ReadFromTree(*this, hRoot);
        plug.AddToReg(m_saveID);
        ++m_saveID;
    }

    for (HTREEITEM hItem = GetChildItem(hRoot); hItem; hItem = GetNextSiblingItem(hItem)) EnumSave(hItem);
}

bool CTreePlug::EnumExport(HTREEITEM hRoot)
{
    TPlugin plug;
    if (plug.ReadFromTree(*this, hRoot))
    {
        for (int i = 0;; ++i)
        {
            CString file = plug.GetFile(i);
            if (file.IsEmpty()) break;

            int j = m_portFiles.GetSize();
            while (--j >= 0)
                if (file.CompareNoCase(m_portFiles[j]) == 0) break;
            if (j >= 0) continue;

            m_portFiles.Add(file);
            if (!CopyFile(plug.GetFullDir() + file, m_portDir + file, false)) return false;
        }
        plug.m_Dir.Empty();
        plug.m_bReducedDir = true;
        m_portPlugs.Add(plug.SaveAll());
    }

    for (HTREEITEM hItem = GetChildItem(hRoot); hItem; hItem = GetNextSiblingItem(hItem))
        if (!EnumExport(hItem)) return false;

    return true;
}

HTREEITEM CTreePlug::EnumCopy(HTREEITEM hSource, HTREEITEM hDestParent, HTREEITEM hDestAfter)
{
    HTREEITEM hNewItem = InsertItem(GetItemText(hSource), hDestParent, hDestAfter);

    TPlugin plug;
    if (plug.ReadFromTree(*this, hSource))
        SetPlugin(hNewItem, plug);
    else
        SetSubmenu(hNewItem);

    for (HTREEITEM hItem = GetChildItem(hSource); hItem; hItem = GetNextSiblingItem(hItem))
        EnumCopy(hItem, hNewItem, TVI_LAST);
    return hNewItem;
}

BEGIN_MESSAGE_MAP(CTreePlug, super)
ON_WM_CREATE()
ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreePlug message handlers

void CTreePlug::Init(bool bLoadFromReg)
{
    SetIndent(4);
    HTREEITEM hRoot = InsertItem(_T("User-Defined Heuristics"));
    SetSubmenu(hRoot);

    if (bLoadFromReg)
    {
        for (int i = 0;; ++i)
        {
            TPlugin plug;
            if (!plug.ReadFromReg(i)) break;
            plug.AddToTree(*this);
        }

        Expand(hRoot, TVE_EXPAND);
    }

    SelectItem(hRoot);
}

void CTreePlug::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
    TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
    *pResult = 0;
    HTREEITEM hItem = GetSelectedItem();
    ASSERT(hItem);
    if (hItem == GetRootItem()) *pResult = 1;
}

void CTreePlug::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
    TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
    *pResult = 0;

    HTREEITEM hItem = pTVDispInfo->item.hItem;
    LPCTSTR txt = pTVDispInfo->item.pszText;
    if (!txt || !*txt) return;

    if (FindConflict(hItem, txt))
    {
        AfxMB(IDP_IDENTICAL_MENU);
        return;
    }
    SetItemText(hItem, txt);
}

/////////////////////////////////////////////////////////////////////////////
// import/export

static void WriteString(CFile& file, LPCTSTR str)
{
    int len = _tcslen(str) + 1;
    file.Write(&len, 4);
    file.Write(str, len);
}

static int ReadInteger(CFile& file)
{
    int i = -1;
    if (file.Read(&i, 4) != 4) throw new CFileException();
    return i;
}

static CString ReadString(CFile& file)
{
    CString s;
    int len = ReadInteger(file);

    if (len <= 0 || len >= (1 << 12)) throw new CFileException();

    int l2 = file.Read(s.GetBuffer(len), len);
    bool bOk = l2 == len && LPCTSTR(s)[len - 1] == 0;
    s.ReleaseBuffer();

    if (!bOk) throw new CFileException();
    return s;
}

static CString GetZipName()
{
    return GetTmpPath() + _T("pack.z");
}

bool CTreePlug::Export(LPCTSTR fileName, HTREEITEM hSource)
{
    BeginWaitCursor();

    CString zipName = GetZipName();
    m_portDir = GetTmpPath() + _T("LekinTemp\\");

    if (!KillDirectory(m_portDir) || !KillDirectory(zipName))
    {
        EndWaitCursor();
        return false;
    }

    if (!CreateDirectory(m_portDir, NULL))
    {
        EndWaitCursor();
        AfxMB(IDP_CANT_CREATE);
        return false;
    }

    m_portPlugs.RemoveAll();
    m_portFiles.RemoveAll();

    if (!EnumExport(hSource))
    {
        AfxMB(IDP_CANT_COPY);
        KillDirectory(m_portDir);
        EndWaitCursor();
        return false;
    }

    EndWaitCursor();

    int countFiles = m_portFiles.GetSize();
    CDlgRunComp dlg;
    dlg.m_dir = m_portDir;
    dlg.m_zip = zipName;
    dlg.m_count = countFiles;

    bool bOk = false;
    if (dlg.DoModal() == IDOK)
    {
        LPWORD pBuffer = NULL;
        theApp.BeginWaitCursor();

        try
        {
            CFile Fzip(zipName, CFile::modeRead);
            CFile Falg(fileName, CFile::modeCreate | CFile::modeWrite);

            CString s;
            s.LoadString(IDS_FILE_HEADER);
            WriteString(Falg, s);
            WriteString(Falg, Company);

            for (int i = 0; i < m_portPlugs.GetSize(); ++i) WriteString(Falg, m_portPlugs[i]);
            WriteString(Falg, strEmpty);
            Falg.Write(&countFiles, 4);

            TControlInfo info;
            ControlSum(Fzip, pBuffer, &info, true);

            Falg.Write(&info, sizeof(TControlInfo));
            Falg.Write(pBuffer, info.m_size);

            Fzip.Close();
            Falg.Close();
            bOk = true;
        }
        catch (CException* pExc)
        {
            pExc->Delete();
        }

        theApp.EndWaitCursor();
        delete pBuffer;
        pBuffer = NULL;

        if (!bOk)
        {
            AfxMB(IDP_CANT_COPY);
            DeleteFile(fileName);
        }
    }
    m_portPlugs.RemoveAll();
    m_portFiles.RemoveAll();
    bOk &= KillDirectory(m_portDir);
    DeleteFile(zipName);
    return bOk;
}

bool CTreePlug::Import(LPCTSTR fileName)
{
    m_portPlugs.RemoveAll();
    CString zipName = GetZipName();

    LPWORD pBuffer = NULL;
    theApp.BeginWaitCursor();
    bool bOk = false;
    CString impCompany;
    int countFiles;

    try
    {
        CFile Fzip(zipName, CFile::modeCreate | CFile::modeWrite);
        CFile Falg(fileName, CFile::modeRead);

        CString s;
        s.LoadString(IDS_FILE_HEADER);
        if (ReadString(Falg) != s) throw new CFileException();
        impCompany = ReadString(Falg);

        while (true)
        {
            s = ReadString(Falg);
            if (s.IsEmpty()) break;
            TPlugin plug;
            plug.Load(s);
            plug.m_SubMenu = impCompany + strBSlash + plug.m_SubMenu;
            plug.m_Dir = impCompany;
            plug.m_bReducedDir = true;
            m_portPlugs.Add(plug.SaveAll());
        }

        countFiles = ReadInteger(Falg);
        if (countFiles < 0 || countFiles > (1 << 10)) throw new CFileException();

        TControlInfo info;
        Falg.Read(&info, sizeof(TControlInfo));
        ControlSum(Falg, pBuffer, &info, false);

        Fzip.Write(pBuffer, info.m_size);
        Fzip.Close();
        Falg.Close();

        bOk = true;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    theApp.EndWaitCursor();
    delete pBuffer;
    pBuffer = NULL;

    if (!bOk)
    {
        AfxMB(IDP_BAD_FORMAT);
        DeleteFile(zipName);
        return false;
    }

    CString portDir = AlgPath + impCompany + strBSlash;
    CreateDirectory(portDir, NULL);

    CDlgRunComp dlg;
    dlg.m_bCompress = false;
    dlg.m_dir = portDir;
    dlg.m_zip = zipName;
    dlg.m_count = countFiles;

    bOk = dlg.DoModal() == IDOK;
    if (bOk)
    {
        for (int i = 0; i < m_portPlugs.GetSize(); ++i)
        {
            TPlugin plug;
            plug.Load(m_portPlugs[i]);
            plug.AddToTree(*this);
        }
    }

    m_portPlugs.RemoveAll();
    DeleteFile(zipName);
    return bOk;
}
