#include "StdAfx.h"

#include "AFLibIDataUnit.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"

using namespace AFLib;
using namespace AFLibMath;
using namespace AFLibIO;
using AFLibGui::TIDataUnit;

TIDataUnit::TIDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, int nIdc, UINT idsHelp) :
    super(key, idc, nIdc, idsHelp),
    m_data(data),
    m_init(init),
    m_zero(zero)
{
    m_data = NanI;
}

void TIDataUnit::Load(LPCTSTR section) const
{
    int data = DynamicGetProfileInt(section, m_key, NanI);
    if (!IsNan(data)) m_data = data;
}

void TIDataUnit::Save(LPCTSTR section) const
{
    DynamicWriteProfileInt(section, m_key, m_data);
}

void TIDataUnit::Zero() const
{
    m_data = m_zero;
}

void TIDataUnit::Init() const
{
    m_data = m_init;
}

void TIDataUnit::Write(TStringBuffer& sb) const
{
    sb.AppendInt(m_data);
}

void TIDataUnit::Read(TStringSpacer& sp) const
{
    m_data = sp.ReadInt();
}

bool TIDataUnit::IsUndefined() const
{
    return IsNan(m_data);
}
