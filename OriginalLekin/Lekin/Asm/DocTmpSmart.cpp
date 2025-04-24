#include "StdAfx.h"

#include "AppAsm.h"
#include "DocTmpSmart.h"

#include "DlgPrintGantt.h"
#include "DocSmart.h"
#include "FrmMain.h"
#include "FrmSmart.h"
#include "MarginGantt.h"

#include "Misc.h"

IMPLEMENT_DYNCREATE(CDocTmpSmart, super)

CDocTmpSmart::CDocTmpSmart() : super(0, NULL, NULL, NULL)
{
    m_pMargin = NULL;
}

CDocTmpSmart::CDocTmpSmart(
    UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass) :
    super(nIDResource, pDocClass, pFrameClass, pViewClass)
{
    HMENU hNew = CreateMenu();

    CMenu m1;
    m1.LoadMenu(IDR_MAINFRAME);
    CMenu m2;
    m2.LoadMenu(GetID() == tvwObj ? IDR_LOG : m_nIDResource);

    CopyMenu(m1.GetSafeHmenu(), hNew, 0);
    CopyMenu(m2.GetSafeHmenu(), hNew, 3);

#ifndef _DEBUG
    DeleteMenu(hNew, ID_TOOL_GENERIC, MF_BYCOMMAND);
    DeleteMenu(hNew, ID_TOOL_CONVERT, MF_BYCOMMAND);
#endif

    m_hMenuShared = hNew;
    m_pRecent = NULL;
    CString s = GetWindowTitle();
    m_pMargin = GetID() == tvwGantt ? new TMarginGantt(s) : new TMargin(s);
    m_pMargin->LoadReg();

    if (GetFileID() == GetID())
    {
        m_pRecent = new CRecentFileList(0, _T("Recent ") + GetUntitled(), _T("File%d"), MRU_COUNT);
        m_pRecent->ReadList();
    }

    if (GetFileID() != -1)
    {
        HMENU hFile = ::GetSubMenu(hNew, 1);
        AppendMenu(hFile, MF_STRING | MF_GRAYED, GetFileID() * MRU_COUNT + ID_FILE_MRU_FILE1, _T("Recent"));
    }
}

CDocTmpSmart::~CDocTmpSmart()
{
    delete m_pMargin;
    m_pMargin = NULL;
    if (!m_pRecent) return;
    m_pRecent->WriteList();
    delete m_pRecent;
    m_pRecent = NULL;
}

int CDocTmpSmart::GetID()
{
    return m_nIDResource - IDR_MCH;
}

int CDocTmpSmart::GetFileID()
{
    switch (m_nIDResource)
    {
        case IDR_GANTT:
        case IDR_OBJ:
            return -1;
        case IDR_LOG:
            return tvwSeq;
        default:
            return GetID();
    }
}

CDlgPrint* CDocTmpSmart::CreateDlgPrint(bool bSetupOnly)
{
    CDlgPrint* pDlg = GetID() == tvwGantt ? new CDlgPrintGantt(bSetupOnly) : new CDlgPrint(bSetupOnly);
    pDlg->m_title = GetWindowTitle();
    pDlg->m_pViewEx = GetView();
    pDlg->m_pMargin = m_pMargin;
    return pDlg;
}

CDocument* CDocTmpSmart::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible)
{
    CDocSmart* pDoc = GetDoc();
    CFrameWnd* pFrame = NULL;

    if (pDoc)
    {
        if (GetID() == tvwMch || GetID() == tvwJob)
        {
            if (!theApp.CheckSequences(true)) return NULL;
        }
        if (!pDoc->SaveModified()) return NULL;
        pDoc->DeleteContents();
        pFrame = GetFrm();
    }
    else
    {
        pDoc = (CDocSmart*)CreateNewDocument();
        pFrame = CreateNewFrame(pDoc, NULL);
    }

    if (lpszPathName != NULL)
    {
        CString dir, file;
        SplitPath(lpszPathName, dir, file);
        if (file.GetLength() == 0 || file[0] == '_')
        {
            AfxMB(IDP_BAD_FILENAME);
            lpszPathName = NULL;
        }
    }

    if (lpszPathName != NULL)
    {
        if (!pDoc->OnOpenDocument(lpszPathName))
        {
            for (int i = m_pRecent->GetSize(); --i >= 0;)
                if ((*m_pRecent)[i] == lpszPathName) m_pRecent->Remove(i);
            lpszPathName = NULL;
        }
        else
        {
            pDoc->SetPathName(lpszPathName);
            m_pRecent->Add(lpszPathName);
        }
    }

    if (lpszPathName == NULL)
    {
        pDoc->SetTitle(GetUntitled());
        pDoc->OnNewDocument();
    }

    InitialUpdateFrame(pFrame, pDoc, bMakeVisible);
    pDoc->SetModifiedFlag2(false);
    return pDoc;
}

bool CDocTmpSmart::DoFile(LPCTSTR lpszPathName, int p1, int p2)
{
    CDocSmart* pDoc = GetDoc();
    TRY
    {
        CFile file(lpszPathName, p1);
        CArchive ar(&file, p2);
        pDoc->Serialize(ar);
        ar.Close();
        file.Close();
    }
    CATCH(CException, e)
    {
        return false;
    }
    END_CATCH
    return true;
}

bool CDocTmpSmart::SaveFile(LPCTSTR lpszPathName)
{
    return DoFile(lpszPathName, CFile::modeCreate | CFile::modeWrite, CArchive::store);
}

bool CDocTmpSmart::AppendFile(LPCTSTR lpszPathName)
{
    return DoFile(lpszPathName, CFile::modeRead, CArchive::load);
}

CFrameWnd* CDocTmpSmart::CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther)
{
    CFrmSmart* pFrame = (CFrmSmart*)super::CreateNewFrame(pDoc, pOther);
    if (!pFrame) return NULL;

    CToolBar& bar = pFrame->m_wndToolBar;
    VERIFY(bar.Create(pFrame, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS));
    VERIFY(bar.LoadToolBar(m_nIDResource));
    theApp.GetMainFrame()->InitBitmaps(bar);

    CToolBar bar2;
    VERIFY(bar2.Create(pFrame, WS_CHILD | CBRS_TOP | CBRS_TOOLTIPS));
    VERIFY(bar2.LoadToolBar(IDT_FILE));
    theApp.GetMainFrame()->InitBitmaps(bar2);
    theApp.GetMainFrame()->InitPrintBitmap(bar2);

    CToolBarCtrl& barCtrl = bar.GetToolBarCtrl();
    CToolBarCtrl& barCtrl2 = bar2.GetToolBarCtrl();

    int count = barCtrl2.GetButtonCount();
    int index = barCtrl.AddBitmap(count, IDT_FILE);
    TBBUTTON button = {0, 0, 0, TBSTYLE_SEP, 0, 0};
    barCtrl.InsertButton(0, &button);

    for (int i = count; --i >= 0;)
    {
        barCtrl2.GetButton(i, &button);
        button.iBitmap += index;
        barCtrl.InsertButton(0, &button);
        if (GetFileID() == -1 && button.idCommand == ID_FILE_PRINT_DIRECT) break;
    }

    bar2.DestroyWindow();
    return pFrame;
}

// Helper functions

CDocSmart* CDocTmpSmart::GetDoc()
{
    POSITION pos = GetFirstDocPosition();
    if (!pos) return NULL;
    CDocument* pDoc = GetNextDoc(pos);
    ASSERT_KINDOF(CDocSmart, pDoc);
    return (CDocSmart*)pDoc;
}

CViewSmart* CDocTmpSmart::GetView()
{
    CDocument* pDoc = GetDoc();
    if (!pDoc) return NULL;
    POSITION pos = pDoc->GetFirstViewPosition();
    if (!pos) return NULL;
    return (CViewSmart*)(pDoc->GetNextView(pos));
}

CFrmSmart* CDocTmpSmart::GetFrm()
{
    CViewSmart* pView = GetView();
    if (!pView) return NULL;
    CFrameWnd* pFrame = pView->GetParentFrame();
    ASSERT_KINDOF(CFrmSmart, pFrame);
    return (CFrmSmart*)pFrame;
}

void CDocTmpSmart::ActivateFrm()
{
    CFrmSmart* pFrame = GetFrm();
    pFrame->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
    if (pFrame->IsIconic()) pFrame->ShowWindow(SW_RESTORE);
}

void CDocTmpSmart::Modify(bool bModified)
{
    CDocSmart* pDoc = GetDoc();
    if (pDoc) pDoc->SetModifiedFlag2(bModified);
}

void CDocTmpSmart::UpdateView()
{
    CDocSmart* pDoc = GetDoc();
    if (pDoc) pDoc->UpdateAllViews(NULL);
}

CString CDocTmpSmart::Prompt()
{
    CString name, ext;
    VERIFY(GetDocString(ext, filterExt));
    VERIFY(GetDocString(name, filterName));
    CString filter = name + _T("|*") + ext + _T("||");

    CFileDialog dlg(true, NULL, GetDoc()->GetTitle(), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, filter);
    if (dlg.DoModal() == IDCANCEL) return strEmpty;
    return dlg.GetPathName();
}

bool CDocTmpSmart::PromptOpen()
{
    CString path = Prompt();
    if (path.IsEmpty()) return false;
    return OpenDocumentFile(path) != NULL;
}

CString CDocTmpSmart::GetWindowTitle()
{
    CString title;
    GetDocString(title, windowTitle);
    return title;
}

CString CDocTmpSmart::GetExt()
{
    CString ext;
    GetDocString(ext, filterExt);
    return ext;
}

CString CDocTmpSmart::GetUntitled()
{
    CString s;
    GetDocString(s, docName);
    return s;
}

CString CDocTmpSmart::GetTitle()
{
    CString name = GetDoc()->GetTitle();
    if (equals(GetExt(), name.Right(4))) name = name.Left(name.GetLength() - 4);
    return name;
}
