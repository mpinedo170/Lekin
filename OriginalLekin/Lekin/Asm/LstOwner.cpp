#include "StdAfx.h"
#include "LstOwner.h"
#include "Colors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int timerUpdate = 100;

/////////////////////////////////////////////////////////////////////////////
// CLstOwner

CLstOwner::CLstOwner() :
  m_draw(ODT_LISTBOX)
{ m_bMultiple = false;
  m_bFocusDetected = m_bDragEnabled = m_bCopyEnabled = false;
  m_iFrom = m_iDrop = -1;
  m_yDrop = m_dirScroll = 0;
  m_drg = drgNone;
  m_firstTabIDC = 0;
}

CLstOwner::~CLstOwner()
{ }

BEGIN_MESSAGE_MAP(CLstOwner, CListBox)
  //{{AFX_MSG_MAP(CLstOwner)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_WM_CAPTURECHANGED()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CLstOwner::ResetItemHeight()
{ LPMEASUREITEMSTRUCT pMIS = m_draw.GetMIS();
  MeasureItem(pMIS);
  SetItemHeight(0, pMIS->itemHeight);
}

bool CLstOwner::IsMultiple()
{ return m_bMultiple; }

int CLstOwner::GetSelCount()
{ if (m_bMultiple) return super::GetSelCount();
  return GetCurSel() == -1 ? 0 : 1;
}

int CLstOwner::GetSingleSel()
{ if (!m_bMultiple) return GetCurSel();

  for (int i=0; i<GetCount(); i++)
    if (GetSel(i)) return i;
  return -1;
}

void CLstOwner::SetSingleSel(int index)
{ if (!m_bMultiple)
  { SetCurSel(index); return; }

  SetSel(-1, false);
  if (index != -1) SetSel(index, true);
}

int CLstOwner::AddItemData(LPCTSTR s, int data)
{ int index = AddString(s);
  SetItemData(index, data);
  return index;
}

int CLstOwner::AddData(LPVOID data)
{ int index = AddString(_T(""));
  SetItemDataPtr(index, data);
  return index;
}

LPVOID CLstOwner::GetSingleData()
{ int i = GetSingleSel();
  if (i < 0) return NULL;
  return GetItemDataPtr(i);
}

CString CLstOwner::GetSingleString()
{ CString s;
  int i = GetSingleSel();
  if (i >= 0) GetText(i, s);
  return s;
}


int CLstOwner::ItemFromPoint2(CPoint& pt, int& dirScroll)
{ dirScroll = 0;
  if (GetCount() == 0) return -1;

  int top = GetTopIndex();
  CRect rectItem; GetItemRect(top, rectItem);
  CRect rectCli; GetClientRect(rectCli);

  if (pt.x < rectCli.left || pt.x >= rectCli.right)
    return -1;
  if (pt.y < rectCli.top)
  { dirScroll = -1; return top; }

  int index = -1;

  if (pt.y >= rectCli.bottom)
  { dirScroll = 1;
    index = top + rectCli.Height() / rectItem.Height();
  }
  else index = top + (pt.y - rectItem.top) / rectItem.Height();

  return min(index, GetCount());
}

int CLstOwner::ItemFromPoint2(CPoint& pt)
{ int temp = 0;
  return ItemFromPoint2(pt, temp);
}

void CLstOwner::GetYDrop()
{ CRect rect;
  if (m_iDrop == -1)
    m_yDrop = -100;
  else if (m_iDrop == GetCount())
  { GetItemRect(m_iDrop-1, rect);
    m_yDrop = rect.bottom;
  }
  else
  { GetItemRect(m_iDrop, rect);
    m_yDrop = rect.top;
  }
}

void CLstOwner::SendSelchange()
{ UINT idc = GetDlgCtrlID();
  GetParent()->SendMessage(WM_COMMAND,
    MAKEWPARAM(idc, LBN_SELCHANGE), LPARAM(m_hWnd));
}

bool CLstOwner::StartDrag(CPoint& pt)
{ int index = ItemFromPoint2(pt);
  if (GetSelCount() != 1 || index != GetSingleSel())
    return false;

  SendSelchange();
  m_iFrom = index; SetCapture();
  m_drg = drgDrag; return true;
}

void CLstOwner::DrawDrop()
{ if (m_iDrop == -1) return;
  CDC* pDC = GetDC();
  CPen pen; pen.CreatePen(PS_SOLID, 3, white);
  CPen* pPenOld = pDC->SelectObject(&pen);
  pDC->SetROP2(R2_XORPEN);

  CRect rect; GetClientRect(rect);
  pDC->MoveTo(rect.left, m_yDrop);
  pDC->LineTo(rect.right, m_yDrop);
  pDC->SelectObject(pPenOld);
  ReleaseDC(pDC);
}

void CLstOwner::DrawNewDrop(CPoint& pt)
{ DrawDrop();
  m_iDrop = ItemFromPoint2(pt, m_dirScroll);
  GetYDrop(); DrawDrop();
}

void CLstOwner::Drop()
{ bool bCopy = m_bCopyEnabled && (GetKeyState(VK_CONTROL) & 0x80000000);
  CString s; GetText(m_iFrom, s);
  int iData = GetItemData(m_iFrom);

  if (!bCopy)
  { if (m_iDrop==m_iFrom || m_iDrop==m_iFrom+1)
    { Escape(); return; }

    DeleteString(m_iFrom);
    if (m_iDrop > m_iFrom) m_iDrop--;
  }

  InsertString(m_iDrop, s);
  SetItemData(m_iDrop, iData);
  SetCurSel(m_iDrop);
  Escape(); OnDrop();
}

void CLstOwner::Escape()
{ m_drg = drgNone; m_iDrop = -1;
  ReleaseCapture();
}

/////////////////////////////////////////////////////////////////////////////
// CLstOwner message handlers

void CLstOwner::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{ m_draw.StartDraw(lpDrawItemStruct);
  GetItemText();

  if (m_bFocusDetected && m_draw.m_bSelected && !m_draw.m_bCtrlFocused)
  { CRect rect(m_draw.m_rect);
    rect.DeflateRect(1, 1);
    m_draw.m_colBack = GetSysColor(COLOR_WINDOW);
    m_draw.m_colText = GetSysColor(COLOR_WINDOWTEXT);

    m_draw.m_dc.FillSolidRect(rect, m_draw.m_colBack);
    m_draw.m_dc.SetTextColor(m_draw.m_colText);
  }

  m_draw.StartRegion();
  Draw(&m_draw.m_dc);
  m_draw.Finish();
}

void CLstOwner::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{ m_draw.StartMeasure(m_hWnd, lpMeasureItemStruct);
  GetItemText();
  lpMeasureItemStruct->itemHeight =
    Measure(lpMeasureItemStruct->itemID);
  m_draw.Finish();
}

void CLstOwner::PreSubclassWindow()
{ super::PreSubclassWindow();
  CRect rect; GetClientRect(rect);
  if (m_bDragEnabled) SetTimer(timerUpdate, 250, NULL);
  Init();
}

void CLstOwner::OnLButtonDown(UINT nFlags, CPoint point)
{ super::OnLButtonDown(nFlags, point);
  if (m_bDragEnabled) m_drg = drgTrig;
}

void CLstOwner::OnMouseMove(UINT nFlags, CPoint point)
{ switch (m_drg)
  { case drgTrig:
      if (StartDrag(point)) break;
      m_drg = drgNone;

    case drgNone:
      super::OnMouseMove(nFlags, point);
      return;
  }
  DrawNewDrop(point);
}

void CLstOwner::OnLButtonUp(UINT nFlags, CPoint point)
{ if (!InDrag())
    super::OnLButtonUp(nFlags, point);
  else if (m_iDrop != -1)
  { DrawDrop(); Drop(); }
  else Escape();
  m_drg = drgNone;
}

void CLstOwner::OnTimer(UINT nIDEvent)
{ if (nIDEvent == timerUpdate)
  { if (!InDrag() || m_dirScroll==0) return;
    int ind = m_iDrop + m_dirScroll;
    if (ind<0 || ind>GetCount()) return;

    DrawDrop();
    SendMessage(WM_VSCROLL, m_dirScroll==-1 ? SB_LINEUP : SB_LINEDOWN, NULL);
    m_iDrop = ind; DrawDrop();
  }

  super::OnTimer(nIDEvent);
}

void CLstOwner::GetItemText()
{ m_draw.GetItemText(); }

void CLstOwner::Draw(CDC* pDC)
{ m_draw.Draw(); }

void CLstOwner::Init()
{ DWORD style = GetStyle();
  m_bMultiple = (style & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0;

  if (style & LBS_OWNERDRAWFIXED)
    ResetItemHeight();
  else
    ASSERT(style & LBS_OWNERDRAWVARIABLE);

  if (m_firstTabIDC == 0) return;
  for (int i=m_firstTabIDC; ; i++)
  { CRect rect;
    CWnd* pWndLabel = GetParent()->GetDlgItem(i);
    if (pWndLabel == NULL) break;
    pWndLabel->GetWindowRect(rect);
    m_arrTab.Add(rect.Width());
  }
  m_arrTab[0] -= 2;
}

int CLstOwner::Measure(int index)
{ return m_draw.Measure(); }

void CLstOwner::OnDrop()
{ }

BOOL CLstOwner::PreTranslateMessage(MSG* pMsg)
{ if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && InDrag())
  { DrawDrop(); Escape();
    return true;
  }
  return super::PreTranslateMessage(pMsg);
}

void CLstOwner::OnCaptureChanged(CWnd *pWnd)
{ super::OnCaptureChanged(pWnd);
  if (!InDrag()) return;
  DrawDrop(); Escape();
}

void CLstOwner::OnKillFocus(CWnd* pNewWnd)
{ super::OnKillFocus(pNewWnd);
  if (m_bFocusDetected) RedrawWindow();
}

void CLstOwner::OnSetFocus(CWnd* pOldWnd)
{ super::OnSetFocus(pOldWnd);
  if (m_bFocusDetected) RedrawWindow();
}

void CLstOwner::OnDestroy()
{ KillTimer(timerUpdate);
  CListBox::OnDestroy();
}
