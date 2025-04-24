#include "StdAfx.h"

#include "AppAsm.h"
#include "Machine.h"

#include "Job.h"
#include "Misc.h"
#include "TokenSet.h"
#include "Workcenter.h"

////////////////////////////////////////////
// TMachine

TMachine::TMachine(TWorkcenter* pWkc) : super(tyMch), TRelStat(*pWkc), m_pWkc(pWkc)
{}

TMachine::TMachine(TWorkcenter* pWkc, LPCTSTR id, int release, TCHAR status) :
    super(tyMch, id, colorWhite),
    TRelStat(release, status),
    m_pWkc(pWkc)
{}

void TMachine::Write(CArchive& ar)
{
    super::Write(ar);
    WriteToken(ar, 4, tokRelease, m_release);
    WriteToken(ar, 4, tokStatus, CString(m_status));
}

void TMachine::Read(TTokenSet2& ts)
{
    super::Read(ts);
    if (m_pWkc->FindMch(m_id)) ThrowDupID(m_id, IDS_MCH);

    m_release = ts.ReadRelease(m_pWkc->m_release);
    m_status = ts.ReadStatus(m_pWkc->m_status);
}

bool TMachine::IsDefault() const
{
    return m_release == m_pWkc->m_release && m_status == m_pWkc->m_status;
}

int TMachine::GetIndex() const
{
    return m_pWkc->m_arrMch.FindExact(this);
}

namespace {
const CString strRelease = "R";
const CString strStatus = "S";
}  // namespace

CString TMachine::Save() const
{
    return TRelStat::Save(m_pWkc->m_release, m_pWkc->m_status);
}

void TMachine::Load(TStringSpacer& sp)
{
    try
    {
        TRelStat::Load(sp);
    }
    catch (CExcMessage* pExc)
    {
        pExc->Prefix(GetToken());
        throw;
    }
}

void TMachine::DestroyThis()
{
    m_pWkc->m_arrMch.DestroyExact(this);
}

TOperation* TMachine::FindOper(LPCTSTR idJob)
{
    TJob* pJob = arrJob.FindKey(idJob);
    if (pJob == NULL) ThrowNotExist(idJob, IDS_JOB);

    LPCTSTR idWkc = m_pWkc->m_id;
    TOperation* pOp = pJob->FindOperation(idWkc);
    if (pOp == NULL) ThrowMessage(IDP_NOT_IN_ROUTE, idJob, idWkc);
    return pOp;
}
