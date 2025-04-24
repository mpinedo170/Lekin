#include "StdAfx.h"

#include "AppAsm.h"
#include "MarginGantt.h"

namespace {
const CString keyGanttX = _T("Pages per row");
const CString keyGanttY = _T("Pages per column");
const CString keyGanttNoSplit = _T("Do not split");
}  // namespace

IMPLEMENT_DYNCREATE(TMarginGantt, TMargin)

TMarginGantt::TMarginGantt()
{
    Init();
}

TMarginGantt::TMarginGantt(LPCTSTR section) : super(section)
{
    Init();
}

void TMarginGantt::Copy(const TMargin* pMargin)
{
    super::Copy(pMargin);
    TMarginGantt* pMargin2 = (TMarginGantt*)pMargin;
    m_nX = pMargin2->m_nX;
    m_nY = pMargin2->m_nY;
    m_bNoSplit = pMargin2->m_bNoSplit;
}

void TMarginGantt::Init()
{
    m_nX = m_nY = 1;
    m_bNoSplit = true;
}

void TMarginGantt::LoadReg()
{
    super::LoadReg();
    m_nX = theApp.GetUserInt(GetSection(), keyGanttX, m_nX);
    m_nY = theApp.GetUserInt(GetSection(), keyGanttY, m_nY);
    m_bNoSplit = theApp.GetUserInt(GetSection(), keyGanttNoSplit, m_bNoSplit) != 0;
}

void TMarginGantt::SaveReg()
{
    super::SaveReg();
    theApp.WriteUserInt(GetSection(), keyGanttX, m_nX);
    theApp.WriteUserInt(GetSection(), keyGanttY, m_nY);
    theApp.WriteUserInt(GetSection(), keyGanttNoSplit, m_bNoSplit);
}
