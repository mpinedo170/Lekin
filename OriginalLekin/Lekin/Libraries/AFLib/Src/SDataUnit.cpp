#include "StdAfx.h"

#include "AFLibSDataUnit.h"

#include "AFLibGlobal.h"
#include "AFLibStringSpacer.h"
#include "AFLibStringWriter.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using namespace AFLibIO;
using AFLibGui::TSDataUnit;

TSDataUnit::TSDataUnit(LPCTSTR key, UINT idc, CString& data, LPCTSTR init, LPCTSTR zero, UINT idsHelp) :
    super(key, idc, 1, idsHelp),
    m_data(data),
    m_init(init),
    m_zero(zero)
{
    m_data = strNoData;
}

void TSDataUnit::Load(LPCTSTR section) const
{
    CString s = DynamicGetProfileString(section, m_key, strNoData);
    if (s != strNoData) m_data = s;
}

void TSDataUnit::Save(LPCTSTR section) const
{
    DynamicWriteProfileString(section, m_key, m_data);
}

void TSDataUnit::Zero() const
{
    m_data = m_zero;
}

void TSDataUnit::Init() const
{
    m_data = m_init;
}

void TSDataUnit::Read(TStringSpacer& sp) const
{
    m_data = sp.ReadCStr();
}

void TSDataUnit::Write(TStringBuffer& sb) const
{
    TStringWriter sw;
    sw.WriteCStr(m_data);
    sb += sw.GetBuffer();
}

bool TSDataUnit::IsUndefined() const
{
    return m_data.IsEmpty();
}
