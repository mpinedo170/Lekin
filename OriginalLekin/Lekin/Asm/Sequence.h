#pragma once

#include "Operation.h"

class TMachine;

struct TSequence
{
    TMachine* m_pMch;         // machine this sequence describes
    int m_release;            // current release date of machine
    TCHAR m_status;           // current status of machine
    TOperationArray m_arrOp;  // set of operations
    int m_index;              // index in the operation queue

    TSequence();
    explicit TSequence(TMachine* pMch);
    TSequence(const TSequence& seq);
    TSequence& operator=(const TSequence& seq);
    bool operator==(const TSequence& seq);

    bool operator!=(const TSequence& seq)
    {
        return !(*this == seq);
    }

    void Save(TStringWriter& sw) const;
    void Load(TStringSpacer& sp);

    void Write(CArchive& ar);
    static TMachine* ReadMch(LPCTSTR name);
    void ReadOper(LPCTSTR name);
    void Init();

    TOperationArray& GetAvail();

    const TOperationArray& GetAvail() const
    {
        return const_cast<TSequence*>(this)->GetAvail();
    }

    int GetAvailCount() const;
    int GetAvailTime() const;
    int GetOpCount() const
    {
        return m_arrOp.GetSize();
    }

    int GetStart(TOperation* pOp) const;  // start             stop
    int GetSetup(TOperation* pOp) const;  //  |     |          |
    int GetStop(TOperation* pOp) const;   //   setup processing
    void Add(TOperation* pOp);
};
