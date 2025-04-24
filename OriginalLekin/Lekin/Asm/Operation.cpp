#include "StdAfx.h"

#include "AppAsm.h"
#include "Operation.h"

#include "Job.h"
#include "Misc.h"
#include "TokenSet.h"
#include "Workcenter.h"

////////////////////////////////////////////
// TOperation

void TOperation::TTimingData::Init()
{
    m_pSeq = NULL;
    m_tag = m_finish = m_setup = -1;
    m_prevStat = _T('A');
    m_rel = 0;
}

////////////////////////////////////////////
// TOperation

void TOperation::Init()
{
    m_pWkc = NULL;
    m_pOpNext = m_pOpPrev = NULL;
    m_rpt = m_release = -1;
    m_tm.Init();
    m_tm2.Init();
}

TOperation::TOperation(TJob* pJob) : m_pJob(pJob), m_proc(1), m_status(_T('A'))
{
    Init();
}

TOperation::TOperation(TJob* pJob, LPCTSTR idWkc, int proc, TCHAR status) :
    m_pJob(pJob),
    m_idWkc(idWkc),
    m_proc(proc),
    m_status(status)
{
    Init();
}

void TOperation::Write(CArchive& ar)
{
    CString id = m_idWkc;
    if (TID::IsShort())
    {
        if (!GetWkc()) return;
        id = m_pWkc->m_idGen;
    }
    WriteToken(ar, 2, tokOper, id + strSemicolon + IntToStr(m_proc) + strSemicolon + m_status);
}

void TOperation::Read(TTokenSet2& ts)
{
    CString line = ts.GetAt(tokOper);
    LPCTSTR ss = line;
    m_idWkc = NextToken(ss, chrSemicolon);
    m_proc = StrToInt(NextToken(ss, chrSemicolon));
    if (m_idWkc.IsEmpty() || m_proc < 0 || m_pJob->FindOperation(m_idWkc)) ThrowBadOp(m_pJob->m_id, IDS_JOB);
    m_status = ss && isalpha(*ss) ? toupper(*ss) : _T('A');
}

void TOperation::Save(TStringWriter& sw) const
{
    CString s = m_idWkc + strComma + IntToStr(m_proc);
    if (m_status != _T('A')) s += strComma + m_status;
    sw.Write(_T("(") + s + _T(")"));
}

void TOperation::Load(TStringSpacer& sp)
{
    sp.TestSymbolHard(_T("("));
    m_idWkc = sp.FindFirst(strComma);
    m_proc = sp.ReadInt();
    m_status = _T('A');

    if (sp.TestSymbol(strComma))
    {
        m_status = sp.GetCurrent();
        sp.Next();
    }
    sp.TestSymbolHard(_T(")"));
}

TWorkcenter* TOperation::GetWkc()
{
    return m_pWkc = arrWorkcenter.FindKey(m_idWkc);
}

void TOperation::DestroyThis()
{
    m_pJob->m_arrOp.DestroyExact(this);
}

void TOperation::QueueAdd()
{
    m_pWkc->m_queue.Add(this);
    m_pWkc->m_earliest = min(m_pWkc->m_earliest, m_release);
}

void TOperation::QueueDel()
{
    TOperationArray& queue = m_pWkc->m_queue;
    queue.RemoveExact(this);

    if (m_pWkc->m_earliest == m_release)
    {
        int earliest = INT_MAX;
        for (int i = queue.GetSize(); --i >= 0;) earliest = min(earliest, queue[i]->m_release);
        m_pWkc->m_earliest = earliest;
    }
}

TID* TOperation::GetStyle()
{
    static TID style(tyWkc);
    return GetWkc() ? m_pWkc : &style;
}

int TOperation::GetIndex() const
{
    return m_pJob->m_arrOp.FindExact(this);
}

int TOperation::GetStart() const
{
    return m_tm.m_finish - m_proc;
}

int TOperation::GetPreStart() const
{
    return GetStart() - m_tm.m_setup;
}

CString TOperation::GetStatString() const
{
    CString s = CString(m_tm.m_prevStat) + m_status;
    return s;
}
