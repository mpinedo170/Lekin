#pragma once

#include "AFLibDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TSDataUnit
// Abstract string-based data unit

namespace AFLibGui {
class AFLIB TSDataUnit : public TDataUnit
{
private:
    typedef TDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TSDataUnit);

public:
    CString& m_data;  // actual data
    CString m_init;   // default value
    CString m_zero;   // "unneeded" value

    TSDataUnit(LPCTSTR key, UINT idc, CString& data, LPCTSTR init, LPCTSTR zero, UINT idsHelp);

    virtual void Load(LPCTSTR section) const;
    virtual void Save(LPCTSTR section) const;
    virtual void Zero() const;
    virtual void Init() const;
    virtual void Write(AFLibIO::TStringBuffer& sb) const;
    virtual void Read(AFLibIO::TStringSpacer& sp) const;
    virtual bool IsUndefined() const;
};
}  // namespace AFLibGui
