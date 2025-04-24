#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TDataUnit
// Abstract data unit

namespace AFLibIO {
class TStringSpacer;
}

namespace AFLibGui {
class AFLIB TDataUnit
{
private:
    DEFINE_COPY_AND_ASSIGN(TDataUnit);

public:
    const CString m_key;  // key for the registry
    const UINT m_idc;     // first dialog ID
    const int m_nIdc;     // number of dialog IDs
    int m_index;          // index in the array
    UINT m_idsHelp;       // IDS of the tool tip

    TDataUnit(LPCTSTR key, UINT idc, int nIdc, UINT idsHelp);
    virtual ~TDataUnit();

    // does this data unit include given dialog control ID?
    bool ContainsIdc(UINT idc) const;

    // read data unit from registry
    virtual void Load(LPCTSTR section) const;

    // write data unit into registry
    virtual void Save(LPCTSTR section) const;

    // used in check box and radio button
    virtual bool PeerNeeded(int index) const;

    // can call Load()?
    virtual bool IsLoadable() const;

    // regular data exchange
    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const = 0;

    // assign the "unneeded" value (m_zero)
    virtual void Zero() const = 0;

    // assign the "default" value (m_init)
    virtual void Init() const = 0;

    // save data in a string
    virtual void Write(AFLibIO::TStringBuffer& sb) const = 0;

    // load data from a string
    virtual void Read(AFLibIO::TStringSpacer& sp) const = 0;

    // for multiple selection -- undefined state
    virtual bool IsUndefined() const = 0;
};
}  // namespace AFLibGui
