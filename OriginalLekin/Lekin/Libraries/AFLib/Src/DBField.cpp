#include "StdAfx.h"

#include "AFLibDBField.h"

using AFLibDB::TDBField;
using namespace AFLib;

namespace {
int MaxFieldID = 1 << 12;
}

CIntMap<CString, LPCTSTR> TDBField::m_mapID;
int TDBField::m_IDGlobal = 0;

TDBField::TDBField(LPCTSTR str) : m_str(str), m_ID(-1)
{
    if (m_mapID.Lookup(m_str, m_ID)) return;
    if (m_IDGlobal >= MaxFieldID) return;

    m_ID = m_IDGlobal;
    ++m_IDGlobal;
    m_mapID.SetAt(m_str, m_ID);
}
