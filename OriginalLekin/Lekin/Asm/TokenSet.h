#pragma once

class TTokenSet2 : public TTokenSet
{
public:
    TTokenSet2(CArchive& ar) : TTokenSet(ar)
    {}
    int ReadInt(LPCTSTR token, int mn, int mx, int def);
    TCHAR ReadStatus(TCHAR def = _T('A'));
    int ReadRelease(int def = 0);
};
