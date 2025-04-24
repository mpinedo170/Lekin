#include "StdAfx.h"

#include "AFLibDitem.h"

#include "AFLibExcMessage.h"
#include "AFLibGlobal.h"
#include "AFLibStringTokenizer.h"
#include "AFLibSuperText.h"
#include "Local.h"

using namespace AFLibGui;
using namespace AFLib;
using namespace AFLibIO;

TDitem::~TDitem()
{}

void TDitem::SetTitle(LPCTSTR title)
{
    m_title = title;
    TestTitle();
}

void TDitem::SetDir(LPCTSTR dir)
{
    m_dir = dir;
    AppendSlash(m_dir);
}

void TDitem::PrivateLoadPath(LPCTSTR path, bool bTest)
{
    TStringTokenizer sp2(path, chrBSlash);
    m_dir.Empty();

    while (true)
    {
        m_title = sp2.ReadStr();
        if (bTest) TestTitle();
        if (sp2.IsEof()) break;
        m_dir += m_title + strBSlash;
    }
}

void TDitem::LoadPath(LPCTSTR path)
{
    PrivateLoadPath(path, true);
}

void TDitem::LoadPathNoTest(LPCTSTR path)
{
    if (IsEmptyStr(path))
    {
        m_dir.Empty();
        m_title.Empty();
    }
    else
        PrivateLoadPath(path, false);
}

void TDitem::TestTitle() const
{
    CString msg;

    try
    {
        TestFileName(m_title, false);
        return;
    }
    catch (CExcMessage* pExc)
    {
        msg = GetExceptionMsg(pExc);
        if (msg.IsEmpty()) throw;
        pExc->Delete();
    }

    const CString& type = GetDitemType();
    if (!type.IsEmpty())
    {
        msg.SetAt(0, ToLower(msg[0]));
        msg = type + strSpace + msg;
    }

    ThrowMessage(msg);
}

void TDitem::Describe(TSuperText& superText) const
{
    if (m_title.IsEmpty()) return;
    superText.SetBold(true);
    superText.SetUnderline(true);
    superText.Add(m_title);
    superText.SetBold(false);
    superText.SetUnderline(false);
    superText.Add(strEol);
}

bool TDitem::Equals(const TDitem& ditem) const
{
    return this == &ditem || GetPath() == ditem.GetPath();
}
