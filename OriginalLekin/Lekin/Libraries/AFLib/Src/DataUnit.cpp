#include "StdAfx.h"

#include "AFLibDataUnit.h"

#include "AFLibExcBreak.h"
#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;
using AFLibGui::TDataUnit;

TDataUnit::TDataUnit(LPCTSTR key, UINT idc, int nIdc, UINT idsHelp) :
    m_key(key),
    m_idc(idc),
    m_nIdc(nIdc),
    m_idsHelp(idsHelp),
    m_index(0)
{}

TDataUnit::~TDataUnit()
{}

bool TDataUnit::ContainsIdc(UINT idc) const
{
    return idc >= m_idc && idc < m_idc + m_nIdc;
}

void TDataUnit::Load(LPCTSTR section) const
{
    CString s = DynamicGetProfileString(section, m_key, strNoData);
    if (s == strNoData) return;
    TStringSpacer sp(s);

    try
    {
        Read(sp);
    }
    catch (AFLibThread::CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }
}

void TDataUnit::Save(LPCTSTR section) const
{
    TStringBuffer sb;
    Write(sb);
    DynamicWriteProfileString(section, m_key, sb);
}

bool TDataUnit::PeerNeeded(int index) const
{
    return true;
}

bool TDataUnit::IsLoadable() const
{
    return !m_key.IsEmpty();
}
