#include "StdAfx.h"

#include "AFLibExcMsgBox.h"

using AFLib::CExcMsgBox;

IMPLEMENT_DYNAMIC(CExcMsgBox, super)

CExcMsgBox::CExcMsgBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt) : super(nIDPrompt, lpszPrompt)
{
    m_nType = nType;
}

int CExcMsgBox::ReportError(UINT nType, UINT nMessageID)
{
    return super::ReportError(nType == -1 ? m_nType : nType, nMessageID);
}
