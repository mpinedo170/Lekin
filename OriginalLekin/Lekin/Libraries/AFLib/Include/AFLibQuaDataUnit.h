#pragma once

#include "AFLibDataUnit.h"
#include "AFLibQuantity.h"

/////////////////////////////////////////////////////////////////////////////
// TQuaDataUnit
// Quantity (double or %) edit box.
// No DDV in DDX, since too many parameters are needed.

namespace AFLibGui {
class AFLIB TQuaDataUnit : public TDataUnit
{
private:
    typedef TDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TQuaDataUnit);

public:
    AFLibMath::TQuantity& m_data;  // actual data
    AFLibMath::TQuantity m_init;   // default value
    AFLibMath::TQuantity m_zero;   // "unneeded" value

    TQuaDataUnit(LPCTSTR key,
        UINT idc,
        AFLibMath::TQuantity& data,
        const AFLibMath::TQuantity& init,
        const AFLibMath::TQuantity& zero,
        UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
    virtual void Zero() const;
    virtual void Init() const;
    virtual void Write(AFLibIO::TStringBuffer& sb) const;
    virtual void Read(AFLibIO::TStringSpacer& sp) const;
    virtual bool IsUndefined() const;
};
}  // namespace AFLibGui
