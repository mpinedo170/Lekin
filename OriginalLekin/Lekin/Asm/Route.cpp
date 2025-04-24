#include "StdAfx.h"

#include "AppAsm.h"
#include "Route.h"

#include "Job.h"
#include "Workcenter.h"

////////////////////////////////////////////
// TRoute

TRoute& TRoute::operator=(const TRoute& route)
{
    m_a.Copy(route.m_a);
    return *this;
}

void TRoute::Load(const TJob& job)
{
    Clear();
    for (int i = 0; i < job.GetOpCount(); ++i) Add(job.m_arrOp[i]);
}

void TRoute::Save(TJob& job)
{
    job.m_arrOp.DestroyAll();
    for (int i = 0; i < GetCount(); ++i) job.AddOperation(GetWkcID(i), GetProcTime(i), GetStatus(i));
}

bool TRoute::operator==(const TRoute& route)
{
    if (GetCount() != route.GetCount()) return false;
    for (int i = 0; i < GetCount(); ++i)
        if (m_a[i] != route.m_a[i]) return false;
    return true;
}

bool TRoute::operator!=(const TRoute& route)
{
    return !(*this == route);
}

bool TRoute::IsValid()
{
    if (GetCount() == 0) return false;
    TJob job;
    job.m_id = _T("_");
    Save(job);
    return job.IsValid();
}

void TRoute::Clear()
{
    m_a.RemoveAll();
}

void TRoute::Add(const TOperation* pOp)
{
    Add(pOp->m_idWkc, pOp->m_proc, pOp->m_status);
}

void TRoute::Add(LPCTSTR idWkc, int proc, TCHAR status)
{
    CString s;
    s.Format(_T("%s\t%d\t%c"), idWkc, proc, status);
    m_a.Add(s);
}

CString TRoute::GetWkcID(int index) const
{
    return GetToken(m_a[index], 0, chrTab);
}

int TRoute::GetProcTime(int index) const
{
    return StrToInt(GetToken(m_a[index], 1, chrTab));
}

TCHAR TRoute::GetStatus(int index) const
{
    CString s = GetToken(m_a[index], 2, chrTab);
    return s.GetLength() > 0 ? s[0] : _T('A');
}

int TRoute::GetCount() const
{
    return m_a.GetSize();
}

int TRoute::GetTotal() const
{
    int proc = 0;
    for (int i = 0; i < GetCount(); ++i) proc += GetProcTime(i);
    return proc;
}

void TRoute::DrawRect(CDC* pDC, const CRect& rect) const
{
    int proc = GetTotal();
    int sum = 0;

    if (rect.Width() <= 0) return;
    if (proc <= 0)
    {
        TID style(tyJob);
        style.DrawRect(pDC, rect, strEmpty);
        return;
    }

    CRect rect2(rect);
    double k = double(rect.Width()) / proc;

    for (int i = 0; i < GetCount(); ++i)
    {
        TWorkcenter* pWkc = arrWorkcenter.FindKey(GetWkcID(i));
        TID style(tyWkc);
        if (pWkc != NULL) style.m_color = pWkc->m_color;
        sum += GetProcTime(i);
        rect2.right = rect.left + int(sum * k);
        style.DrawRect(pDC, rect2, strEmpty);
        rect2.left = rect2.right;
    }
}

void TRoute::CreateDefault()
{
    Clear();
    EnumerateWkc Add(pWkc->m_id, 1, _T('A'));
    if (TJob::GetShop() == OneShop) break;
    EnumerateEnd
}
