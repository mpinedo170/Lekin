#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgPluginMan.h"

#include "DlgPlugin.h"

#include "Draws.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPluginMan dialog

CDlgPluginMan::CDlgPluginMan() : super(CDlgPluginMan::IDD)
{
    m_plug.Init();
    m_hCurrent = m_hDragged = m_hDrop = NULL;
    m_bDragCopy = false;
}

void CDlgPluginMan::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);

    DDX_Check(pDX, IDC_GUI, m_plug.m_bGui);
    DDX_Text(pDX, IDC_LIM_SEC, m_plug.m_limSec);
    DDV_MinMaxInt(pDX, m_plug.m_limSec, 1, 9999);
    DDX_Check(pDX, IDC_LIM_ASK, m_plug.m_bLimAsk);
    DDX_Check(pDX, IDC_LIM_ENFORCE, m_plug.m_bLimEnforce);

    if (!pDX->m_bSaveAndValidate)
    {
        CString desc = m_plug.m_Desc;
        bool bGui = false;

        if (m_plug.m_bConsole != -1)
        {
            bGui = m_plug.m_bConsole != 0;

            CString buf;
            for (int i = 0; i < 6; ++i)
                if (m_plug.m_Wkt & (1 << i))
                {
                    if (!buf.IsEmpty()) buf += _T(", ");
                    buf += WktGetLabel(TWorkspace(i));
                }
            desc += _T("\r\n\r\nSupports: ");
            desc += buf;
            desc += strDot;

            if (m_plug.m_Advanced)
            {
                desc += _T("\r\n\r\nAlso: ");
                desc += TPlugin::Supports(m_plug.m_Advanced);
                desc += strDot;
            }
        }

        EnableDlgItem(IDC_GUI, bGui);
        SetDlgItemText(IDC_DESCRIPTION, desc);
        ShowDlgItem(IDC_OBJECTIVES, m_plug.m_Obj == ALL_OBJ);
        RedrawWindow(&m_objRect);
    }
}

BEGIN_MESSAGE_MAP(CDlgPluginMan, super)
ON_NOTIFY(TVN_SELCHANGED, IDC_HEURISTICS, OnSelchangedHeuristics)
ON_BN_CLICKED(IDC_NEW_FOLDER, OnNewFolder)
ON_BN_CLICKED(IDC_REMOVE, OnRemove)
ON_BN_CLICKED(IDC_NEW_ITEM, OnNewItem)
ON_BN_CLICKED(IDB_ROLLBACK, OnRollback)
ON_BN_CLICKED(IDC_COPY, OnCopy)
ON_NOTIFY(TVN_BEGINDRAG, IDC_HEURISTICS, OnBegindragHeuristics)
ON_WM_LBUTTONUP()
ON_WM_TIMER()
ON_WM_PAINT()
ON_BN_CLICKED(IDC_ALTER, OnAlter)
ON_BN_CLICKED(IDC_EXPORT, OnExport)
ON_BN_CLICKED(IDC_IMPORT, OnImport)
ON_CONTROL_RANGE(EN_CHANGE, IDC_LIM_SEC, IDC_LIM_SEC, OnChangeConsole)
ON_CONTROL_RANGE(BN_CLICKED, IDC_GUI, IDC_GUI, OnChangeConsole)
ON_CONTROL_RANGE(BN_CLICKED, IDC_LIM_ASK, IDC_LIM_ENFORCE, OnChangeConsole)
END_MESSAGE_MAP()

void CDlgPluginMan::UnchangeConsole()
{
    EnableDlgItem(IDB_ROLLBACK, false);
}

void CDlgPluginMan::LoadConsole()
{
    m_hCurrent = m_treeHeu.GetSelectedItem();
    ASSERT(m_hCurrent);
    bool bData = m_plug.ReadFromTree(m_treeHeu, m_hCurrent);
    UpdateData(false);
    bool bPress = m_hCurrent != m_treeHeu.GetRootItem();

    static const UINT arrIDCBut[] = {IDC_COPY, IDC_REMOVE, 0};

    static const UINT arrIDCConsole[] = {IDC_ALTER, IDC_LIM_SEC, IDC_LIM_ASK, IDC_LIM_ENFORCE, IDC_LAB0,
        IDC_LAB1, IDC_LAB2, IDC_LAB3, IDC_LAB4, 0};

    EnableDlgItems(arrIDCBut, bPress);
    EnableDlgItems(arrIDCConsole, bData);

    UnchangeConsole();
    if (!bData) m_hCurrent = NULL;
}

bool CDlgPluginMan::SaveConsole()
{
    if (!m_hCurrent) return true;
    if (!m_treeHeu.IsPlugin(m_hCurrent))
    {
        m_hCurrent = NULL;
        return true;
    }

    if (!UpdateData(true)) return false;

    m_treeHeu.SetPlugin(m_hCurrent, m_plug);
    UnchangeConsole();
    m_hCurrent = NULL;
    return true;
}

void CDlgPluginMan::AlterName(HTREEITEM hItem)
{
    m_treeHeu.AlterName(hItem);
    m_treeHeu.SelectItem(hItem);
    LoadConsole();
}

void CDlgPluginMan::FindPlace(HTREEITEM hTarget, HTREEITEM& hParent, HTREEITEM& hAfter)
{
    // create a sibling if Target is not a submenu
    if (m_treeHeu.IsPlugin(hTarget))
    {
        hParent = m_treeHeu.GetParentItem(hTarget);
        hAfter = hTarget;
    }
    else  // create a child
    {
        hParent = hTarget;
        hAfter = TVI_FIRST;
    }
}

HTREEITEM CDlgPluginMan::AddItem(LPCTSTR title, HTREEITEM hTarget)
{
    if (!hTarget) hTarget = m_treeHeu.GetSelectedItem();
    HTREEITEM hParent, hAfter;
    FindPlace(hTarget, hParent, hAfter);
    return m_treeHeu.InsertItem(title, hParent, hAfter);
}

HTREEITEM CDlgPluginMan::AddNewItem(LPCTSTR title)
{
    HTREEITEM hNewItem = AddItem(title, m_treeHeu.GetSelectedItem());
    AlterName(hNewItem);
    return hNewItem;
}

void CDlgPluginMan::SetDragCursor()
{
    if (!m_hDragged) return;
    m_bDragCopy = GetKeyState(VK_CONTROL) < 0;
    CPoint cur;
    ::GetCursorPos(&cur);
    m_treeHeu.ScreenToClient(&cur);

    UINT flags;
    m_hDrop = m_treeHeu.HitTest(cur, &flags);
    if (m_hDrop)
    {
        for (HTREEITEM hItem = m_hDrop; hItem; hItem = m_treeHeu.GetParentItem(hItem))
            if (hItem == m_hDragged)
            {
                m_hDrop = NULL;
                break;
            }
    }
    else
    {
        if (flags & TVHT_ABOVE)
            m_treeHeu.SendMessage(WM_VSCROLL, SB_LINEUP, NULL);
        else if (flags & TVHT_BELOW)
            m_treeHeu.SendMessage(WM_VSCROLL, SB_LINEDOWN, NULL);
        else if (flags & TVHT_TORIGHT)
            m_treeHeu.SendMessage(WM_HSCROLL, SB_LINERIGHT, NULL);
        else if (flags & TVHT_TOLEFT)
            m_treeHeu.SendMessage(WM_HSCROLL, SB_LINELEFT, NULL);
    }
    m_treeHeu.SelectDropTarget(m_hDrop);
    ::SetCursor(theApp.LoadCursor(m_hDrop ? (m_bDragCopy ? IDU_DRAG_PLUS : IDU_DRAG) : IDU_NODROP));
}

void CDlgPluginMan::Drop()
{
    if (!m_hDrop) return;
    HTREEITEM hParent, hAfter;
    FindPlace(m_hDrop, hParent, hAfter);
    HTREEITEM hNewItem = m_treeHeu.Copy(m_hDragged, hParent, hAfter);
    if (!m_bDragCopy) m_treeHeu.DeleteItem(m_hDragged);
    AlterName(hNewItem);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgPluginMan message handlers

BOOL CDlgPluginMan::OnInitDialog()
{
    m_btnRollback.SubclassDlgItem(IDB_ROLLBACK, this);
    m_treeHeu.SubclassDlgItem(IDC_HEURISTICS, this);
    m_treeHeu.Init(true);
    GetDlgItem(IDC_OBJECTIVES)->GetWindowRect(&m_objRect);
    ScreenToClient(&m_objRect);

    super::OnInitDialog();
    LoadConsole();
    m_treeHeu.SetFocus();
    return false;
}

void CDlgPluginMan::OnOK()
{
    if (!SaveConsole()) return;
    m_treeHeu.Save();
    EndDialog(IDOK);
}

void CDlgPluginMan::OnSelchangedHeuristics(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (m_hDragged) return;

    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    *pResult = 0;
    if (m_hCurrent == pNMTreeView->itemNew.hItem) return;

    if (m_hCurrent && !SaveConsole())
        m_treeHeu.SelectItem(m_hCurrent);
    else
        LoadConsole();
}

void CDlgPluginMan::OnBegindragHeuristics(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (m_hDragged) return;
    if (!SaveConsole()) return;

    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    *pResult = 0;
    HTREEITEM hItem = pNMTreeView->itemNew.hItem;
    if (hItem == m_treeHeu.GetRootItem()) return;

    if (m_treeHeu.GetSelectedItem() != hItem) m_treeHeu.SelectItem(hItem);

    m_hDragged = hItem;
    m_hDrop = NULL;
    m_bDragCopy = false;
    SetCapture();
    SetDragCursor();
    SetTimer(timerUpdate, 50, NULL);
}

void CDlgPluginMan::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == timerUpdate) SetDragCursor();
    super::OnTimer(nIDEvent);
}

void CDlgPluginMan::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_hDragged)
    {
        KillTimer(timerUpdate);
        ReleaseCapture();
        ::SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
        m_treeHeu.SelectDropTarget(NULL);
        Drop();
        m_hDragged = m_hDrop = NULL;
    }
    super::OnLButtonUp(nFlags, point);
}

void CDlgPluginMan::OnRemove()
{
    m_hCurrent = NULL;
    HTREEITEM hItem = m_treeHeu.GetSelectedItem();
    ASSERT(hItem);
    if (hItem == m_treeHeu.GetRootItem()) return;
    m_treeHeu.DeleteItem(hItem);
}

void CDlgPluginMan::OnNewFolder()
{
    if (!SaveConsole()) return;
    HTREEITEM hItem = AddNewItem(_T("New Submenu"));
    m_treeHeu.SetSubmenu(hItem);
    m_treeHeu.EditLabel(hItem);
}

void CDlgPluginMan::OnNewItem()
{
    HTREEITEM hItem = AddNewItem(_T("New Algorithm"));
    TPlugin plug;
    plug.Default();
    m_treeHeu.SetPlugin(hItem, plug);
    LoadConsole();
    OnAlter();
    UnchangeConsole();
}

void CDlgPluginMan::OnRollback()
{
    if (!m_hCurrent) return;
    m_plug.ReadFromTree(m_treeHeu, m_hCurrent);
    LoadConsole();
}

void CDlgPluginMan::OnCopy()
{
    if (!SaveConsole()) return;
    HTREEITEM hItem = m_treeHeu.GetSelectedItem();
    ASSERT(hItem);
    if (hItem == m_treeHeu.GetRootItem()) return;

    HTREEITEM hNewItem = m_treeHeu.Copy(hItem, m_treeHeu.GetParentItem(hItem), hItem);
    AlterName(hNewItem);
    m_treeHeu.EditLabel(hNewItem);
}

void CDlgPluginMan::OnChangeConsole(UINT nIDC)
{
    EnableDlgItem(IDB_ROLLBACK, true);
}

BOOL CDlgPluginMan::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        CEdit* pEdit = m_treeHeu.GetEditControl();
        if (pEdit != NULL && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
        {
            pEdit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
            return 1;
        }

        if (pMsg->wParam == VK_ESCAPE && GetDlgItem(IDB_ROLLBACK)->IsWindowEnabled())
        {
            OnRollback();
            return 1;
        }
    }
    return super::PreTranslateMessage(pMsg);
}

void CDlgPluginMan::OnPaint()
{
    CPaintDC dc(this);

    dc.SetTextAlign(TA_LEFT | TA_TOP);
    dc.SetTextColor(colorText());
    dc.SetBkMode(TRANSPARENT);
    int sp = 0;
    CRect rect(m_objRect);

    for (int i = 0; i < objN; ++i)
        if (m_plug.m_Obj & (1 << i))
        {
            int sz = DrawFancy(&dc, rect, TObjective(i), false);
            if (sp == 0) sp = sz / 3;
            rect.left += sz + sp;
        }
}

void CDlgPluginMan::OnAlter()
{
    if (!m_hCurrent) return;
    if (!UpdateData(true)) return;

    CDlgPlugin dlg;
    dlg.m_plug = m_plug;
    dlg.m_pHeu = &m_treeHeu;
    dlg.m_hItem = m_hCurrent;

    if (dlg.DoModal() != IDOK)
    {
        if (m_plug.m_Exe.IsEmpty()) OnRemove();
        return;
    }

    m_plug = dlg.m_plug;
    m_treeHeu.SetItemText(m_hCurrent, m_plug.m_Name);
    UpdateData(false);
    OnChangeConsole(0);
}

namespace {
const LPCTSTR strFileFilter = _T("LEKIN Algorithm Packs|*.alg|All Files|*.*||");
}

void CDlgPluginMan::OnExport()
{
    if (!SaveConsole()) return;
    CFileDialog dlg(false, NULL, _T("*.alg"), OFN_HIDEREADONLY, strFileFilter);
    if (dlg.DoModal() != IDOK) return;

    m_treeHeu.Export(dlg.GetPathName(), m_treeHeu.GetSelectedItem());
    LoadConsole();
}

void CDlgPluginMan::OnImport()
{
    if (!SaveConsole()) return;
    CFileDialog dlg(true, NULL, _T("*.alg"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, strFileFilter);
    if (dlg.DoModal() != IDOK) return;

    m_treeHeu.Import(dlg.GetPathName());
    LoadConsole();
}
