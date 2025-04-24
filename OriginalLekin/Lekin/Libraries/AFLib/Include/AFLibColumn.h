#pragma once

#include "AFLibDBGlobal.h"
#include "AFLibDefine.h"
#include "AFLibPointers.h"

namespace AFLib {
class TMemoryBulk;
}

/////////////////////////////////////////////////////////////////////////////
// CColumn
// A Column for CRecordsetAF

namespace AFLibDB {
class CRecordsetAF;
class CDatabaseAF;

class AFLIB CColumn
{
private:
    friend CRecordsetAF;
    DEFINE_COPY_AND_ASSIGN(CColumn);

    // initialize m_nullLength
    void InitNullLength();

protected:
    CRecordsetAF* m_pRs;        // host recordset
    int m_fieldSize;            // field size in m_data
    TDBFieldType m_dft;         // simplified field type
    LPVOID m_data;              // actual data
    CByteArray m_isNull;        // null indicators
    AFLib::TMemoryBulk* m_pMb;  // memory for CharBin columns

    CString m_fieldName;  // field name
    int m_rawODBCType;    // original ODBC field type (SQL_ constants)
    bool m_bNullable;     // is nullable?
    int m_origColSize;    // column size as defined in SQLDescribeCol

    int m_rawFieldSize;   // raw field size in bytes
    int m_realFieldSize;  // min # of bytes to avoid truncating;
                          //   filled when the recordset is fetched

    LPBYTE m_pTempData;     // temporary data buffer for binding data
    SQLLEN* m_pTempLen;     // temporary buffer for length/null indicators
    int m_nTempBulk;        // number of elements in the temprary buffer
    bool m_bBcpTempBuffer;  // temporary buffer is for BCP
    SQLLEN m_nullLength;    // length indicator for a null value

    // structure for keeping character or binary data
    struct TCharBin
    {
        int m_length;   // length in BYTES
        LPBYTE m_pStr;  // actual data
    };

    // protected constructor: only Recordset can create columns
    explicit CColumn(CRecordsetAF* pRs);

    // throw formatted message
    void Throw(UINT ids, LPCTSTR s4) const;

    // reallocate memory (grow data storage)
    virtual void Realloc(int oldCapacity);

    // delete rows
    virtual void DeleteRow(int nRow, int count);

    // delete all rows
    void DeleteAllRows();

    // memory is allocated in the current thread's heap
    bool IsLocalMem();

    // get Char or Binary data into the buffer
    int GetSData(int nRow, LPVOID pBuffer, SQLINTEGER& nBytes) const;

    // binary copy, pNewData has to be in the right format; fails for strings!
    void SetData(int nRow, LPCVOID pNewData);

    // SetData() for Char or Binary types
    void SetSData(int nRow, LPCVOID pNewData, int length, bool bEmptyAsNull);

    //--- Temporary buffer functions ---

    // create buffer for data binding
    virtual void CreateTempBuffer();

    // destroy temporary buffer
    virtual void DestroyTempBuffer();

    // fill the temporary buffer with zeros
    virtual void CleanTempBuffer();

    // copy data from the temporary buffer to the main data storage area
    virtual void UnloadTempBuffer(int nRowsFetched);

    // fill in the Nulls (helper for UnloadTempBuffer())
    virtual void UnloadTempBufferNulls(int oldNRows, int nRowsFetched);

    // copy one element from the temporary buffer (helper for UnloadTempBuffer())
    virtual void UnloadTempBufferElement(int nRow, LPCBYTE pData, SQLLEN length);

    // for UnloadTempBuffer() and LoadTempBuffer(): can do simple memcpy to load/unload the entire buffer?
    virtual bool CanCopyTempBufferDirect();

    //--- Temporary buffer functions for BCP or Bulk Insert ---

    // create buffer for BCP data binding (NOT for ODBC Bulk Insert)
    virtual void CreateBcpTempBuffer();

    // fill the temporary buffer with nulls (for BulkInsert)
    virtual void NullifyTempBuffer();

    // copy buffer from source column
    virtual void LoadTempBuffer(const CColumn& colSrc, int rStart, int count);

    // copy only one element
    virtual void LoadTempBufferElement(const CColumn& colSrc, int rSrc, LPVOID pData, SQLLEN* pLen);

    // copy date (helper for LoadTempBufferElement())
    void LoadTempBufferDate(const CColumn& colSrc, int rSrc, LPVOID pData, SQLLEN* pLen);

    //--- End of temporary buffer functions ---

    // get place in the data buffer where given row is
    LPCVOID GetData(int nRow) const
    {
        return static_cast<LPCSTR>(m_data) + m_fieldSize * nRow;
    }

    // GetData(), non-const
    LPVOID GetDataNC(int nRow)
    {
        return const_cast<LPVOID>(GetData(nRow));
    }

    // GetData() for character and binary data
    const TCharBin& GetCharBin(int nRow) const
    {
        return *static_cast<const TCharBin*>(GetData(nRow));
    }

    // GetCharBin(), non-const
    TCharBin& GetCharBinNC(int nRow)
    {
        return *const_cast<TCharBin*>(&GetCharBin(nRow));
    }

public:
    virtual ~CColumn();

    // key for sorting
    const CString& GetGLKey() const
    {
        return m_fieldName;
    }

    // copy from given column (structure only or structure and data)
    virtual void Copy(const CColumn& column);

    // once m_origRawType and m_origColSize have been initialized, call this function
    virtual void ResetFormat();

    // reset types and m_rawFieldSize for BCP or ODBC Bulk Insert
    virtual void ResetForBulkInsert(const CColumn& colSrc);

    // SetBool() works with bit and tiny-int fields
    virtual void SetBool(int nRow, bool data);

    // SetInt() works with integer and floating fields; checks bounds
    virtual void SetInt(int nRow, INT64 data, bool bThrowOnBadData);

    // SetDouble() works only with floating fields; checks bounds for 'float' type
    virtual void SetDouble(int nRow, double data, bool bThrowOnBadData);

    // SetDateTime() works with date/time fields
    virtual void SetDateTime(int nRow, const COleDateTime& data);

    // SetBinary() works with binary fields
    virtual void SetBinary(int nRow, const CByteArray& data);

    // SetString() sets anything
    virtual void SetString(int nRow, LPCTSTR data, bool bThrowOnBadData);

    // only for setting char data -- forces the input string to be Ascii
    virtual void SetStringA(int nRow, LPCSTR data);

    // only for setting char data -- forces the input string to be Unicode
    virtual void SetStringW(int nRow, LPCWSTR data);

    // sets the null indicator
    virtual void SetNull(int nRow);

    // is this a character column?
    bool IsChar() const
    {
        return m_dft == dftAscii || m_dft == dftUnicode;
    }

    // is this a binary column?
    bool IsBin() const
    {
        return m_dft == dftBinary;
    }

    // is this a Unicode column?
    bool IsUnicode() const
    {
        return m_dft == dftUnicode;
    }

    // is this a date column?
    bool IsDate() const
    {
        return m_dft == dftDateTime;
    }

    // is this a "complicated" column?
    bool IsCharBin() const
    {
        return IsChar() || IsBin();
    }

    // get the type of the column
    TDBFieldType GetType() const
    {
        return m_dft;
    }

    // get the name of the column
    const CString& GetName() const
    {
        return m_fieldName;
    }

    // get the original type of the column
    int GetRawODBCType() const
    {
        return m_rawODBCType;
    }

    // get max raw field size (used in Bulk Insert)
    int GetMaxRawFieldSize() const;

    // get max necessary field size (for a requery)
    int GetMaxBoundFieldSize() const;

    // is nullable?
    bool IsNullable() const
    {
        return m_bNullable;
    }

    // is the data null?
    bool IsNull(int nRow) const
    {
        return m_isNull[nRow] != 0;
    }

    // get boolean data
    virtual bool GetBool(int nRow) const;

    // get integer data (no float or 64-bit or conversion from string)
    virtual int GetInt(int nRow) const;

    // get 64-bit integer data (no conversion from string)
    virtual INT64 GetInt64(int nRow) const;

    // get floating point data (no conversion from string)
    virtual double GetDouble(int nRow) const;

    // get date/time data
    virtual const COleDateTime& GetDateTime(int nRow) const;

    // get binary data
    virtual void GetBinary(int nRow, CByteArray& data) const;

    // get any data
    virtual CString GetString(int nRow) const;

    // get ascii string (only for char fields)
    virtual CStringA GetStringA(int nRow) const;

    // get unicode string (only for char fields)
    virtual CStringW GetStringW(int nRow) const;

    // get ascii string directly into buffer, without allocating a CString
    virtual int GetRawString(int nRow, LPSTR buffer, int bufLen) const;

    // get unicode string directly into buffer, without allocating a CString
    virtual int GetRawString(int nRow, LPWSTR buffer, int bufLen) const;

    // get formatted field, or null; formats according to pColumn
    virtual CString GetFmtString(int nRow, const CColumn* pColumn = NULL) const;
};
}  // namespace AFLibDB
