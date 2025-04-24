#include "StdAfx.h"

#include "AFLibCardIntervalSet.h"

#include "AFLibGlobal.h"

using namespace AFLib;
using namespace AFLibIO;

const double TDblIntervalSet::m_privateMax = DBL_MAX;

namespace {
const CString strInf = _T("inf");
}

void TCharIntervalSet::ValToString(CHAR val, TStringBuffer& s) const
{
    if (m_min != 0 && val == m_min)
    {
        s += chrDash;
        s += strInf;
    }
    else if (val == m_max)
        s += strInf;
    else
        s.Append(&val, 1);
}

void TWCharIntervalSet::ValToString(WCHAR val, TStringBuffer& s) const
{
    if (m_min != 0 && val == m_min)
    {
        s += chrDash;
        s += strInf;
    }
    else if (val == m_max)
        s += strInf;
    else
        s.Append(&val, 1);
}

void TIntIntervalSet::ValToString(int val, TStringBuffer& s) const
{
    if (val == m_min)
    {
        s += chrDash;
        s += strInf;
    }
    else if (val == m_max)
        s += strInf;
    else
        s.AppendInt(val);
}

void TUintIntervalSet::ValToString(UINT val, TStringBuffer& s) const
{
    if (val == m_max)
        s += strInf;
    else
        s.AppendUint(val);
}

void TDblIntervalSet::ValToString(double val, TStringBuffer& s) const
{
    if (val == m_min)
    {
        s += chrDash;
        s += strInf;
    }
    else if (val == m_max)
        s += strInf;
    else
        s.AppendDouble(val);
}
