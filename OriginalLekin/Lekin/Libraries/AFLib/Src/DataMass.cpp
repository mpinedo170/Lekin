#include "StdAfx.h"

#include "AFLibDataMass.h"

#include "AFLib3State.h"
#include "AFLibDataUnit.h"
#include "AFLibExcMessage.h"
#include "AFLibMathGlobal.h"

using namespace AFLib;
using namespace AFLibMath;
using AFLibGui::TDataMass;
using AFLibGui::TDataUnit;

TDataMass::TDataMass()
{}

TDataMass::~TDataMass()
{
    m_arrDU.DestroyAll();
}

void TDataMass::AddDU(TDataUnit* pDu)
{
    pDu->m_index = GetDUCount();
    m_arrDU.Add(pDu);
    pDu->Init();
    if (IsDULoadable(pDu)) pDu->Load(m_section);
}

void TDataMass::InsertDU(TDataUnit* pDu)
{
    pDu->m_index = 0;
    for (int i = 0; i < GetDUCount(); ++i) ++m_arrDU[i]->m_index;
    m_arrDU.InsertAt(0, pDu);
}

int TDataMass::GetDUCount()
{
    return m_arrDU.GetSize();
}

const TDataUnit* TDataMass::GetDU(int index)
{
    return m_arrDU[index];
}

const TDataUnit* TDataMass::FindDU(UINT idc)
{
    for (int i = 0; i < GetDUCount(); ++i)
    {
        const TDataUnit* pDu = GetDU(i);
        if (pDu->ContainsIdc(idc)) return pDu;
    }
    return NULL;
}

bool TDataMass::IsDULoadable(const TDataUnit* pDu)
{
    return !m_section.IsEmpty() && pDu->IsLoadable();
}

void TDataMass::ZeroDU(const TDataUnit* pDu)
{
    pDu->Zero();
}

void TDataMass::SaveDU(const TDataUnit* pDu)
{
    pDu->Save(m_section);
}

void TDataMass::DDXDU(CDataExchange* pDX, const TDataUnit* pDu)
{
    pDu->DDX(pDX, IsDUMultiSel(pDu));
}

bool TDataMass::IsDUNeeded(const TDataUnit* pDu)
{
    return true;
}

bool TDataMass::IsDUMultiSel(const TDataUnit* pDu)
{
    return false;
}

bool TDataMass::IsDUUndefined(const TDataUnit* pDu)
{
    return pDu->IsUndefined();
}

void TDataMass::InitData()
{
    for (int i = 0; i < GetDUCount(); ++i) GetDU(i)->Init();
}

void TDataMass::LoadData()
{
    for (int i = 0; i < GetDUCount(); ++i)
    {
        const TDataUnit* pDu = GetDU(i);
        pDu->Init();
        if (IsDULoadable(pDu)) pDu->Load(m_section);
    }
}

void TDataMass::SaveData()
{
    if (m_section.IsEmpty()) return;

    for (int i = 0; i < GetDUCount(); ++i)
    {
        const TDataUnit* pDu = GetDU(i);
        if (pDu->m_key.IsEmpty()) continue;
        if (IsDUMultiSel(pDu) && IsDUUndefined(pDu)) continue;
        if (!IsDUNeeded(pDu)) continue;
        SaveDU(pDu);
    }
}

void TDataMass::ValidateData()
{
    for (int i = 0; i < GetDUCount(); ++i)
    {
        const TDataUnit* pDu = GetDU(i);
        if (!IsDUNeeded(pDu)) ZeroDU(pDu);
    }
}

void TDataMass::DDXData(CDataExchange* pDX, int iStart, int iEnd, bool bGoodOnly)
{
    try
    {
        for (int i = iStart; i <= iEnd; ++i)
        {
            const TDataUnit* pDu = GetDU(i);
            if (bGoodOnly && !IsDUNeeded(pDu)) continue;

            if (pDX->m_pDlgWnd->GetDlgItem(pDu->m_idc) != NULL) pDX->PrepareCtrl(pDu->m_idc);
            DDXDU(pDX, pDu);
        }
    }
    catch (CExcMessage* pExc)
    {
        pExc->ReportError();
        pExc->Delete();
        pDX->Fail();
    }
}

void TDataMass::FillMultiSel(int selCount, int& dataDlg, int dataItem)
{
    if (selCount == 0)
        dataDlg = dataItem;
    else if (dataDlg != dataItem)
        dataDlg = NanI;
}

void TDataMass::FillMultiSel(int selCount, T3State& dataDlg, bool dataItem)
{
    if (selCount == 0)
        dataDlg = dataItem;
    else if (dataDlg != dataItem)
        dataDlg = t3Undef;
}

void TDataMass::FillMultiSel(int selCount, CString& dataDlg, LPCTSTR dataItem)
{
    if (selCount == 0)
        dataDlg = dataItem;
    else if (dataDlg != dataItem)
        dataDlg.Empty();
}

void TDataMass::FillMultiSel(int selCount, CString& dataDlg, TCHAR dataItem)
{
    const TCHAR dataItem2[2] = {dataItem, 0};
    FillMultiSel(selCount, dataDlg, dataItem2);
}

void TDataMass::FillMultiSel(int selCount, double& dataDlg, double dataItem)
{
    if (selCount == 0)
        dataDlg = dataItem;
    else if (dataDlg != dataItem)
        dataDlg = Nan;
}

void TDataMass::ReadMultiSel(int dataDlg, int& dataItem)
{
    if (dataDlg != NanI) dataItem = dataDlg;
}

void TDataMass::ReadMultiSel(const T3State& dataDlg, bool& dataItem)
{
    if (dataDlg != t3Undef) dataItem = dataDlg;
}

void TDataMass::ReadMultiSel(LPCTSTR dataDlg, CString& dataItem)
{
    if (dataDlg[0]) dataItem = dataDlg;
}

void TDataMass::ReadMultiSel(LPCTSTR dataDlg, TCHAR& dataItem)
{
    if (dataDlg[0]) dataItem = dataDlg[0];
}

void TDataMass::ReadMultiSel(double dataDlg, double& dataItem)
{
    if (!IsNan(dataDlg)) dataItem = dataDlg;
}
