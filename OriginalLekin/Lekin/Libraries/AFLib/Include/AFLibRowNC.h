#pragma once

#include "AFLibRow.h"

/////////////////////////////////////////////////////////////////////////////
// CRowNC
// A non-const Row for CRecordsetAF

namespace AFLibDB {
class AFLIB CRowNC : public CRow
{
private:
    typedef CRow super;
    DEFINE_COPY_AND_ASSIGN(CRowNC);

public:
    // use newRow in constructor to add a new row to the recordset
    enum
    {
        newRow = -1
    };

    CRowNC(CRecordsetAF& rs, int index);

    // delete this row
    void Kill();

    // set boolean data
    void SetBool(const TDBField& dbf, bool data);
    void SetBool(LPCTSTR name, bool data);
    void SetBool(int nCol, bool data);

    // set integer or floating data
    void SetInt(const TDBField& dbf, INT64 data, bool bThrowOnBadData = true);
    void SetInt(LPCTSTR name, INT64 data, bool bThrowOnBadData = true);
    void SetInt(int nCol, INT64 data, bool bThrowOnBadData = true);

    // set floating data
    void SetDouble(const TDBField& dbf, double data, bool bThrowOnBadData = true);
    void SetDouble(LPCTSTR name, double data, bool bThrowOnBadData = true);
    void SetDouble(int nCol, double data, bool bThrowOnBadData = true);

    // set time/date data
    void SetDateTime(const TDBField& dbf, const COleDateTime& value);
    void SetDateTime(LPCTSTR name, const COleDateTime& value);
    void SetDateTime(int nCol, const COleDateTime& value);

    // set binary data
    void SetBinary(const TDBField& dbf, const CByteArray& value);
    void SetBinary(LPCTSTR name, const CByteArray& value);
    void SetBinary(int nCol, const CByteArray& value);

    // SetString() sets anything
    void SetString(const TDBField& dbf, LPCTSTR data, bool bThrowOnBadData = true);
    void SetString(LPCTSTR name, LPCTSTR data, bool bThrowOnBadData = true);
    void SetString(int nCol, LPCTSTR data, bool bThrowOnBadData = true);

    // SetStringNull() treats empty string as null
    void SetStringNull(const TDBField& dbf, LPCTSTR data, bool bThrowOnBadData = true);
    void SetStringNull(LPCTSTR name, LPCTSTR data, bool bThrowOnBadData = true);
    void SetStringNull(int nCol, LPCTSTR data, bool bThrowOnBadData = true);

    // only for setting char data -- forces the input string to be Ascii
    void SetStringA(const TDBField& dbf, LPCSTR data);
    void SetStringA(LPCTSTR name, LPCSTR data);
    void SetStringA(int nCol, LPCSTR data);

    // only for setting char data -- forces the input string to be Unicode
    void SetStringW(const TDBField& dbf, LPCWSTR data);
    void SetStringW(LPCTSTR name, LPCWSTR data);
    void SetStringW(int nCol, LPCWSTR data);

    // sets the field to null
    void SetNull(const TDBField& dbf);
    void SetNull(LPCTSTR name);
    void SetNull(int nCol);
};
}  // namespace AFLibDB
