#include "StdAfx.h"

#include "AFLibExcMessage.h"

#include "AFLibBlockMBCounter.h"
#include "AFLibExcMsgBox.h"
#include "AFLibGlobal.h"
#include "AFLibModalCounter.h"
#include "Local.h"

using namespace AFLibPrivate;
using namespace AFLibGui;
using AFLib::CExcMessage;

IMPLEMENT_DYNAMIC(CExcMessage, super)

CExcMessage::CExcMessage(LPCTSTR message)
{
    m_idsHelp = 0;
    m_message = message;
}

CExcMessage::CExcMessage(UINT idsHelp, LPCTSTR message)
{
    m_idsHelp = idsHelp;
    if (!IsEmptyStr(message))
        m_message = message;
    else if (idsHelp != 0)
        m_message.LoadString(idsHelp);
}

void CExcMessage::Prefix(LPCTSTR prefix)
{
    m_message = prefix + strPrefixer + m_message;
}

void CExcMessage::PrefixLine(LPCTSTR prefix)
{
    if (m_message.IsEmpty())
        m_message = prefix;
    else
    {
        Capitalize(m_message);
        m_message = prefix + strNewLine + m_message;
    }
}

void CExcMessage::Postfix(LPCTSTR postfix)
{
    m_message += postfix;
}

void CExcMessage::Prefix(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    Prefix(FormatStrings(AfxGetApp()->m_hInstance, ids, &str1));
}

void CExcMessage::PrefixLine(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    PrefixLine(FormatStrings(AfxGetApp()->m_hInstance, ids, &str1));
}

void CExcMessage::Postfix(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    Postfix(FormatStrings(AfxGetApp()->m_hInstance, ids, &str1));
}

int CExcMessage::ReportError(UINT nType, UINT nError)
{
    if (TBlockMBCounter::IsMsgBoxBlocked())
    {
        CExcMsgBox* pExc = new CExcMsgBox(GetExceptionMsg(this), nType, m_idsHelp);
        Delete();
        throw pExc;
    }

    TModalCounter modalCounter;
    return super::ReportError(nType, nError);
}

BOOL CExcMessage::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, LPUINT pnHelpContext)
{
    if (pnHelpContext != NULL) *pnHelpContext = m_idsHelp;
    _tcsncpy_s(lpszError, nMaxError, m_message, nMaxError - 1);
    lpszError[nMaxError - 1] = 0;
    Capitalize(lpszError);
    return true;
}
