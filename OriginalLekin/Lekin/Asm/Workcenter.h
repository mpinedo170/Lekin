#pragma once

#include "Machine.h"
#include "Operation.h"
#include "Setup.h"

class TWorkcenter : public TID, public TRelStat
{
private:
    typedef TID super;

    static bool m_bFlexible;  // NOT restrict to 1 machine per workcenter
    TMachine* PrivateSingleMachine();

    void Init();

public:
    static bool IsFlexible()
    {
        return m_bFlexible;
    }

    static void SetFlexible(bool bFlexible);
    static CString GetLabel(int id);
    static CString GetSettingName()
    {
        return GetLabel(int(m_bFlexible));
    }
    static void SingleMachine();

    static bool SetupPresent();
    static bool AvailPresent();

    int m_temp;  // just in case...

    TSetup m_setup;  // setup times
    TMachineArray m_arrMch;

    // used in scheduling
    TOperationArray m_queue;
    int m_earliest;
    double m_avS;
    double m_avP;

    TWorkcenter();
    TWorkcenter(const TID& style, int machines, int release, TCHAR status);
    ~TWorkcenter();

    void Write(CArchive& ar);
    void Read(TTokenSet2& ts);

    void Save(TStringWriter& sw) const;
    void Load(TStringSpacer& sp);

    void ChangeName(LPCTSTR nameNew, bool bMoveOpers);
    bool HasOpers() const;

    void ComputeAv();
    TMachine* FindMch(LPCTSTR id);

    int GetMchCount() const
    {
        return m_arrMch.GetSize();
    }
};

class TWorkcenterArray : public TIDArray<TWorkcenter>
{};

extern TWorkcenterArray arrWorkcenter;

void SerializeMch(CArchive& ar);
