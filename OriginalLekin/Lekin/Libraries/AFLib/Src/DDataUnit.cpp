#include "StdAfx.h"

#include "AFLibDDataUnit.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibMath;
using AFLibGui::TDDataUnit;

TDDataUnit::TDDataUnit(LPCTSTR key, UINT idc, double& data, double init, double zero, UINT idsHelp) :
    super(key, idc, 1, idsHelp),
    m_data(data),
    m_init(init),
    m_zero(zero)
{
    m_data = Nan;
}

void TDDataUnit::Zero() const
{
    m_data = m_zero;
}

void TDDataUnit::Init() const
{
    m_data = m_init;
}

void TDDataUnit::Read(TStringSpacer& sp) const
{
    m_data = sp.ReadDouble();
}

void TDDataUnit::Write(TStringBuffer& sb) const
{
    sb.AppendDouble(m_data, 10);
}

bool TDDataUnit::IsUndefined() const
{
    return IsNan(m_data);
}
