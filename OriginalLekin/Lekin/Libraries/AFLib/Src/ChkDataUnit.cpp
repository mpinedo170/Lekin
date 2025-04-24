#include "StdAfx.h"

#include "AFLibChkDataUnit.h"

#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"

using namespace AFLib;
using namespace AFLibMath;
using namespace AFLibIO;
using AFLibGui::TChkDataUnit;

TChkDataUnit::TChkDataUnit(LPCTSTR key, UINT idc, bool& data, bool init, bool zero, UINT idsHelp) :
    super(key, idc, 1, idsHelp),
    m_pDataBool(&data),
    m_pData3State(NULL),
    m_init(init),
    m_zero(zero)
{
    *m_pDataBool = false;
}

TChkDataUnit::TChkDataUnit(
    LPCTSTR key, UINT idc, T3State& data, const T3State& init, const T3State& zero, UINT idsHelp) :
    super(key, idc, 1, idsHelp),
    m_pDataBool(NULL),
    m_pData3State(&data),
    m_init(init),
    m_zero(zero)
{
    *m_pData3State = false;
}

T3State TChkDataUnit::GetData() const
{
    return m_pDataBool != NULL ? T3State(*m_pDataBool) : *m_pData3State;
}

int TChkDataUnit::DataToInt() const
{
    return m_pDataBool != NULL ? (*m_pDataBool ? 1 : 0) : (m_pData3State->ToInt());
}

void TChkDataUnit::IntToData(int data) const
{
    if (m_pDataBool != NULL)
        *m_pDataBool = data != 0;
    else
        *m_pData3State = T3State(data, 0);
}

void TChkDataUnit::Load(LPCTSTR section) const
{
    int data = DynamicGetProfileInt(section, m_key, NanI);
    if (!IsNan(data)) IntToData(data);
}

void TChkDataUnit::Save(LPCTSTR section) const
{
    DynamicWriteProfileInt(section, m_key, DataToInt());
}

bool TChkDataUnit::PeerNeeded(int index) const
{
    return DataToInt() != 0;
}

void TChkDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    if (m_pDataBool != NULL)
        DDX_Check(pDX, m_idc, *m_pDataBool);
    else
    {
        DDX_Check(pDX, m_idc, *m_pData3State);
        if (pDX->m_bSaveAndValidate && bUndefOk && *m_pData3State == t3Undef) *m_pData3State = true;
    }
}

void TChkDataUnit::Zero() const
{
    IntToData(m_zero.ToInt());
}

void TChkDataUnit::Init() const
{
    IntToData(m_init.ToInt());
}

void TChkDataUnit::Write(TStringBuffer& sb) const
{
    sb += BoolToPM(DataToInt() != 0);
}

void TChkDataUnit::Read(TStringSpacer& sp) const
{
    IntToData(sp.ReadPM());
}

bool TChkDataUnit::IsUndefined() const
{
    int data = DataToInt();
    return data != 0 && data != 1;
}
