#include "StdAfx.h"

#include "AppAsm.h"
#include "RelStat.h"

#include "Misc.h"

namespace {
const CString strRelease = "R";
const CString strStatus = "S";
}  // namespace

TRelStat::TRelStat() : m_release(0), m_status(_T('A'))
{}

TRelStat::TRelStat(int release, TCHAR status) : m_release(release), m_status(status)
{}

CString TRelStat::Save(int defRelease, TCHAR defStatus) const
{
    CString s;
    if (m_release != defRelease) s = strRelease + strEqual + IntToStr(m_release);

    if (m_status != defStatus)
    {
        if (!s.IsEmpty()) s += strSpace;
        s += strStatus + strEqual + m_status;
    }

    return _T("(") + s + _T(")");
}

void TRelStat::Load(TStringSpacer& sp)
{
    sp.TestSymbolHard(_T("("));

    enum
    {
        iiiRelease,
        iiiStatus
    };

    static const LPCTSTR arrKey[] = {strRelease, strStatus, NULL};

    while (true)
    {
        int iii = sp.TestSymbolArray(arrKey);
        if (iii < 0) break;

        sp.TestSymbolHard(strEqual);

        switch (iii)
        {
            case iiiRelease:
                m_release = sp.ReadInt();
                CheckInt(m_release);
                break;

            case iiiStatus:
            {
                CString status = sp.ReadAlnum();
                CheckStatus(status);
                m_status = ToUpper(status[0]);
                break;
            }
        }
    }
    sp.TestSymbolHard(_T(")"));
}
