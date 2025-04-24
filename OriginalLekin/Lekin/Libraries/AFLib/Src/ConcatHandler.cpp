#include "StdAfx.h"

#include "AFLibConcatHandler.h"

#include "AFLibGlobal.h"
#include "AFLibStrings.h"

using AFLibPrivate::TConcatHandler;

TConcatHandler::TConcatHandler()
{
    m_bConcatOn = true;
}

TConcatHandler::~TConcatHandler()
{}

void TConcatHandler::SetConcat(bool bOn)
{
    m_bConcatOn = bOn;
}

bool TConcatHandler::GetConcat()
{
    return m_bConcatOn;
}

bool TConcatHandler::IsChrSafe(TCHAR ch)
{
    if (ch <= _T(' ') || ch >= 127) return false;
    return AFLib::IsAlnumU(ch) || ch == AFLib::chrAt;
}

TConcatHandler::TConcatManipulator::TConcatManipulator(TConcatHandler& host, bool bConcatOn) : m_host(host)
{
    m_bConcatOnSaved = m_host.GetConcat();
    m_host.SetConcat(bConcatOn);
}

TConcatHandler::TConcatManipulator::~TConcatManipulator()
{
    m_host.SetConcat(m_bConcatOnSaved);
}
