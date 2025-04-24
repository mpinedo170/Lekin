#include "StdAfx.h"

#include "AFLibFuncManager.h"

#include "AFLibCalculator.h"
#include "AFLibGlobal.h"
#include "Local.h"

using namespace AFLibCalc;
using namespace AFLib;
using namespace AFLibPrivate;

TFuncManager::TFuncManager(bool bIgnoreCase)
{
    m_bIgnoreCase = bIgnoreCase;
    m_pIdCurFunc = NULL;
    m_nUndefFunc = m_maxLocalRef = 0;
    m_arrPos.SetSize(0, 1 << 8);
}

TFuncManager::~TFuncManager()
{
    m_map.DestroyAll();
}

void TFuncManager::PrivateStartFuncHelper(LPCTSTR id, LPCTSTR id2, const TFuncPrototype& fpr, int pos)
{
    TIDDef* pIdDef = m_map.GetAt(id2);

    if (pIdDef == NULL)
    {
        m_pIdCurFunc = new TIDDef(fpr, pos);
        m_map.SetAt(id2, m_pIdCurFunc);
        return;
    }

    if (pIdDef->m_pos >= 0 && pos >= 0) ThrowMessage(LocalAfxString(IDS_DUP_ID, id));

    if (pIdDef->m_fpr != fpr) ThrowMessage(LocalAfxString(IDS_PROTO_CONFLICT, id));

    if (pos < 0) return;

    int oldPos = -pIdDef->m_pos;
    while (m_arrPos.GetSize() < oldPos) m_arrPos.Add(-1);

    m_arrPos.SetAtGrow(oldPos, pos);
    pIdDef->m_pos = pos;
    m_pIdCurFunc = pIdDef;
}

void TFuncManager::PrivateStartFunc(LPCTSTR id, const TFuncPrototype& fpr, int pos)
{
    if (m_bIgnoreCase)
    {
        CString id2 = id;
        id2.MakeLower();
        PrivateStartFuncHelper(id, id2, fpr, pos);
    }
    else
        PrivateStartFuncHelper(id, id, fpr, pos);
}

void TFuncManager::StartFunc(LPCTSTR id, const TFuncPrototype& fpr, TCalculator& calc)
{
    PrivateStartFunc(id, fpr, calc.GetPos());
    calc.AddStack(0);
}

void TFuncManager::EndFunc(TCalculator& calc)
{
    if (m_pIdCurFunc == NULL) return;

    calc.SetLabel(m_pIdCurFunc->m_pos, m_maxLocalRef);
    m_pIdCurFunc = NULL;
    m_maxLocalRef = 0;
}

void TFuncManager::DeclareFunc(LPCTSTR id, const TFuncPrototype& fpr)
{
    --m_nUndefFunc;
    PrivateStartFunc(id, fpr, m_nUndefFunc);
}

void TFuncManager::AddLocalVar(int ref)
{
    if (m_pIdCurFunc == NULL) return;
    m_maxLocalRef = max(m_maxLocalRef, ref + 1);
}

const TFuncManager::TIDDef* TFuncManager::GetFuncDef(LPCTSTR id) const
{
    if (IsEmptyStr(id)) return NULL;
    if (!m_bIgnoreCase) m_map.GetAt(id);

    CString id2 = id;
    id2.MakeLower();
    return m_map.GetAt(id2);
}

int TFuncManager::GetNewFuncPos(int ref) const
{
    return ref >= 0 && ref < m_arrPos.GetSize() ? m_arrPos[ref] : -1;
}
