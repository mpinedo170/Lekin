#include "StdAfx.h"

#include "AppAsm.h"
#include "Workcenter.h"

#include "Job.h"
#include "Misc.h"
#include "TokenSet.h"

////////////////////////////////////////////
// TWorkcenter

extern const CString tokComment = _T("Comment");
extern const CString tokStatus = _T("Status");

TWorkcenterArray arrWorkcenter;

bool TWorkcenter::m_bFlexible = true;

void TWorkcenter::SetFlexible(bool bFlexible)
{
    if (m_bFlexible == bFlexible) return;

    EnumerateWkc pWkc->m_arrMch.DestroyAll();
    EnumerateEnd

        m_bFlexible = bFlexible;
    SingleMachine();
}

CString TWorkcenter::GetLabel(int id)
{
    CString s(MAKEINTRESOURCE(IDS_PARK0 + id));
    return s;
}

bool TWorkcenter::SetupPresent()
{
    EnumerateWkc if (pWkc->m_setup.GetDim() > 0) return true;
    EnumerateEnd return false;
}

bool TWorkcenter::AvailPresent()
{
    EnumerateWkc for (int i = 0; i < pWkc->GetMchCount();
                      ++i) if (pWkc->m_arrMch[i]->m_release > 0) return true;
    EnumerateEnd return false;
}

void TWorkcenter::Init()
{
    m_temp = 0;
    m_avP = 1;
    m_avS = 0;
    m_earliest = 0;
}

TWorkcenter::TWorkcenter() : super(tyWkc)
{
    Init();
}

TWorkcenter::TWorkcenter(const TID& style, int machines, int release, TCHAR status) :
    super(style),
    TRelStat(release, status)
{
    Init();
    if (IsFlexible())
    {
        m_arrMch.SetSize(machines);
        for (int i = 0; i < machines; ++i)
        {
            CString s;
            s.Format(_T("%s.%02d"), m_id, i + 1);
            m_arrMch[i] = new TMachine(this, s, m_release, m_status);
        }
    }
    else
        PrivateSingleMachine();
}

void TWorkcenter::Write(CArchive& ar)
{
    super::Write(ar);
    if (!IsShort())
    {
        WriteToken(ar, 2, tokRelease, m_release);
        WriteToken(ar, 2, tokStatus, CString(m_status));
    }
    m_setup.Write(ar);

    if (!IsFlexible() && !IsShort()) return;
    for (int i = 0; i < GetMchCount(); ++i) m_arrMch[i]->Write(ar);
}

void TWorkcenter::Read(TTokenSet2& ts)
{
    super::Read(ts);
    if (arrWorkcenter.Exists(m_id)) ThrowDupID(m_id, IDS_WKC);

    m_release = ts.ReadRelease();
    m_status = ts.ReadStatus();

    m_setup.Read(ts);
}

namespace {
const CString keyMch = _T("Machines");
const CString keyM = _T("M");
}  // namespace

void TWorkcenter::Save(TStringWriter& sw) const
{
    super::Save(sw);
    sw.Write(TRelStat::Save(-1, 0));
    m_setup.Save(sw);

    if (!IsFlexible()) return;

    sw.Write(keyMch + strEqual + IntToStr(GetMchCount()));
    bool bFirst = true;

    for (int i = 0; i < GetMchCount(); ++i)
    {
        if (m_arrMch[i]->IsDefault()) continue;
        if (bFirst) sw.NewLine();
        bFirst = false;
        sw.Write(keyM + IntToStr(i) + strEqual + m_arrMch[i]->Save());
    }

    sw.NewLine();
}

void TWorkcenter::Load(TStringSpacer& sp)
{
    try
    {
        super::Load(sp);
        TRelStat::Load(sp);
        m_setup.Load(sp);

        sp.TestSymbolHard(keyMch);
        sp.TestSymbolHard(strEqual);

        int nMch = sp.ReadInt();
        CheckInt(nMch, 1, MaxMch);

        m_arrMch.DestroyAll();
        for (int i = 0; i < nMch; ++i) m_arrMch.Add(new TMachine(this));

        while (sp.TestSymbol(keyM))
        {
            int index = sp.ReadInt();
            CheckInt(index, 0, nMch - 1);
            sp.TestSymbolHard(strEqual);
            m_arrMch[index]->Load(sp);
        }
    }
    catch (CExcMessage* pExc)
    {
        pExc->Prefix(GetToken());
    }
}

TWorkcenter::~TWorkcenter()
{
    m_arrMch.DestroyAll();
}

bool TWorkcenter::HasOpers() const
{
    EnumerateJob if (pJob->FindOperation(m_id)) return true;
    EnumerateEnd return false;
}

void TWorkcenter::ChangeName(LPCTSTR newName, bool bMoveOpers)
{
    if (bMoveOpers)
    {
        EnumerateJob TOperation* pOp = pJob->FindOperation(m_id);
        if (pOp) pOp->m_idWkc = newName;
        EnumerateEnd
    }
    m_id = newName;
}

void TWorkcenter::ComputeAv()
{
    int status[26] = {0};
    int count = 0, p = 0;

    EnumerateJob TOperation* pOp = pJob->FindOperation(m_id);
    if (!pOp) continue;
    ++count;
    ++status[pOp->m_status - 'A'];
    p += pOp->m_proc;
    EnumerateEnd

        m_avP = 1;
    m_avS = 0;
    if (!count) return;

    int dim = m_setup.GetDim();
    if (dim > 0)
    {
        double s = 0;
        for (int i = 0; i < dim; ++i)
            for (int j = 0; j < dim; ++j) s += status[i] * status[j] * m_setup.GetAt(i, j);
        m_avS = s / (count * count);
    }
    m_avP = double(p) / count;
}

TMachine* TWorkcenter::PrivateSingleMachine()
{
    TMachine* pMch = NULL;
    int count = m_arrMch.GetSize();
    if (count == 0)
    {
        pMch = new TMachine(this, m_id, m_release, m_status);
        m_arrMch.Add(pMch);
    }
    else
    {
        m_arrMch.DestroyAt(1, count - 1);
        pMch = m_arrMch[0];
        pMch->m_id = m_id;
        pMch->m_release = m_release;
        pMch->m_status = m_status;
    }
    if (IsFlexible()) pMch->m_id += ".01";
    return pMch;
}

void TWorkcenter::SingleMachine()
{
    EnumerateWkc if (!IsFlexible() || pWkc->m_arrMch.GetSize() == 0) pWkc->PrivateSingleMachine();
    EnumerateEnd
}

TMachine* TWorkcenter::FindMch(LPCTSTR id)
{
    return m_arrMch.FindKey(id);
}
