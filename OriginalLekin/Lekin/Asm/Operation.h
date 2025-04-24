#pragma once

class TID;
class TJob;
class TWorkcenter;
struct TSequence;
class TTokenSet2;

class TOperation
{
private:
    void Init();

public:
    struct TTimingData
    {
        TSequence* m_pSeq;  // sequence scheduled in
        int m_tag;          // number in the queue
        int m_rel;          // extra release date (=0 for active schedule)
        int m_setup;        // setup time
        int m_finish;       // completion time
        TCHAR m_prevStat;   // previous status

        void Init();
    };

    TJob* m_pJob;     // daddy job
    CString m_idWkc;  // operation workcenter
    int m_proc;       // operation processing time
    TCHAR m_status;   // operation leaves the machine in this state

    // These fields are used for scheduling
    TWorkcenter* m_pWkc;    // operation workcenter
    TOperation* m_pOpPrev;  // previous operation
    TOperation* m_pOpNext;  // next operation
    int m_rpt;              // remaining processing time
    int m_release;          // release time

    TTimingData m_tm;   // permanent timing data (for viewing)
    TTimingData m_tm2;  // temporary timing data (for recomputing)

    explicit TOperation(TJob* pJob);
    TOperation(TJob* pJob, LPCTSTR idWkc, int proc, TCHAR status);

    const CString& GetGLKey() const
    {
        return m_idWkc;
    }

    void Write(CArchive& ar);
    void Read(TTokenSet2& ts);
    void DestroyThis();  // deletes this
                         // call DestroyThis() instead of operator delete!

    void Save(TStringWriter& sw) const;
    void Load(TStringSpacer& sp);

    void QueueAdd();
    void QueueDel();

    TWorkcenter* GetWkc();
    TID* GetStyle();
    int GetIndex() const;

    int GetPreStart() const;
    int GetStart() const;

    int GetStop() const
    {
        return m_tm.m_finish;
    }

    int GetStop2() const
    {
        return m_tm2.m_finish;
    }

    CString GetStatString() const;
};

class TOperationArray : public CKeyedStrArray<TOperation>
{};
