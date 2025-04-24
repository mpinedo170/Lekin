#include "StdAfx.h"

#include "AppAsm.h"
#include "Job.h"

#include "Misc.h"
#include "TokenSet.h"
#include "Workcenter.h"
#include "Workspace.h"

////////////////////////////////////////////
// TJob

extern const CString tokOper = _T("Oper");
extern const CString tokRelease = _T("Release");
extern const CString tokDue = _T("Due");
extern const CString tokWeight = _T("Weight");

TJobArray arrJob;

TShop TJob::m_shop = JobShop;

TJob::TJob() : super(tyJob), m_release(0), m_due(0), m_weight(1)
{}

TJob::~TJob()
{
    m_arrOp.DestroyAll();
}

void TJob::Write(CArchive& ar)
{
    super::Write(ar);
    WriteToken(ar, 2, tokRelease, m_release);
    WriteToken(ar, 2, tokDue, m_due);
    WriteToken(ar, 2, tokWeight, m_IOFlag == objTime || IsObjWeighted(m_IOFlag) ? m_weight : 1);
}

void TJob::Read(TTokenSet2& ts)
{
    super::Read(ts);
    if (arrJob.Exists(m_id)) ThrowDupID(m_id, IDS_JOB);

    m_release = ts.ReadRelease();
    m_due = ts.ReadInt(tokDue, 0, 9999, 0);
    m_weight = ts.ReadInt(tokWeight, 0, 9999, 1);
}

namespace {
const CString strRelease = "R";
const CString strDue = "D";
const CString strWeight = "W";
}  // namespace

void TJob::Save(TStringWriter& sw) const
{
    super::Save(sw);

    CString s = strRelease + strEqual + IntToStr(m_release);
    s += strSpace + strDue + strEqual + IntToStr(m_due);
    s += strSpace + strWeight + strEqual + IntToStr(m_weight);
    sw.Write(s);
    sw.SetIndent(2, 2);
    sw.NewLine();

    for (int i = 0; i < GetOpCount(); ++i) m_arrOp[i]->Save(sw);

    sw.SetIndent(0);
    sw.NewLine();
}

void TJob::Load(TStringSpacer& sp)
{
    super::Load(sp);

    enum
    {
        iiiRelease,
        iiiDue,
        iiiWeight
    };

    static const LPCTSTR arrKey[] = {strRelease, strDue, strWeight, NULL};

    try
    {
        while (true)
        {
            int iii = sp.TestSymbolArray(arrKey);
            if (iii < 0) break;

            sp.TestSymbolHard(strEqual);

            switch (iii)
            {
                case iiiRelease:
                    m_release = sp.ReadInt();
                    CheckInt(m_release);
                    break;

                case iiiDue:
                    m_due = sp.ReadInt();
                    CheckInt(m_due);
                    break;

                case iiiWeight:
                    m_weight = sp.ReadInt();
                    CheckInt(m_weight, 1, 9999);
                    break;
            }
        }

        while (sp.TestSymbolNoMove(_T("(")))
        {
            TOperation* pOp = new TOperation(this);
            m_arrOp.Add(pOp);
            pOp->Load(sp);
        }
    }
    catch (CExcMessage* pExc)
    {
        pExc->Prefix(GetToken());
        throw;
    }
}

TOperation* TJob::AddOperation(LPCTSTR idWkc, int proc, TCHAR status)
{
    if (FindOperation(idWkc)) return NULL;
    TOperation* pOp = new TOperation(this, idWkc, proc, status);
    m_arrOp.Add(pOp);
    return pOp;
}

TOperation* TJob::FindOperation(CString idWkc, TOperation* pOpExclude)
{
    int index = m_arrOp.LSearch(idWkc, pOpExclude);
    return index >= 0 ? m_arrOp[index] : NULL;
}

bool TJob::PrivateInvalidate(bool bForce, TOperationArray& arrOpOrphan)
{
    for (int i = GetOpCount(); --i >= 0;)
    {
        TOperation* pOp = m_arrOp[i];
        if (!pOp->GetWkc()) goto BAD;
        for (int j = i + 1; j < GetOpCount(); ++j)
            if (m_arrOp[j]->m_pWkc == pOp->m_pWkc) goto BAD;
        continue;
    BAD:
        if (!bForce) return false;
        arrOpOrphan.Add(pOp);  // add operation to the "orphan" array,
        m_arrOp.RemoveAt(i);   // but do NOT delete it yet
    }
    return true;
}

bool TJob::IsValid()
{
    TOperationArray arrOpOrphan;
    return PrivateInvalidate(false, arrOpOrphan);
}

void TJob::Invalidate(TOperationArray& arrOpOrphan)
{
    PrivateInvalidate(true, arrOpOrphan);
}

int TJob::GetProcTime() const
{
    int s = 0;
    for (int i = 0; i < m_arrOp.GetSize(); ++i) s += m_arrOp[i]->m_proc;
    return s;
}

void TJob::PrivateInvalidateFlowShop()
{
    TOperationArray arrOpTemp;
    arrOpTemp.SetSize(arrWorkcenter.GetSize());

    for (int i = 0; i < GetOpCount(); ++i) m_arrOp[i]->m_pWkc = NULL;

    EnumerateWkc TOperation* pOp = FindOperation(pWkc->m_id);
    if (pOp == NULL) pOp = new TOperation(this, pWkc->m_id, 0, _T('A'));
    pOp->m_pWkc = pWkc;
    arrOpTemp[wi] = pOp;
    EnumerateEnd

        for (int i = 0; i < GetOpCount(); ++i)
    {
        TOperation* pOp = m_arrOp[i];
        if (pOp->m_pWkc == NULL) arrOpTemp.Add(pOp);
    }

    m_arrOp.Copy(arrOpTemp);
}

void TJob::PrivateInvalidateOneShop()
{
    CString idWkc = arrWorkcenter[0]->m_id;
    TCHAR status = _T('A');
    int proc = GetProcTime();

    if (proc <= 0)
        proc = 1;
    else
        status = m_arrOp[0]->m_status;

    if (GetOpCount() == 1)
        m_arrOp[0]->m_idWkc = idWkc;
    else
    {
        TOperation* pOp = new TOperation(this, idWkc, proc, status);
        m_arrOp.DestroyAll();
        m_arrOp.Add(pOp);
    }
}

void TJob::PrivateSetShop()
{
    switch (m_shop)
    {
        case OneShop:
            PrivateInvalidateOneShop();
            break;
        case FlowShop:
            PrivateInvalidateFlowShop();
            break;
    }
}

int TJob::GetStart()
{
    return m_arrOp[0]->GetStart();
}

int TJob::GetStop(int which)
{
    return (&m_arrOp[GetOpCount() - 1]->m_tm)[which].m_finish;
}

int TJob::GetT()
{
    return max(GetStop() - m_due, 0);
}

int TJob::GetWT()
{
    return GetT() * m_weight;
}

bool TJob::IsLate()
{
    return GetStop() > m_due;
}

////////////////////////////////////////////
// Global

CString TJob::GetLabel(TShop shop)
{
    CString s(MAKEINTRESOURCE(IDS_POOL0 + shop));
    return s;
}

void TJob::SetShop(TShop shop)
{
    if (m_shop == shop) return;
    m_shop = shop;
    InvalidateShop();
}

void TJob::InvalidateShop()
{
    if (!GotWkc()) return;
    if (GetShop() == JobShop) return;
    EnumerateJob pJob->PrivateSetShop();
    EnumerateEnd
}

bool TJob::DeleteEmpty()
{
    bool bWas = false;
    for (int i = arrJob.GetSize(); --i >= 0;)
    {
        TJob* pJob = arrJob[i];
        if (pJob->GetProcTime() == 0)
        {
            arrJob.DestroyExact(pJob);
            bWas = true;
        }
    }
    return bWas;
}

bool TJob::DuePresent()
{
    EnumerateJob if (pJob->m_due > 0) return true;
    EnumerateEnd return false;
}

bool TJob::ReleasePresent()
{
    EnumerateJob if (pJob->m_release > 0) return true;
    EnumerateEnd return false;
}
