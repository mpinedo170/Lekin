#pragma once

class CTreePlug;

struct TPlugin
{
    CString m_SubMenu;
    CString m_Name;
    CString m_Desc;

    CString m_Dir;
    CString m_Exe;
    CString m_Arg;
    CString m_Files;

    // hard parameters
    int m_bReducedDir;  // bool
    int m_bConsole;     // bool
    int m_bShortData;   // bool
    int m_Obj;
    int m_Wkt;
    int m_Advanced;

    // soft parameters
    int m_bGui;  // bool
    int m_limSec;
    int m_bLimAsk;      // bool
    int m_bLimEnforce;  // bool

    // used only for execution
    TObjective m_Obj2;

    TPlugin()
    {
        Init();
    }

    void Init();
    void Default();
    void Load(LPCTSTR data);
    CString Save();
    CString SaveAll();

    void AddToTree(CTreePlug& tree);
    void AddToMenu(CMenu& menu, UINT nID);
    void AddToReg(int id);

    bool ReadFromTree(CTreePlug& tree, HTREEITEM item);
    bool ReadFromReg(int id);
    bool ReadFromReg2(UINT nID);

    bool PrepareProcess();
    bool RunProcess(HANDLE& hProcess, HANDLE& hThread);

    bool IsConsole();
    CString GetFullDir();
    CString GetFullExe();
    CString GetFile(int index);
    void ReduceDir();

    static void DeleteAll();
    static CString Supports(int mask);
};

#define keyAlg _T("Algorithms")
