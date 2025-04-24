#pragma once

#include "ID.h"

class TMachine;
class TOperation;
struct TSequence;

class TSchedule : public TID
{
private:
    typedef TID super;

public:
    struct TDelay
    {
        TOperation* m_pOp;
        int m_rel;

        TDelay() : m_pOp(NULL), m_rel(0)
        {}
        TDelay(TOperation* pOp, int rel) : m_pOp(pOp), m_rel(rel)
        {}
    };

    static double m_k1, m_k2;
    static int m_newTime;

    CSmartArray<TSequence> m_arrSequence;
    CArray<TDelay> m_arrDelay;
    int m_arrObj[objN];

    enum TError
    {
        Ok,
        Double,
        Missed,
        Cycle
    };

    TOperation* m_pOpError;
    TError m_error;
    bool m_bShown;

    TSchedule();
    TSchedule(const TSchedule& sch) : super(tySch)
    {
        *this = sch;
    }
    ~TSchedule();

    TSchedule& operator=(const TSchedule& sch);
    bool operator==(const TSchedule& sch);

    bool operator!=(const TSchedule& sch)
    {
        return !(*this == sch);
    }

    void Write(CArchive& ar);
    void Read(TTokenSet2& ts);
    bool FinishReading();

    void Save(TStringWriter& sw) const;
    void Load(TStringSpacer& sp);

    int GetSeqCount() const
    {
        return m_arrSequence.GetSize();
    }

    void Clear();
    void ClearObj();
    void CreateSequences();
    void PrepareTiming();
    void Create(TRule rule);
    bool Recompute(bool bUpdateTiming);
    void FormatError();

    TSequence* FindMch(const TMachine* pMch);
    void AddDelay(TOperation* pOp, int rel);
    void DelDelay(const TOperation* pOp);

    static CString GetLabel(int IDObj);

    enum TClearTiming
    {
        Completely,
        Normal,
        Extra
    };
    static void ClearTiming(TClearTiming how);
};

class TScheduleArray : public TIDArray<TSchedule>
{};

extern TScheduleArray arrSchedule;
extern TSchedule* pSchActive;

void SerializeSch(CArchive& ar);
