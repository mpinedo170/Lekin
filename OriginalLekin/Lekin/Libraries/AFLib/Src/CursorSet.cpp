#include "StdAfx.h"

#include "AFLibCursorSet.h"

#include "AFLibGuiGlobal.h"

using AFLibGui::TCursorSet;

TCursorSet::TCursorSet()
{
    Clear();
    m_timerMS = 150;
}

void TCursorSet::Init()
{
    m_hCurStop = LoadCursor(NULL, IDC_NO);
    m_hCurDrag = LoadCursor(NULL, IDC_ARROW);
    m_hCurCopy = GetArrowPlusCursor();
}

void TCursorSet::Clear()
{
    m_hCurStop = m_hCurDrag = m_hCurCopy = NULL;
}

void TCursorSet::SetCursor(bool bOk, bool bCopy) const
{
    ::SetCursor(bOk ? (bCopy ? m_hCurCopy : m_hCurDrag) : m_hCurStop);
}

void TCursorSet::RestoreCursor()
{
    ::SetCursor(LoadCursor(NULL, IDC_ARROW));
}
