#include "StdAfx.h"

#include "AppAsm.h"
#include "Sequence.h"

#include "Job.h"
#include "Misc.h"
#include "Workcenter.h"

////////////////////////////////////////////
// TSequence

TSequence::TSequence()
{
    m_pMch = NULL;
    m_release = 0;
    m_status = _T('A');
    m_index = 0;
}

TSequence::TSequence(TMachine* pMch) : m_pMch(pMch)
{
    Init();
}

TSequence::TSequence(const TSequence& seq)
{
    *this = seq;
}

TSequence& TSequence::operator=(const TSequence& seq)
{
    m_pMch = seq.m_pMch;
    m_release = seq.m_release;
    m_status = seq.m_status;
    m_arrOp.Copy(seq.m_arrOp);
    return *this;
}

bool TSequence::operator==(const TSequence& seq)
{
    if (m_pMch != seq.m_pMch) return false;
    int len = GetOpCount();
    if (len != seq.GetOpCount()) return false;
    for (int i = 0; i < len; ++i)
        if (m_arrOp[i] != seq.m_arrOp[i]) return false;
    return true;
}

void TSequence::Init()
{
    m_release = m_pMch->m_release;
    m_status = m_pMch->m_status;
    m_index = 0;
}

void TSequence::Write(CArchive& ar)
{
    CString name = m_pMch->m_pWkc->m_id;
    if (TWorkcenter::IsFlexible())
    {
        name += strSemicolon;
        name += m_pMch->m_id;
    }
    WriteToken(ar, 2, TID::GetToken(tyMch), name);

    for (int i = 0; i < GetOpCount(); ++i)
    {
        TOperation* pOp = m_arrOp[i];
        name = pOp->m_pJob->m_id;
        int rel = pOp->m_tm2.m_rel;
        if (rel > 0)
        {
            name += strSemicolon;
            name += IntToStr(rel);
        }
        WriteToken(ar, 4, tokOper, name);
    }
}

namespace {
const CString keyM = _T("M");
}

void TSequence::Save(TStringWriter& sw) const
{
    for (int i = 0; i < GetOpCount(); ++i)
    {
        if (i == 0) sw.Write(_T("("));

        sw.WriteSafeStrNoDelim(m_arrOp[i]->m_pJob->m_id);
        sw.WriteNoDelim(strColon + IntToStr(m_arrOp[i]->GetIndex()));

        sw.WriteNoDelim(i == GetOpCount() - 1 ? _T(")") : strComma);
        sw.Write(strEmpty);
    }
}

void TSequence::Load(TStringSpacer& sp)
{
    sp.TestSymbolHard(_T("("));

    while (!sp.TestSymbol(_T(")")))
    {
        CString idJob = sp.ReadSafeStr();
        TJob* pJob = arrJob.FindKey(idJob);
        TOperation* pOp = NULL;

        if (pJob == NULL) ThrowMessage(IDP_NOT_EXIST, TID::GetToken(tyJob), idJob);

        if (sp.TestSymbol(strColon))
        {
            int index = sp.ReadInt();
            CheckInt(index, 0, pJob->GetOpCount() - 1);
            pOp = pJob->m_arrOp[index];
        }
        else
        {
            CString idWkc = m_pMch->m_pWkc->m_id;
            pOp = pJob->FindOperation(idWkc);
            if (pOp == NULL) ThrowMessage(IDP_NOT_IN_ROUTE, idJob, idWkc);
        }

        m_arrOp.Add(pOp);
    }
}

TMachine* TSequence::ReadMch(LPCTSTR name)
{
    TCHAR sep = TID::IsShort() ? chrDot : chrSemicolon;
    CString idWkc = NextToken(name, sep);
    TWorkcenter* pWkc = TID::IsShort() ? arrWorkcenter.FindGen(idWkc) : arrWorkcenter.FindKey(idWkc);

    if (pWkc == NULL) ThrowNotExist(idWkc, IDS_WKC);

    TMachine* pMch = NULL;
    if (TWorkcenter::IsFlexible())
    {
        CString idMch = name;
        CheckID(idMch, IDS_MCH);

        pMch = TID::IsShort() ? pWkc->m_arrMch.FindGen(idMch) : pWkc->FindMch(idMch);

        if (pMch == NULL) ThrowNotExist(idMch, IDS_MCH);
    }
    else
        pMch = pWkc->m_arrMch[0];
    return pMch;
}

void TSequence::ReadOper(LPCTSTR name)
{
    ASSERT(m_pMch != NULL);

    CString idJob = NextToken(name, chrSemicolon);
    if (TID::IsShort())
    {
        TJob* pJob = arrJob.FindGen(idJob);
        if (pJob) idJob = pJob->m_id;
    }

    TOperation* pOp = m_pMch->FindOper(idJob);
    if (pOp->m_tm2.m_rel != -1) ThrowBadOp(m_pMch->m_id, IDS_MCH);
    VERIFY(pOp->GetWkc() == m_pMch->m_pWkc);

    m_arrOp.Add(pOp);
    int rel = StrToInt(NextToken(name, chrSemicolon));
    pOp->m_tm2.m_rel = max(rel, 0);
}

TOperationArray& TSequence::GetAvail()
{
    return m_pMch->m_pWkc->m_queue;
}

int TSequence::GetAvailCount() const
{
    return GetAvail().GetSize();
}

int TSequence::GetAvailTime() const
{
    return max(m_release, m_pMch->m_pWkc->m_earliest);
}

int TSequence::GetSetup(TOperation* pOp) const
{
    TWorkcenter* pWkc = m_pMch->m_pWkc;
    ASSERT(pWkc == pOp->m_pWkc);
    return pWkc->m_setup.GetAt(m_status - 'A', pOp->m_status - 'A');
}

int TSequence::GetStart(TOperation* pOp) const
{
    return max(pOp->m_tm2.m_rel, max(m_release, pOp->m_release));
}

int TSequence::GetStop(TOperation* pOp) const
{
    return GetStart(pOp) + GetSetup(pOp) + pOp->m_proc;
}

void TSequence::Add(TOperation* pOp)
{
    pOp->m_tm2.m_setup = GetSetup(pOp);
    pOp->m_tm2.m_prevStat = m_status;
    m_release = GetStop(pOp);
    m_status = pOp->m_status;

    if (m_index == GetOpCount())
    {
        m_arrOp.Add(pOp);
    }
    else
    {
        ASSERT(m_arrOp[m_index] == pOp);
    }

    pOp->m_tm2.m_pSeq = this;
    pOp->m_tm2.m_tag = m_index;
    pOp->m_tm2.m_finish = m_release;
    ++m_index;
    TOperation* pOp2 = pOp->m_pOpNext;
    if (pOp2) pOp2->m_release = m_release;
}
