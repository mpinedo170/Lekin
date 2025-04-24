#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgPlugin.h"

#include "Misc.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPlugin dialog

CDlgPlugin::CDlgPlugin() : super(CDlgPlugin::IDD)
{
    m_bCopy = false;
    m_pHeu = NULL;
    m_hItem = 0;
}

void CDlgPlugin::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FILES, m_lstFiles);
    DDX_Check(pDX, IDC_COPY, m_bCopy);
    DDX_Text(pDX, IDC_NAME, m_plug.m_Name);
    DDX_Text(pDX, IDC_DESCRIPTION, m_plug.m_Desc);
    DDX_Text(pDX, IDC_ARG, m_plug.m_Arg);

    CString exe = m_plug.GetFullExe();
    DDX_Text(pDX, IDC_EXE, exe);

    if (!pDX->m_bSaveAndValidate)
        AddFiles(m_plug.m_Files, false);
    else
    {
        if (m_plug.m_Name.IsEmpty() || m_plug.m_Desc.IsEmpty())
        {
            AfxMB(IDP_EMPTY_NAME);
            pDX->Fail();
        }
        if (m_pHeu->FindConflict(m_hItem, m_plug.m_Name))
        {
            AfxMB(IDP_IDENTICAL_MENU);
            pDX->Fail();
        }

        SplitPath(exe, m_plug.m_Dir, m_plug.m_Exe);
        if (m_plug.m_Exe.IsEmpty())
        {
            AfxMB(IDP_BAD_EXE);
            pDX->Fail();
        }
        m_plug.ReduceDir();

        m_plug.m_Files = strEmpty;
        for (int i = 0; i < GetFCount(); ++i)
        {
            m_plug.m_Files += m_lstFiles.GetItemText(i, 0);
            m_plug.m_Files += strSemicolon;
        }
    }

    DDX_Radio(pDX, IDC_GUI, m_plug.m_bConsole);
    DDX_Check(pDX, IDC_SHORT, m_plug.m_bShortData);

    DDX_Mask(pDX, IDC_ADV0, 2, m_plug.m_Advanced, 0);
    DDX_Mask(pDX, IDC_OBJ0, objN, m_plug.m_Obj, IDP_NO_OBJ);
    DDX_Mask(pDX, IDC_WKT0, 6, m_plug.m_Wkt, IDP_NO_WKT);
}

BEGIN_MESSAGE_MAP(CDlgPlugin, super)
ON_WM_DROPFILES()
ON_NOTIFY(NM_CLICK, IDC_FILES, OnClickFiles)
ON_BN_CLICKED(IDB_ADDFILE, OnAddFile)
ON_BN_CLICKED(IDB_DELFILE, OnDelFile)
ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
END_MESSAGE_MAP()

void CDlgPlugin::CheckSelection()
{
    CString dir = GetDir();
    bool bOk = !dir.IsEmpty();

    m_btnAdd.EnableWindow(bOk);
    EnableDlgItem(IDOK, bOk);
    m_btnDel.EnableWindow(m_lstFiles.GetSelectedCount() > 0);

    int len = AlgPath.GetLength();
    EnableDlgItem(IDC_COPY, !bOk || _tcsnicmp(LPCTSTR(dir), LPCTSTR(AlgPath), len) != 0);
}

CString CDlgPlugin::GetDir()
{
    CString path;
    GetDlgItemText(IDC_EXE, path);
    CString dir, s;
    SplitPath(path, dir, s);
    return dir;
}

int CDlgPlugin::GetFCount()
{
    return m_lstFiles.GetItemCount();
}

bool CDlgPlugin::Add(LPCTSTR file, bool bSelect)
{
    if (m_lstFiles.GetImageList(LVSIL_SMALL) != &m_images)
    {
        m_images.Create(IDB_FILE, 16, 16, colorGrayL);
        m_lstFiles.SetImageList(&m_images, LVSIL_SMALL);
    }

    for (int i = 0; i < GetFCount(); ++i) m_lstFiles.SetItemState(i, LVIS_SELECTED, 0);

    for (int i = 0; i < GetFCount(); ++i)
    {
        CString s = m_lstFiles.GetItemText(i, 0);
        if (s.CompareNoCase(file) == 0)
        {
            if (bSelect) m_lstFiles.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
            return true;
        }
    }

    CString path = GetDir() + file;
    DWORD attr = GetFileAttributes(path);
    if (attr & FILE_ATTRIBUTE_DIRECTORY) return false;

    int icon = 0;
    SHFILEINFO info;
    if (SHGetFileInfo(
            path, attr, &info, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES))
        icon = m_images.Add(info.hIcon);

    m_lstFiles.InsertItem(LVIF_TEXT | LVIF_STATE | LVIF_IMAGE, GetFCount(), file, bSelect ? LVIS_SELECTED : 0,
        LVIS_SELECTED, icon, 0);
    return true;
}

bool CDlgPlugin::SetExeFile(LPCTSTR path)
{
    SHFILEINFO info;
    if (SHGetFileInfo(path, 0, &info, sizeof(SHFILEINFO), SHGFI_EXETYPE) == 0)
    {
        AfxMB(IDP_BAD_EXE);
        return false;
    }

    CString dir, s;
    SplitPath(path, dir, s);
    if (dir.CompareNoCase(GetDir()) != 0 && GetFCount() > 0)
    {
        CString s1;
        s1.LoadString(IDP_WRONG_DIR);
        CString s2;
        s2.LoadString(IDP_CHG_EXE);
        if (AfxMB2(MB_YESNO, IDP_CHG_EXE, s1 + _T("  ") + s2) != IDYES) return false;
        m_lstFiles.DeleteAllItems();
    }

    SetDlgItemText(IDC_EXE, path);
    CheckDlgButton(IDC_COPY, 0);
    CheckSelection();
    return true;
}

bool CDlgPlugin::AddFiles(LPCTSTR files, bool bSelect)
{
    for (int i = 0; i < GetFCount(); ++i) m_lstFiles.SetItemState(i, ~LVIS_SELECTED, LVIS_SELECTED);
    bool bOk = true;

    while (true)
    {
        CString file = NextToken(files, chrSemicolon);
        if (file.IsEmpty()) break;

        if (file.FindOneOf(strPathSeparators) != -1)
        {
            CString dir, s;
            SplitPath(file, dir, s);
            file = s;
            if (dir.CompareNoCase(GetDir()) != 0)
            {
                AfxMB(IDP_WRONG_DIR);
                bOk = false;
                break;
            }
        }

        if (!Add(file, bSelect))
        {
            AfxMB(IDP_BAD_FILENAME);
            bOk = false;
            break;
        }
    }
    CheckSelection();
    return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgPlugin message handlers

BOOL CDlgPlugin::OnInitDialog()
{
    m_btnAdd.SubclassDlgItem(IDB_ADDFILE, this);
    m_btnDel.SubclassDlgItem(IDB_DELFILE, this);
    FillObjectives();
    FillWorkspaces();
    super::OnInitDialog();

    CRect rect;
    m_lstFiles.GetClientRect(rect);
    m_lstFiles.InsertColumn(0, strEmpty, LVCFMT_LEFT, rect.Width());
    CheckSelection();
    return true;
}

void CDlgPlugin::OnDropFiles(HDROP hDropInfo)
{
    CPoint pt;
    TCHAR path[1024];
    CRect rect;
    int count = 0;

    if (!DragQueryPoint(hDropInfo, &pt))
    {
        LekinBeep();
        return;
    }

    if ((count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0)) <= 0)
    {
        LekinBeep();
        return;
    }

    GetDlgItem(IDC_EXE)->GetWindowRect(rect);
    ScreenToClient(rect);
    if (rect.PtInRect(pt))
    {
        if (count != 1)
        {
            AfxMB(IDP_ONLY_ONE);
            return;
        }
        DragQueryFile(hDropInfo, 0, path, 1024);
        SetExeFile(path);
        return;
    }

    if (GetDir().IsEmpty())
    {
        AfxMB(IDP_BAD_EXE);
        return;
    }

    GetDlgItem(IDC_FILES_BOX)->GetWindowRect(rect);
    ScreenToClient(rect);
    if (!rect.PtInRect(pt))
    {
        LekinBeep();
        return;
    }

    CString files;
    for (int i = 0; i < count; ++i)
    {
        DragQueryFile(hDropInfo, 0, path, 1024);
        files += path;
        files += strSemicolon;
    }
    AddFiles(files, true);
}

void CDlgPlugin::OnClickFiles(NMHDR* pNMHDR, LRESULT* pResult)
{
    CheckSelection();
    *pResult = 0;
}

void CDlgPlugin::OnAddFile()
{
    CFileDialog dlg(true, NULL, GetDir() + _T("*.*"),
        OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Programs|*.exe;*.cmd;*.bat;*.com;*.dll|All Files|*.*||"));
    if (dlg.DoModal() == IDCANCEL) return;

    POSITION pos = dlg.GetStartPosition();
    CString files;
    while (pos)
    {
        files += dlg.GetNextPathName(pos);
        files += strSemicolon;
    }
    AddFiles(files, true);
}

void CDlgPlugin::OnDelFile()
{
    for (int i = GetFCount(); --i >= 0;)
        if (m_lstFiles.GetItemState(i, LVIS_SELECTED)) m_lstFiles.DeleteItem(i);
    CheckSelection();
}

void CDlgPlugin::OnBrowse()
{
    CString exe;
    GetDlgItemText(IDC_EXE, exe);
    CFileDialog dlg(true, NULL, exe, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Programs|*.exe;*.cmd;*.bat;*.com|")
        _T("Windows Executables (*.exe)|*.exe|")
        _T("Dos Binary (*.com)|*.com|")
        _T("Batch Files|*.cmd;*.bat|")
        _T("All Files|*.*||"));
    if (dlg.DoModal() == IDCANCEL) return;
    SetExeFile(dlg.GetPathName());
}

void CDlgPlugin::OnOK()
{
    if (!UpdateData(true)) return;
    if (m_bCopy)
    {
        CString dir = AlgPath + Company;
        CreateDirectory(dir, NULL);
        AppendSlash(dir);

        bool bOk = true;
        for (int i = 0; bOk; ++i)
        {
            CString file = m_plug.GetFile(i);
            if (file.IsEmpty()) break;
            bOk = CopyFile(m_plug.GetFullDir() + file, dir + file, false) != 0;
        }

        if (!bOk)
        {
            AfxMB(IDP_CANT_COPY);
            return;
        }
        else
        {
            m_plug.m_Dir = Company;
            m_plug.m_bReducedDir = true;
        }
    }
    EndDialog(IDOK);
}
