#pragma once

#include "ID.h"
#include "RelStat.h"

class TOperation;
class TWorkcenter;

class TMachine : public TID, public TRelStat
{
private:
    typedef TID super;

public:
    TWorkcenter* m_pWkc;  // daddy workcenter

    explicit TMachine(TWorkcenter* pWkc);
    TMachine(TWorkcenter* pWkc, LPCTSTR id, int release, TCHAR status);

    bool IsTrivial() const
    {
        return m_release == 0 && m_status == _T('A');
    }

    void Write(CArchive& ar);
    void Read(TTokenSet2& ts);

    bool IsDefault() const;
    int GetIndex() const;

    CString Save() const;
    void Load(TStringSpacer& sp);

    void DestroyThis();  // deletes this
                         // call DestroyThis() instead of operator delete!
    TOperation* FindOper(LPCTSTR idJob);
};

class TMachineArray : public TIDArray<TMachine>
{};
