#pragma once

#include "AFLib3State.h"
#include "AFLibDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TChkDataUnit
// Single check box; may be either simple boolean or 3-state

namespace AFLibGui {
class AFLIB TChkDataUnit : public TDataUnit
{
private:
    typedef TDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TChkDataUnit);

protected:
    // pointer to actual data, if data is boolean
    bool* const m_pDataBool;

    // pointer to actual data, if data is 3-state
    AFLib::T3State* const m_pData3State;

    // convert data to integer
    int DataToInt() const;

    // convert integer to data
    void IntToData(int data) const;

public:
    AFLib::T3State m_init;  // default value
    AFLib::T3State m_zero;  // "unneeded" value

    TChkDataUnit(LPCTSTR key, UINT idc, bool& data, bool init, bool zero, UINT idsHelp = 0);
    TChkDataUnit(LPCTSTR key,
        UINT idc,
        AFLib::T3State& data,
        const AFLib::T3State& init,
        const AFLib::T3State& zero,
        UINT idsHelp = 0);

    // get data
    AFLib::T3State GetData() const;

    virtual void Load(LPCTSTR section) const;
    virtual void Save(LPCTSTR section) const;
    virtual bool PeerNeeded(int index) const;
    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
    virtual void Zero() const;
    virtual void Init() const;
    virtual void Write(AFLibIO::TStringBuffer& sb) const;
    virtual void Read(AFLibIO::TStringSpacer& sp) const;
    virtual bool IsUndefined() const;
};
}  // namespace AFLibGui
