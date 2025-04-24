#include "StdAfx.h"

#include "AFLibQuaDataUnit.h"

#include "AFLibStringSpacer.h"

using namespace AFLibIO;
using AFLibGui::TQuaDataUnit;
using AFLibMath::TQuantity;

TQuaDataUnit::TQuaDataUnit(
    LPCTSTR key, UINT idc, TQuantity& data, const TQuantity& init, const TQuantity& zero, UINT idsHelp) :
    super(key, idc, 1, idsHelp),
    m_data(data),
    m_init(init),
    m_zero(zero)
{}

void TQuaDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    m_data.DDX(pDX, m_idc, bUndefOk);
}

void TQuaDataUnit::Zero() const
{
    m_data = m_zero;
}

void TQuaDataUnit::Init() const
{
    m_data = m_init;
}

void TQuaDataUnit::Read(TStringSpacer& sp) const
{
    TQuantity data;
    data.Load(sp);
    m_data = data;
}

void TQuaDataUnit::Write(TStringBuffer& sb) const
{
    m_data.Save(sb);
}

bool TQuaDataUnit::IsUndefined() const
{
    return m_data.IsNan();
}
