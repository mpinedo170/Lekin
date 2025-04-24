#include "StdAfx.h"

#include "AppAsm.h"
#include "Schedule.h"

#include "Job.h"
#include "Misc.h"
#include "Sequence.h"
#include "TokenSet.h"
#include "Workcenter.h"

namespace {
const CString tokSch = _T("Schedule");
const CString tokTime = _T("Time");
}  // namespace

////////////////////////////////////////////
// Operation Evaluation Functions

double TSchedule::m_k1 = 1;
double TSchedule::m_k2 = 1;
int TSchedule::m_newTime = 0;

typedef double (*TEvalFunc)(TOperation* pOp, TSequence* pSeq);

double EDD(TOperation* pOp, TSequence* pSeq)
{
    return pOp->m_pJob->m_due;
}

double MS(TOperation* pOp, TSequence* pSeq)
{
    return pOp->m_pJob->m_due - pOp->m_rpt;
}

double FCFS(TOperation* pOp, TSequence* pSeq)
{
    return pOp->m_release;
}

double LPT(TOperation* pOp, TSequence* pSeq)
{
    return 1. / pOp->m_rpt;
}

double SPT(TOperation* pOp, TSequence* pSeq)
{
    return max(pOp->m_release - pSeq->m_release, 0) + pSeq->GetSetup(pOp) + pOp->m_rpt * 0.99;
}

double WSPT(TOperation* pOp, TSequence* pSeq)
{
    double rpt = SPT(pOp, pSeq);
    int w = pOp->m_pJob->m_weight;
    if (w <= 0) return DBL_MAX;
    return rpt / w;
}

double CR(TOperation* pOp, TSequence* pSeq)
{
    double slk = pOp->m_pJob->m_due - pSeq->m_release;
    double rpt = SPT(pOp, pSeq);
    return slk / rpt;
}

double ATCS(TOperation* pOp, TSequence* pSeq)
{
    TJob* pJob = pOp->m_pJob;
    int t = max(pOp->m_release, pSeq->m_release);
    double rpt = SPT(pOp, pSeq);

    double ss = pJob->m_weight / rpt;
    ss *= exp(-max(pJob->m_due - rpt - t, 0.) / (TSchedule::m_k1 * pOp->m_pWkc->m_avP));
    double avS = pOp->m_pWkc->m_avS;
    if (avS > 0) ss *= exp(-pSeq->GetSetup(pOp) / (TSchedule::m_k2 * avS));
    return -ss;
}

////////////////////////////////////////////
// TSchedule

TSchedule* pSchActive = NULL;
TScheduleArray arrSchedule;

CString TSchedule::GetLabel(int IDObj)
{
    CString s(MAKEINTRESOURCE(IDS_OBJ0 + IDObj));
    return s;
}

TSchedule::TSchedule() : super(tySch)
{
    Clear();
    ClearObj();
    m_arrObj[objTime] = 1;
    m_id = "Current";
    RandomColor();
    m_bShown = true;
}

TSchedule::~TSchedule()
{
    Clear();
}

TSchedule& TSchedule::operator=(const TSchedule& sch)
{
    Clear();
    m_id = sch.m_id;
    m_color = sch.m_color;
    m_bShown = sch.m_bShown;

    m_arrSequence.SetSize(sch.m_arrSequence.GetSize());
    for (int i = 0; i < m_arrSequence.GetSize(); ++i) m_arrSequence[i] = new TSequence(*sch.m_arrSequence[i]);

    m_arrDelay.SetSize(sch.m_arrDelay.GetSize());
    for (int i = 0; i < m_arrDelay.GetSize(); ++i) m_arrDelay[i] = TDelay(sch.m_arrDelay[i]);

    for (int i = 0; i < objN; ++i) m_arrObj[i] = sch.m_arrObj[i];

    return *this;
}

bool TSchedule::operator==(const TSchedule& sch)
{
    int len = GetSeqCount();
    if (len != sch.GetSeqCount()) return false;

    for (int i = 0; i < len; ++i)
        if (*m_arrSequence[i] != *sch.m_arrSequence[i]) return false;

    len = m_arrDelay.GetSize();
    if (len != sch.m_arrDelay.GetSize()) return false;

    for (int i = 0; i < len; ++i)
    {
        if (m_arrDelay[i].m_pOp != sch.m_arrDelay[i].m_pOp) return false;
        if (m_arrDelay[i].m_rel != sch.m_arrDelay[i].m_rel) return false;
    }

    return true;
}

void TSchedule::Write(CArchive& ar)
{
    TID::Write(ar);
    WriteToken(ar, 2, tokTime, m_arrObj[objTime]);

    ClearTiming(Normal);
    for (int i = 0; i < m_arrDelay.GetSize(); ++i) m_arrDelay[i].m_pOp->m_tm2.m_rel = m_arrDelay[i].m_rel;

    for (int i = 0; i < m_arrSequence.GetSize(); ++i) m_arrSequence[i]->Write(ar);
    NextLn(ar);
}

void TSchedule::Read(TTokenSet2& ts)
{
    TID::Read(ts);
    CheckID(m_id, IDS_SCH);
    m_arrObj[objTime] = max(ts.ReadInt(tokTime, 0, 1 << 30, m_newTime), 1);

    arrSchedule.AlterName(m_id);
    ClearTiming(Extra);
    CreateSequences();
}

bool TSchedule::FinishReading()
{
    EnumerateJob for (int i = 0; i < pJob->GetOpCount(); ++i)
    {
        TOperation* pOp = pJob->m_arrOp[i];
        int rel = pOp->m_tm2.m_rel;
        switch (rel)
        {
            case -1:
            {
                m_pOpError = pOp;
                m_error = Missed;
                FormatError();
                return false;
            }
            case 0:
                break;
            default:
                AddDelay(pOp, rel);
        }
    }
    EnumerateEnd

        if (!Recompute(false))
    {
        FormatError();
        return false;
    }
    return true;
}

void TSchedule::Save(TStringWriter& sw) const
{
    super::Save(sw);
    sw.Write(_T("{"));
    sw.SetIndent(2);

    for (int i = 0; i < GetSeqCount(); ++i)
    {
        const TSequence* pSeq = m_arrSequence[i];
        const TMachine* pMch = pSeq->m_pMch;

        sw.WriteSafeStr(pMch->m_pWkc->m_id);
        if (pMch->m_pWkc->GetMchCount() > 1) sw.WriteNoDelim(strColon + IntToStr(pMch->GetIndex()));
        pSeq->Save(sw);
        sw.NewLine();
    }

    sw.NewLine();
    sw.SetIndent(0);
    sw.Write(_T("}"));
    sw.NewLine();
}

void TSchedule::Load(TStringSpacer& sp)
{
    super::Load(sp);
    sp.TestSymbolHard(_T("{"));

    while (!sp.TestSymbol(_T("}")))
    {
        CString idWkc = sp.ReadSafeStr();
        int index = 0;
        TWorkcenter* pWkc = arrWorkcenter.FindKey(idWkc);

        if (sp.TestSymbol(strColon))
        {
            index = sp.ReadInt();
            CheckInt(index, 0, pWkc->GetMchCount() - 1);
        }

        TMachine* pMch = pWkc->m_arrMch[index];
        TSequence* pSeq = new TSequence(pMch);
        m_arrSequence.Add(pSeq);
        pSeq->Load(sp);
    }
}

void TSchedule::Clear()
{
    m_arrSequence.DestroyAll();
}

void TSchedule::ClearObj()
{
    for (int i = 1; i < objN; ++i) m_arrObj[i] = 0;
    m_error = Ok;
    m_pOpError = NULL;
}

TSequence* TSchedule::FindMch(const TMachine* pMch)
{
    for (int i = 0; i < GetSeqCount(); ++i)
    {
        TSequence* pSeq = m_arrSequence[i];
        if (pSeq->m_pMch == pMch) return pSeq;
    }
    return NULL;
}

void TSchedule::CreateSequences()
{
    int j = 0;
    EnumerateWkc for (int i = 0; i < pWkc->GetMchCount(); ++i)
    {
        TMachine* pMch = pWkc->m_arrMch[i];
        if (GetSeqCount() <= j || m_arrSequence[j]->m_pMch != pMch)
        {
            TSequence* pSeq = new TSequence(pMch);
            m_arrSequence.InsertAt(j, pSeq);
        }
        ++j;
    }
    EnumerateEnd
}

void TSchedule::PrepareTiming()
{
    ClearTiming(Normal);
    EnumerateJob pJob->m_arrOp[0]->m_release = pJob->m_release;
    for (int i = pJob->GetOpCount(); --i >= 1;)
    {
        TOperation* pOpPrev = pJob->m_arrOp[i - 1];
        TOperation* pOpNext = pJob->m_arrOp[i];
        pOpPrev->m_pOpNext = pOpNext;
        pOpNext->m_pOpPrev = pOpPrev;
    }
    EnumerateEnd

        EnumerateWkc pWkc->m_queue.RemoveAll();
    pWkc->m_earliest = INT_MAX;
    EnumerateEnd

    CreateSequences();
}

void TSchedule::Create(TRule rule)
{
    static TEvalFunc funcs[] = {ATCS, EDD, MS, FCFS, LPT, SPT, WSPT, CR};

    TEvalFunc eval = funcs[rule];

    Clear();
    PrepareTiming();

    EnumerateJob TOperation* pOp = pJob->m_arrOp[0];
    pOp->QueueAdd();
    EnumerateEnd

        while (true)
    {  // find a machine to schedule
        TSequence* pSeq = NULL;
        for (int i = 0; i < m_arrSequence.GetSize(); ++i)
        {
            TSequence* pSeq2 = m_arrSequence[i];
            int k = pSeq == NULL ? -1 : sgn(pSeq2->GetAvailTime() - pSeq->GetAvailTime());

            switch (k)
            {
                case 1:
                    continue;
                case 0:
                    if (pSeq2->GetAvailCount() <= pSeq->GetAvailCount()) continue;
                case -1:
                    if (pSeq2->GetAvailCount() > 0) pSeq = pSeq2;
            }
        }

        if (pSeq == NULL) break;
        TOperationArray& queue = pSeq->GetAvail();

        // find an operation which ends first
        int rel = INT_MAX;
        for (int i = queue.GetSize(); --i >= 0;)
        {
            int rel2 = pSeq->GetStop(queue[i]);
            if (rel2 < rel) rel = rel2;
        }

        // find an operation to schedule on pSeq->m_pMch
        TOperation* pOp = NULL;
        double lev = INT_MAX;

        for (int i = 0; i < queue.GetSize(); ++i)
        {
            TOperation* pOp2 = queue[i];

            // pure dominance
            if (pOp2->m_release >= rel) continue;

            // use rule
            double lev2 = eval(pOp2, pSeq);
            if (lev2 >= lev) continue;

            pOp = pOp2;
            lev = lev2;
        }

        if (pOp == NULL) pOp = queue[0];

        // schedule 'pOp' on 'pSeq'
        pSeq->Add(pOp);
        pOp->QueueDel();
        TOperation* pOp2 = pOp->m_pOpNext;
        if (pOp2 != NULL) pOp2->QueueAdd();
    }
}

bool TSchedule::Recompute(bool bUpdateTiming)
{
    PrepareTiming();

    for (int i = 0; i < m_arrSequence.GetSize(); ++i)
    {
        TSequence* pSeq = m_arrSequence[i];
        pSeq->Init();
        for (int j = 0; j < pSeq->GetOpCount(); ++j)
        {
            TOperation* pOp = pSeq->m_arrOp[j];
            if (pOp->m_tm2.m_pSeq)
            {
                m_pOpError = pOp;
                m_error = Double;
                return false;
            }

            pOp->m_tm2.m_pSeq = pSeq;
            pOp->m_tm2.m_tag = j;
        }
    }

    for (int i = 0; i < m_arrDelay.GetSize(); ++i) m_arrDelay[i].m_pOp->m_tm2.m_rel = m_arrDelay[i].m_rel;

    while (true)
    {
        int flag = 0;
        EnumerateJob for (int i = 0; i < pJob->GetOpCount(); ++i)
        {
            TOperation* pOp = pJob->m_arrOp[i];
            TSequence* pSeq = pOp->m_tm2.m_pSeq;
            if (!pSeq)
            {
                m_pOpError = pOp;
                m_error = Missed;
                return false;
            }

            if (pOp->m_tm2.m_finish >= 0) continue;
            if (pOp->m_tm2.m_tag > pSeq->m_index) continue;

            if (pOp->m_release < 0)
            {
                if (!flag)
                {
                    m_pOpError = pOp;
                    flag = -1;
                }  // there are unscheduled opers
            }
            else
            {
                pSeq->Add(pOp);
                flag = 1;
            }  // at least one oper scheduled
        }
        EnumerateEnd if (flag == -1)
        {
            m_error = Cycle;
            return false;
        }

        if (flag == 0) break;
    }

    ClearObj();

    EnumerateJob int Cj = pJob->GetStop(1);
    int Tj = max(Cj - pJob->m_due, 0);
    int w = pJob->m_weight;
    m_arrObj[objCmax] = max(m_arrObj[objCmax], Cj);
    m_arrObj[objLmax] = max(m_arrObj[objLmax], Tj);
    m_arrObj[objUj] += Tj > 0;
    m_arrObj[objCj] += Cj;
    m_arrObj[objTj] += Tj;
    m_arrObj[objWCj] += w * Cj;
    m_arrObj[objWTj] += w * Tj;
    EnumerateEnd

        if (bUpdateTiming) EnumerateJob for (int i = pJob->GetOpCount(); --i >= 0;)
    {
        TOperation* pOp = pJob->m_arrOp[i];
        pOp->m_tm = pOp->m_tm2;
    }
    EnumerateEnd return true;
}

void TSchedule::FormatError()
{
    ASSERT(m_pOpError);
    ASSERT(m_error != Ok);
    CString s = m_pOpError->m_idWkc + strComma + m_pOpError->m_pJob->m_id;
    int err = IDP_SCH_ERR1 - 1 + int(m_error);
    AfxMB(err, s, m_id);
}

void TSchedule::AddDelay(TOperation* pOp, int delay)
{
    m_arrDelay.Add(TDelay(pOp, delay));
}

void TSchedule::DelDelay(const TOperation* pOp)
{
    for (int i = 0; i < m_arrDelay.GetSize(); ++i)
        if (m_arrDelay[i].m_pOp == pOp)
        {
            m_arrDelay.RemoveAt(i);
            return;
        }
}

////////////////////////////////////////////
// Global

void TSchedule::ClearTiming(TClearTiming how)
{
    EnumerateJob int due = pJob->m_due;
    int rpt = 0;
    for (int i = pJob->GetOpCount(); --i >= 0;)
    {
        TOperation* pOp = pJob->m_arrOp[i];
        rpt += pOp->m_proc;
        pOp->m_rpt = rpt;
        pOp->m_pOpNext = pOp->m_pOpPrev = NULL;
        pOp->m_release = -1;
        pOp->m_tm2.Init();
        switch (how)
        {
            case Completely:
                pOp->m_tm.Init();
                break;
            case Extra:
                pOp->m_tm2.m_rel = -1;
        }
    }
    EnumerateEnd
}

void SerializeSch(CArchive& ar)
{
    if (ar.IsStoring())
    {
        EnumerateSch pSch->Write(ar);
        EnumerateEnd return;
    }

    TTokenSet2 ts(ar);
    static TSchedule* pSch = NULL;
    if (pSch)
    {
        delete pSch;
        pSch = NULL;
    }
    TSequence* pSeq = NULL;

    while (true)
    {
        CString token, data;
        bool bEof = ReadNext(ar, token, data);

        if (equals(token, tokSch) || equals(token, tokRGB) || equals(token, tokTime))
        {
            ts.SetAt(token, data);
            continue;
        }

        if (bEof || !ts.IsEmpty())
        {
            if (pSch)
            {
                if (!pSch->FinishReading())
                    delete pSch;
                else
                    arrSchedule.Add(pSch);
            }
            pSch = NULL;
            if (bEof) break;

            pSch = new TSchedule();
            pSch->Read(ts);
            ts.RemoveAll();
        }
        else if (pSch == NULL)
            ThrowMessage(IDP_BAD_TOKEN, token);

        if (equals(token, TID::GetToken(tyMch)))
        {
            TMachine* pMch = TSequence::ReadMch(data);
            pSeq = pSch->FindMch(pMch);
            ASSERT(pSeq);
            continue;
        }

        if (equals(token, tokOper))
        {
            if (pSeq == NULL) ThrowBadOp(strEmpty, IDS_SCH);
            pSeq->ReadOper(data);
            continue;
        }

        ThrowMessage(IDP_BAD_TOKEN, token);
    }
}
