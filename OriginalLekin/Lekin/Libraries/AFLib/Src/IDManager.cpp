#include "StdAfx.h"

#include "AFLibIDManager.h"

#include "AFLibCalculator.h"
#include "AFLibGlobal.h"
#include "Local.h"

using namespace AFLibCalc;
using namespace AFLib;
using namespace AFLibPrivate;

TIDManager::TIDManager(const TIDManager* pIDMPrev, bool bIgnoreCase)
{
    PrivateInit(pIDMPrev, bIgnoreCase);
    m_pIDMLoop = m_pIDMPrev == NULL ? NULL : m_pIDMPrev->m_pIDMLoop;
}

TIDManager::TIDManager(const TIDManager* pIDMPrev, bool bIgnoreCase, LPCTSTR label)
{
    PrivateInit(pIDMPrev, bIgnoreCase);
    m_pIDMLoop = this;
    m_pIDMLoopPrev = m_pIDMPrev != NULL ? m_pIDMPrev->m_pIDMLoop : NULL;

    if (IsEmptyStr(label)) return;

    if (ProtectedFindLoopedIDM(label) != NULL) ThrowMessage(LocalAfxString(IDS_DUP_LOOP_LABEL, label));

    m_label = label;
}

TIDManager::~TIDManager()
{
    m_map.DestroyAll();
}

void TIDManager::PrivateInit(const TIDManager* pIDMPrev, bool bIgnoreCase)
{
    m_pIDMPrev = pIDMPrev;
    m_bIgnoreCase = bIgnoreCase;
    m_bAcceptContinue = true;
    m_bGlobal = true;
    m_nRef = 0;
    m_pIDMLoop = m_pIDMLoopPrev = NULL;

    if (pIDMPrev == NULL) return;

    m_bGlobal = pIDMPrev->m_bGlobal;
    m_nRef = pIDMPrev->m_nRef;
}

void TIDManager::SetLocal()
{
    if (!m_bGlobal) return;

    m_bGlobal = false;
    m_nRef = 0;
}

int TIDManager::AddVar(LPCTSTR id, TSimpleType stp, int ref)
{
    if (ref < 0) ref = m_nRef;
    if (stp.IsSimple()) stp.SetModifyer(tmdRef);

    // empty ID means reserve space, but do not add the name
    if (!IsEmptyStr(id))
    {
        if (m_bIgnoreCase)
        {
            CString id2 = id;
            id2.MakeLower();

            if (m_map.Exists(id2)) ThrowMessage(LocalAfxString(IDS_DUP_ID, id));

            m_map.SetAt(id2, new TIDDef(stp, ref, m_bGlobal));
        }
        else
        {
            if (m_map.Exists(id)) ThrowMessage(LocalAfxString(IDS_DUP_ID, id));

            m_map.SetAt(id, new TIDDef(stp, ref, m_bGlobal));
        }
    }

    m_nRef = max(m_nRef, ref + 1);
    return ref;
}

const TIDManager::TIDDef* TIDManager::GetVarDef(LPCTSTR id) const
{
    if (IsEmptyStr(id)) return NULL;

    CString id2;
    const TIDDef* pIddef = NULL;

    for (const TIDManager* pIDM = this; pIDM != NULL; pIDM = pIDM->m_pIDMPrev)
    {
        if (!pIDM->m_bIgnoreCase)
        {
            pIddef = pIDM->m_map.GetAt(id);
            if (pIddef != NULL) return pIddef;
            continue;
        }

        if (id2.IsEmpty())
        {
            id2 = id;
            id2.MakeLower();
        }

        pIddef = pIDM->m_map.GetAt(id2);
        if (pIddef != NULL) return pIddef;
    }

    return NULL;
}

void TIDManager::ConvertFuncArgs()
{
    for (POSITION pos = m_map.GetStartPosition(); pos != NULL;)
    {
        CString ID;
        TIDDef* pIddef = NULL;
        m_map.GetNextAssoc(pos, ID, pIddef);
        pIddef->m_ref -= m_nRef + 1;
    }
    m_nRef = 0;
}

TIDManager* TIDManager::ProtectedFindLoopedIDM(LPCTSTR label)
{
    for (TIDManager* pIDM = m_pIDMLoop; pIDM != NULL; pIDM = pIDM->m_pIDMLoopPrev)
    {
        int a = pIDM->m_bIgnoreCase ? pIDM->m_label.CompareNoCase(label) : pIDM->m_label.Compare(label);
        if (a == 0) return pIDM;
    }

    return NULL;
}

TIDManager* TIDManager::FindLoopedIDM(LPCTSTR label, bool bContinue)
{
    if (IsEmptyStr(label))
    {
        TIDManager* pIDM = m_pIDMLoop;

        for (; pIDM != NULL; pIDM = pIDM->m_pIDMLoopPrev)
            if (!bContinue || pIDM->AcceptsContinue()) break;

        if (pIDM == NULL) ThrowMessage(LocalAfxString(IDS_MISPLACED_BRK_CONT));
        return pIDM;
    }

    TIDManager* pIDM = ProtectedFindLoopedIDM(label);
    if (pIDM == NULL) ThrowMessage(LocalAfxString(IDS_BAD_LOOP_LABEL, label));
    if (bContinue && !pIDM->AcceptsContinue()) ThrowMessage(LocalAfxString(IDS_NON_CONT_LABEL, label));
    return pIDM;
}

void TIDManager::AddBreak(int pos)
{
    m_arrBreak.Add(pos);
}

void TIDManager::AddContinue(int pos)
{
    m_arrContinue.Add(pos);
}

void TIDManager::FinalizeLoop(TCalculator& calc, int posBreak, int posContinue)
{
    ASSERT(m_pIDMLoop == this);

    for (int i = 0; i < m_arrBreak.GetSize(); ++i) calc.SetLabel(m_arrBreak[i], posBreak);

    for (int i = 0; i < m_arrContinue.GetSize(); ++i) calc.SetLabel(m_arrContinue[i], posContinue);
}
