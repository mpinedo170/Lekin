#include "StdAfx.h"

#include "AppAsm.h"
#include "Plugin.h"

#include "DlgObj1.h"
#include "Misc.h"
#include "TreePlug.h"
#include "Workcenter.h"
#include "Workspace.h"

static void AppendString(CString& buf, LPCTSTR s)
{
    LPTSTR ss = LPTSTR(s);
    for (; *ss; ++ss)
        if (*ss == chrSemicolon) *ss = chrComma;
    buf += s;
    buf += strSemicolon;
}

void TPlugin::Init()
{
    m_Desc.Empty();
    m_Dir.Empty();
    m_Exe.Empty();
    m_Arg.Empty();
    m_Files.Empty();
    m_bReducedDir = true;
    m_bConsole = -1;
    m_bShortData = false;
    m_Obj = m_Wkt = 0;
    m_Advanced = 0;

    m_bGui = false;
    m_limSec = 0;
    m_bLimAsk = m_bLimEnforce = false;

    m_Obj2 = objTime;
}

void TPlugin::Default()
{
    Init();
    m_bConsole = true;
    m_bShortData = true;
    m_Obj = 0x80;
    m_Wkt = 0x3F;
    m_Advanced = 3;
    m_limSec = 60;
}

void TPlugin::Load(LPCTSTR data)
{
    TStringTokenizer sp(data, chrSemicolon);
    CString s;

    Default();
    m_SubMenu = sp.ReadStr();
    m_Name = sp.ReadStr();
    m_Desc = sp.ReadStr();
    m_Arg = sp.ReadStr();
    m_Dir = sp.ReadStr();
    m_Exe = sp.ReadStr();

    while (true)
    {
        s = sp.ReadStr();
        if (s.IsEmpty()) break;
        m_Files += s;
        m_Files += strSemicolon;
    }

    for (int i = 0; i < 10; ++i) (&m_bReducedDir)[i] = sp.ReadInt();
}

CString TPlugin::Save()
{
    CString s;

    AppendString(s, m_Desc);
    AppendString(s, m_Arg);
    AppendString(s, m_Dir);
    AppendString(s, m_Exe);
    s += m_Files;
    s += strSemicolon;

    for (int i = 0; i < 10; ++i)
    {
        s += IntToStr((&m_bReducedDir)[i]);
        s += strSemicolon;
    }

    return s;
}

CString TPlugin::SaveAll()
{
    CString s;
    AppendString(s, m_SubMenu);
    AppendString(s, m_Name);
    s += Save();
    return s;
}

void TPlugin::AddToTree(CTreePlug& tree)
{
    LPCTSTR ss = m_SubMenu;
    HTREEITEM hItem = tree.GetRootItem();
    ASSERT(hItem);

    CString line;
    while (true)
    {
        line = NextToken(ss, chrBSlash);
        if (line.IsEmpty()) break;
        HTREEITEM hI = tree.FindItem(hItem, line);
        if (hI == NULL) break;

        if (tree.IsPlugin(hI))
        {
            tree.DeleteItem(hI);
            break;
        }
        hItem = hI;
    }

    while (!line.IsEmpty())
    {
        hItem = tree.InsertItem(line, hItem);
        tree.SetSubmenu(hItem);
        line = NextToken(ss, chrBSlash);
    }

    HTREEITEM hI = tree.FindItem(hItem, m_Name);
    if (!hI) hI = tree.InsertItem(m_Name, hItem);
    tree.SetPlugin(hI, *this);
}

void TPlugin::AddToMenu(CMenu& menu, UINT nID)
{
    LPCTSTR ss = m_SubMenu;
    CMenu* pM = &menu;

    CString line;
    while (true)
    {
        line = NextToken(ss, chrBSlash);
        if (line.IsEmpty()) break;

        int i = pM->GetMenuItemCount();
        while (--i >= 0)
        {
            CString sub;
            pM->GetMenuString(i, sub, MF_BYPOSITION);
            if (sub == line) break;
        }
        if (i < 0) break;
        pM = pM->GetSubMenu(i);
    }

    while (!line.IsEmpty())
    {
        HMENU hM;
        hM = ::CreateMenu();
        pM->AppendMenu(MF_POPUP | MF_STRING, UINT(hM), line);
        pM = CMenu::FromHandle(hM);
        line = NextToken(ss, chrBSlash);
    }

    pM->AppendMenu(MF_STRING, nID, m_Name);
}

void TPlugin::AddToReg(int id)
{
    CString key;
    key.Format(_T("%03d"), id);
    theApp.WriteMachineString(keyAlg, key, SaveAll());
}

bool TPlugin::ReadFromTree(CTreePlug& tree, HTREEITEM item)
{
    HTREEITEM root = tree.GetRootItem();
    ASSERT(root);

    Init();
    LPCTSTR main = LPCTSTR(tree.GetItemData(item));
    if (!main) return false;

    CString name = tree.GetItemText(item);
    CString path;

    while (true)
    {
        item = tree.GetParentItem(item);
        if (item == root || !item) break;
        path = tree.GetItemText(item) + strBSlash + path;
    }

    CString s;
    AppendString(s, path);
    AppendString(s, name);
    Load(s + main);
    return true;
}

bool TPlugin::ReadFromReg(int id)
{
    CString key;
    key.Format(_T("%03d"), id);
    CString s = theApp.GetMachineString(keyAlg, key);
    if (s.IsEmpty()) return false;
    Load(s);
    return true;
}

bool TPlugin::ReadFromReg2(UINT nID)
{
    if (!ReadFromReg(nID - ID_PLUGIN0)) return false;
    return (1 << int(WktGet()) & m_Wkt) != 0;
}

bool TPlugin::PrepareProcess()
{
    int bad = 0;

    if (!(m_Advanced & 1))
        if (TWorkcenter::SetupPresent()) bad |= 1;

    if (!(m_Advanced & 2))
        if (TWorkcenter::AvailPresent()) bad |= 2;

    if (bad)
    {
        CString msg;
        AfxFormatString1(msg, IDP_NOT_SUP_ADV, Supports(bad));
        if (!theApp.AskConfirmation(IDP_NOT_SUP_ADV, 1, msg)) return false;
    }

    bool bObjAsk = false;
    m_Obj2 = objTime;

    for (int i = 1; i < objN; ++i)
        if (m_Obj & (1 << i))
        {
            if (m_Obj2 != objTime)
            {
                bObjAsk = true;
                break;
            }

            m_Obj2 = TObjective(i);
        }

    if (m_Obj2 == objTime) return false;

    if (bObjAsk || m_bLimAsk)
    {
        CDlgObj1 dlg(m_Obj);
        dlg.LoadData();
        dlg.m_bAskLimit = m_bLimAsk != 0;
        dlg.m_Limit = m_limSec;

        if (!(m_Obj & (1 << dlg.m_Select))) dlg.m_Select = m_Obj2;

        if (dlg.DoModal() != IDOK) return false;
        m_Obj2 = dlg.m_Select;
        m_limSec = dlg.m_Limit;
    }

    return true;
}

#define keyPath _T("PATH")

bool TPlugin::RunProcess(HANDLE& hProcess, HANDLE& hThread)
{
    hProcess = hThread = NULL;

    CString exe = GetFullExe();
    CString command;
    command.Format(_T("\"%s\" %d %d"), LPCTSTR(exe), m_Obj2, m_limSec);
    if (!m_Arg.IsEmpty())
    {
        command += strSpace;
        command += m_Arg;
    }

    CString path;
    int len = GetEnvironmentVariable(keyPath, NULL, 0);
    if (len > 0)
    {
        GetEnvironmentVariable(keyPath, path.GetBuffer(len), len);
        path.ReleaseBuffer();
    }

    CString path2 = GetFullDir();
    path2.Delete(path2.GetLength() - 1);

    path2 += strSemicolon + ExePath;
    path2.Delete(path2.GetLength() - 1);

    path2 += strSemicolon + path;
    SetEnvironmentVariable(keyPath, path2);

    bool bResult = MakeProcess(command, GetTmpPath(), mpSpecial, hProcess, hThread);
    SetEnvironmentVariable(keyPath, path);
    return bResult;
}

bool TPlugin::IsConsole()
{
    return m_bConsole && !m_bGui;
}

CString TPlugin::GetFullDir()
{
    CString dir = m_Dir;
    if (dir.IsEmpty())
        dir = AlgPath;
    else if (m_bReducedDir)
        dir = AlgPath + dir + strBSlash;
    return dir;
}

CString TPlugin::GetFullExe()
{
    return m_Exe.IsEmpty() ? strEmpty : GetFullDir() + m_Exe;
}

CString TPlugin::GetFile(int index)
{
    if (index <= 0) return m_Exe;
    LPCTSTR ss = m_Files;
    CString file;

    for (; index > 0; --index) file = NextToken(ss, chrSemicolon);

    return file;
}

void TPlugin::ReduceDir()
{
    m_bReducedDir = false;
    int len = AlgPath.GetLength();
    if (!_tcsnicmp(LPCTSTR(AlgPath), LPCTSTR(m_Dir), len))
    {
        m_Dir = m_Dir.Right(m_Dir.GetLength() - len);
        if (!m_Dir.IsEmpty()) m_Dir = m_Dir.Left(m_Dir.GetLength() - 1);
        m_bReducedDir = true;
    }
}

void TPlugin::DeleteAll()
{
    try
    {
        CRegKey key = theApp.GetMachineKey(keyAlg, true);
        for (int i = 0;; ++i)
        {
            CString name;
            name.Format(_T("%03d"), i);
            if (key.DeleteValue(name) != ERROR_SUCCESS) break;
        }
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }
}

CString TPlugin::Supports(int mask)
{
    CString buf;
    for (int i = 0; i < 2; ++i)
        if (mask & (1 << i))
        {
            if (!buf.IsEmpty()) buf += " and ";
            CString s;
            s.LoadString(IDS_ADV0 + i);
            buf += s;
        }
    return buf;
}
