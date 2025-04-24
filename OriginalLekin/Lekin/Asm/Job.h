#pragma once

#include "ID.h"
#include "Operation.h"

class TJob : public TID
{
private:
    typedef TID super;

    static TShop m_shop;
    bool PrivateInvalidate(bool bForce, TOperationArray& arrOpOrphan);
    void PrivateInvalidateFlowShop();
    void PrivateInvalidateOneShop();
    void PrivateSetShop();

public:
    int m_release;  // job release date
    int m_due;      // job due date
    int m_weight;   // job priority
    TOperationArray m_arrOp;

    static TShop GetShop()
    {
        return m_shop;
    }
    static void SetShop(TShop shop);
    static CString GetLabel(TShop shop);
    static CString GetSettingName()
    {
        return GetLabel(m_shop);
    }

    static void InvalidateShop();
    static bool DeleteEmpty();
    static bool DuePresent();
    static bool ReleasePresent();

    TJob();
    ~TJob();
    void Write(CArchive& ar);
    void Read(TTokenSet2& ts);

    void Save(TStringWriter& sw) const;
    void Load(TStringSpacer& sp);

    TOperation* AddOperation(LPCTSTR idWkc, int proc, TCHAR status);
    TOperation* FindOperation(CString idWkc, TOperation* pOpExclude = NULL);

    int GetProcTime() const;
    int GetOpCount() const
    {
        return m_arrOp.GetSize();
    }

    bool IsValid();
    void Invalidate(TOperationArray& arrOpOrphan);

    int GetStart();
    int GetStop(int which = 0);
    int GetT();
    int GetWT();
    bool IsLate();
};

class TJobArray : public TIDArray<TJob>
{};

extern TJobArray arrJob;

void SerializeJobs(CArchive& ar);
