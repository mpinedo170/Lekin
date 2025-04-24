#include "StdAfx.h"

#include "AFLibRecordsetParam.h"

using AFLibDB::TRecordsetParam;
using namespace AFLib;

TRecordsetParam::TRecordsetParam()
{
    m_nBulk = m_maxCharBuffer = m_maxLongCharBuffer = 0;
    m_bSmartRows = m_bThrowOnBadData = m_bLocalMem = t3Undef;
}

void TRecordsetParam::Default()
{
    m_bSmartRows = true;
    m_bThrowOnBadData = false;
    m_bLocalMem = true;
    m_nBulk = 1 << 10;
    m_maxCharBuffer = 1 << 10;
    m_maxLongCharBuffer = 1 << 12;
}

void TRecordsetParam::Change(const TRecordsetParam& param)
{
    if (param.m_bSmartRows != t3Undef) m_bSmartRows = param.m_bSmartRows;

    if (param.m_bThrowOnBadData != t3Undef) m_bThrowOnBadData = param.m_bThrowOnBadData;

    if (param.m_bLocalMem != t3Undef) m_bLocalMem = param.m_bLocalMem;

    if (param.m_nBulk > 0) m_nBulk = param.m_nBulk;

    if (param.m_maxCharBuffer > 0) m_maxCharBuffer = max(param.m_maxCharBuffer, 16);

    if (param.m_maxLongCharBuffer > 0) m_maxLongCharBuffer = max(param.m_maxLongCharBuffer, 16);
}
