#pragma once

#include "AFLibDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TDDataUnit
// Abstract double-based data unit

namespace AFLibGui {
class AFLIB TDDataUnit : public TDataUnit
{
private:
    typedef TDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TDDataUnit);

public:
    double& m_data;  // actual data
    double m_init;   // default value
    double m_zero;   // "unneeded" value

    TDDataUnit(LPCTSTR key, UINT idc, double& data, double init, double zero, UINT idsHelp);

    virtual void Zero() const;
    virtual void Init() const;
    virtual void Write(AFLibIO::TStringBuffer& sb) const;
    virtual void Read(AFLibIO::TStringSpacer& sp) const;
    virtual bool IsUndefined() const;
};
}  // namespace AFLibGui
