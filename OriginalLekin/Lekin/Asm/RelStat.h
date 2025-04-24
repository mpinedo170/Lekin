#pragma once

struct TRelStat
{
    int m_release;   // release date
    TCHAR m_status;  // starting status

    TRelStat();
    TRelStat(int release, TCHAR status);

    CString Save(int defRelease, TCHAR defStatus) const;
    void Load(TStringSpacer& sp);
};
