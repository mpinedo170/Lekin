#pragma once

#include "AFLibDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TIDataUnit
// Abstract integer-based data unit

namespace AFLibGui {
class AFLIB TIDataUnit : public TDataUnit
{
private:
    typedef TDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TIDataUnit);

public:
    int& m_data;  // actual data
    int m_init;   // default value
    int m_zero;   // "unneeded" value

    TIDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, int nIdc, UINT idsHelp);

    virtual void Load(LPCTSTR section) const;
    virtual void Save(LPCTSTR section) const;
    virtual void Zero() const;
    virtual void Init() const;
    virtual void Write(AFLibIO::TStringBuffer& sb) const;
    virtual void Read(AFLibIO::TStringSpacer& sp) const;
    virtual bool IsUndefined() const;
};
}  // namespace AFLibGui
