#include "StdAfx.h"

#include "AppAsm.h"
#include "TokenSet.h"

#include "Job.h"
#include "Misc.h"
#include "Workcenter.h"
#include "Workspace.h"

namespace {
CString tokOrd = _T("Ordinary");
CString tokOrd2 = _T("Single");
CString tokFlex = _T("Flexible");
CString tokSing = _T("Single");
CString tokJob = _T("Job");
CString tokFlow = _T("Flow");
CString tokShop = _T("Shop");
}  // namespace

int TTokenSet2::ReadInt(LPCTSTR token, int mn, int mx, int def)
{
    int data = StrToInt(GetAt(token, IntToStr(def)));
    CheckInt(data, mn, mx);
    return data;
}

TCHAR TTokenSet2::ReadStatus(TCHAR def)
{
    CString temp = GetAt(tokStatus, CString(def));
    CheckStatus(temp);
    return toupper(temp[0]);
}

int TTokenSet2::ReadRelease(int def)
{
    return ReadInt(tokRelease, 0, 9999, def);
}

////////////////////////////////////////////
// Serialization

void SerializeMch(CArchive& ar)
{
    CString tokWkc = TID::GetToken(tyWkc);
    CString tokMch = TID::GetToken(tyMch);
    CString tokWkc2 = TID::GetTokenWkc2();

    if (ar.IsStoring())
    {
        WriteToken(ar, 0, TWorkcenter::IsFlexible() ? tokFlex : tokOrd, strEmpty);

        EnumerateWkc pWkc->Write(ar);
        NextLn(ar);
        EnumerateEnd return;
    }

    TTokenSet2 ts(ar);
    CString token, data;
    static TWorkcenter* pWkc = NULL;
    static TMachine* pMch = NULL;

    if (pWkc)
    {
        delete pWkc;
        pWkc = NULL;
    }
    if (pMch)
    {
        delete pMch;
        pMch = NULL;
    }

    while (true)
    {
        bool bEof = ReadNext(ar, token, data);
        if (equals(token, tokWkc2))
            token = tokWkc;
        else if (equals(token, tokOrd2))
            token = tokOrd;

        if (equals(token, tokOrd))
        {
            TWorkcenter::SetFlexible(false);
            continue;
        }
        if (equals(token, tokFlex))
        {
            TWorkcenter::SetFlexible(true);
            continue;
        }

        if (bEof || equals(token, tokWkc) || equals(token, tokMch))
        {
            if (ts.Exists(tokWkc))
            {
                if (arrWorkcenter.GetSize() >= MaxWkc) ThrowTooMany(MaxWkc, IDS_WKC);
                pWkc = new TWorkcenter();
                pWkc->Read(ts);
                arrWorkcenter.Add(pWkc);
                pWkc = NULL;
            }
            else if (ts.Exists(tokMch))
            {
                int count = arrWorkcenter.GetSize() - 1;
                if (count < 0) ThrowMessage(IDP_BAD_MCH);

                TWorkcenter* pWkc2 = arrWorkcenter[count];
                pMch = new TMachine(pWkc2);
                pMch->Read(ts);
                if (!TWorkcenter::IsFlexible())
                    delete pMch;
                else
                    pWkc2->m_arrMch.Add(pMch);
                pMch = NULL;
            }

            ts.RemoveAll();
        }

        if (bEof) break;
        ts.SetAt(token, data);
    }

    if (GetTotalMchCount() > MaxMch) ThrowTooMany(MaxMch, IDS_MCH);

    TWorkcenter::SingleMachine();
}

void SerializeJobs(CArchive& ar)
{
    static LPCTSTR shops[] = {tokSing, tokFlow, tokJob};

    if (ar.IsStoring())
    {
        WriteToken(ar, 0, tokShop, shops[TJob::GetShop()]);
        EnumerateJob pJob->Write(ar);
        for (int i = 0; i < pJob->GetOpCount(); ++i) pJob->m_arrOp[i]->Write(ar);
        NextLn(ar);
        EnumerateEnd return;
    }

    TTokenSet2 ts(ar);
    CString token, data;
    static TJob* pJob = NULL;
    static TOperation* pOp = NULL;

    if (pJob)
    {
        delete pJob;
        pJob = NULL;
    }
    if (pOp)
    {
        delete pOp;
        pOp = NULL;
    }

    while (true)
    {
        bool bEof = ReadNext(ar, token, data);
        int count = arrJob.GetSize() - 1;
        TJob* pJob2 = count < 0 ? NULL : arrJob[count];

        if (equals(token, tokShop))
        {
            int i;
            for (i = 0; data != shops[i]; ++i)
                if (i == 2) ThrowMessage(IDP_BAD_SHOP);
            TJob::SetShop(TShop(i));
            continue;
        }

        if (bEof || equals(token, tokJob) || equals(token, tokOper))
        {
            if (ts.Exists(tokJob))
            {
                if (pJob2 && pJob2->GetProcTime() == 0) ThrowMessage(IDP_OPLESS, pJob2->m_id);

                if (arrJob.GetSize() >= MaxJob) ThrowTooMany(MaxJob, IDS_JOB);

                pJob = new TJob;
                pJob->Read(ts);
                arrJob.Add(pJob);
                pJob = NULL;
            }
            else if (ts.Exists(tokOper))
            {
                if (pJob2 == NULL) ThrowBadOp(strEmpty, IDS_JOB);

                if (TJob::GetShop() == OneShop && pJob2->GetOpCount() >= 1)
                    ThrowMessage(IDP_NOT_SINGLE, pJob2->m_id);

                pOp = new TOperation(pJob2);
                pOp->Read(ts);
                pJob2->m_arrOp.Add(pOp);
                pOp = NULL;
            }
            if (bEof) break;
            ts.RemoveAll();
        }
        ts.SetAt(token, data);
    }

    TJob::InvalidateShop();
}
