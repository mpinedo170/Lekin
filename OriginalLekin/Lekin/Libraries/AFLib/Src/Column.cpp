#include "StdAfx.h"

#include "AFLibRecordsetAF.h"

#include "AFLibMathGlobal.h"
#include "AFLibMemoryBulk.h"
#include "AFLibThread.h"
#include "Local.h"
#undef DBCHAR
#include <odbcss.h>  // SQL Server functions; includes WRONG definition of LPCBYTE!

using namespace AFLib;
using namespace AFLibMath;
using namespace AFLibDB;
using namespace AFLibThread;
using AFLibPrivate::LocalAfxString;

CColumn::CColumn(CRecordsetAF* pRs) :
    m_pRs(pRs),
    m_dft(dftUnknown),
    m_data(NULL),
    m_rawODBCType(SQL_UNKNOWN_TYPE)
{
    m_fieldSize = m_origColSize = m_rawFieldSize = m_realFieldSize = m_nTempBulk = 0;
    m_bBcpTempBuffer = false;
    m_nullLength = 0;

    m_pMb = NULL;
    m_pTempData = NULL;
    m_pTempLen = NULL;
}

CColumn::~CColumn()
{
    DestroyTempBuffer();
    if (m_data != NULL) DeleteAllRows();
    delete m_pMb;
}

void CColumn::Throw(UINT ids, LPCTSTR s4) const
{
    ThrowMessage(LocalAfxString(ids, DBFieldTypeToStr(GetType()), GetName(), m_pRs->GetQuery(), s4));
}

bool CColumn::IsLocalMem()
{
    return m_pRs->GetRecordsetParam().m_bLocalMem;
}

void CColumn::Realloc(int oldCapacity)
{
    int oldNRows = m_isNull.GetSize();
    int newNRows = m_pRs->GetRowCount();

    if (newNRows > oldNRows)
        m_isNull.InsertAt(oldNRows, 1, newNRows - oldNRows);
    else if (newNRows < oldNRows)
        m_isNull.SetSize(newNRows);

    int newCapacity = m_pRs->GetCapacity();
    if (newCapacity == oldCapacity) return;

    LPBYTE newData = reinterpret_cast<LPBYTE>(CThread::AllocMem(newCapacity * m_fieldSize, IsLocalMem()));

    if (oldCapacity > 0)
    {
        memcpy(newData, m_data, oldCapacity * m_fieldSize);
        CThread::FreeMem(m_data);
        m_data = NULL;
    }

    m_data = newData;
    memset(GetDataNC(oldCapacity), 0, (newCapacity - oldCapacity) * m_fieldSize);
}

void CColumn::DeleteRow(int nRow, int count)
{
    int oldNRows = m_isNull.GetSize();
    m_isNull.RemoveAt(nRow, count);

    int moveCount = oldNRows - nRow - count;
    if (moveCount > 0) memmove(GetDataNC(nRow), GetData(nRow + count), moveCount * m_fieldSize);

    memset(GetDataNC(oldNRows - count), 0, count * m_fieldSize);
}

void CColumn::DeleteAllRows()
{
    DeleteRow(0, m_pRs->GetRowCount());
    CThread::FreeMem(m_data);
    m_data = NULL;
    if (m_pMb != NULL) m_pMb->Clear();
}

void CColumn::InitNullLength()
{
    m_nullLength = SQL_NULL_DATA;

    // if column is not nullable, nulls will be replaced by zeros
    if (!IsNullable())
    {
        if (IsBin())
            m_nullLength = 1;
        else if (IsChar())
            m_nullLength = 0;
        else
            m_nullLength = m_rawFieldSize;
    }
}

void CColumn::CreateTempBuffer()
{
    ASSERT(m_pTempData == NULL && m_pTempLen == NULL);

    m_nTempBulk = m_pRs->GetRecordsetParam().m_nBulk;
    m_pTempData = reinterpret_cast<LPBYTE>(CThread::AllocMem(m_nTempBulk * m_rawFieldSize, IsLocalMem()));
    m_pTempLen = reinterpret_cast<SQLLEN*>(CThread::AllocMem(m_nTempBulk * sizeof(SQLLEN), IsLocalMem()));
    m_bBcpTempBuffer = false;
    InitNullLength();
}

void CColumn::CreateBcpTempBuffer()
{
    ASSERT(m_pTempData == NULL && m_pTempLen == NULL);

    m_nTempBulk = 1;
    LPBYTE pData = reinterpret_cast<LPBYTE>(CThread::AllocMem(m_rawFieldSize + sizeof(SQLLEN), IsLocalMem()));

    m_pTempLen = reinterpret_cast<SQLLEN*>(pData);
    m_pTempData = pData + sizeof(SQLLEN);
    m_bBcpTempBuffer = true;
    InitNullLength();
}

void CColumn::DestroyTempBuffer()
{
    if (m_bBcpTempBuffer)
        CThread::FreeMem(m_pTempLen);
    else
    {
        CThread::FreeMem(m_pTempData);
        CThread::FreeMem(m_pTempLen);
    }

    m_pTempData = NULL;
    m_pTempLen = NULL;
    m_nTempBulk = 0;
    m_bBcpTempBuffer = false;
    m_nullLength = SQL_NULL_DATA;
}

void CColumn::CleanTempBuffer()
{
    memset(m_pTempData, 0, m_nTempBulk * m_rawFieldSize);
    memset(m_pTempLen, 0, m_nTempBulk * sizeof(SQLLEN));
}

void CColumn::UnloadTempBufferNulls(int oldNRows, int nRowsFetched)
{
    for (int i = 0; i < nRowsFetched; ++i)
    {
        SQLLEN indicator = m_pTempLen[i];
        if (indicator > 0 || indicator == SQL_NO_TOTAL)
        {  // SQL_NULL_DATA = -1, so m_isNull will stay 1
            m_isNull[oldNRows + i] = 0;
        }
    }
}

void CColumn::UnloadTempBufferElement(int nRow, LPCBYTE pData, SQLLEN length)
{
    ASSERT(!CanCopyTempBufferDirect());

    m_realFieldSize = max(m_realFieldSize, int(length));

    // check if data is too long
    if (length > m_rawFieldSize || length == SQL_NO_TOTAL) length = m_rawFieldSize;

    switch (GetType())
    {
        case dftAscii:
            while (length > 0 && pData[length - 1] == 0) --length;
            SetSData(nRow, pData, length, false);
            break;

        case dftUnicode:
            while (length >= 2 && *reinterpret_cast<LPCWSTR>(pData + length - 2) == 0) length -= 2;
            SetSData(nRow, pData, length, false);
            break;

        case dftBinary:
            SetSData(nRow, pData, length, true);
            break;

        case dftDateTime:
        {
            const TIMESTAMP_STRUCT& ts = *reinterpret_cast<const TIMESTAMP_STRUCT*>(pData);
            COleDateTime odt(ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
            SetDateTime(nRow, odt);
            break;
        }

        default:
            ASSERT(false);
    }
}

bool CColumn::CanCopyTempBufferDirect()
{
    return !IsCharBin() && !IsDate();
}

void CColumn::UnloadTempBuffer(int nRowsFetched)
{
    int oldNRows = m_isNull.GetSize() - nRowsFetched;

    UnloadTempBufferNulls(oldNRows, nRowsFetched);
    if (CanCopyTempBufferDirect())
    {
        ASSERT(m_rawFieldSize == m_fieldSize);
        memcpy(GetDataNC(oldNRows), m_pTempData, nRowsFetched * m_fieldSize);
        return;
    }

    if (IsCharBin())
    {
        int len = 0;
        for (int i = 0; i < nRowsFetched; ++i) len += max(int(m_pTempLen[i]), 0);
        m_pMb->AllocateBlock(len);
    }

    for (int i = 0; i < nRowsFetched; ++i)
        if (!IsNull(oldNRows + i))
            UnloadTempBufferElement(oldNRows + i, m_pTempData + i * m_rawFieldSize, m_pTempLen[i]);
}

namespace {
const TIMESTAMP_STRUCT nullDate = {1900, 1, 1};
const float secInDay300 = 24 * 60 * 60 * 300;
const COleDateTime odtSQLZero(1900, 1, 1, 0, 0, 0);
}  // namespace

void CColumn::NullifyTempBuffer()
{
    memset(m_pTempData, 0, m_nTempBulk * m_rawFieldSize);

    if (IsDate() && !IsNullable() && !m_bBcpTempBuffer)
        for (int r = 0; r < m_nTempBulk; ++r)
            memcpy(m_pTempData + r * m_rawFieldSize, &nullDate, m_rawFieldSize);

    for (int r = 0; r < m_nTempBulk; ++r) m_pTempLen[r] = m_nullLength;
}

void CColumn::LoadTempBufferDate(const CColumn& colSrc, int rSrc, LPVOID pData, SQLLEN* pLen)
{
    *pLen = m_rawFieldSize;

    if (colSrc.IsNull(rSrc))
    {
        if (IsNullable() || m_bBcpTempBuffer)
            *pLen = m_nullLength;
        else
            memcpy(pData, &nullDate, m_rawFieldSize);
        return;
    }

    const COleDateTime& odt = colSrc.GetDateTime(rSrc);

    if (m_bBcpTempBuffer)
    {
        DBDATETIME& dstDate = *reinterpret_cast<DBDATETIME*>(pData);
        double days = (odt - odtSQLZero).m_span;
        dstDate.dtdays = int(floor(days));
        dstDate.dttime = Rint((days - dstDate.dtdays) * secInDay300);
    }
    else
    {
        TIMESTAMP_STRUCT& dstDate = *reinterpret_cast<TIMESTAMP_STRUCT*>(pData);
        dstDate.year = odt.GetYear();
        dstDate.month = odt.GetMonth();
        dstDate.day = odt.GetDay();
        dstDate.hour = odt.GetHour();
        dstDate.minute = odt.GetMinute();
        dstDate.second = odt.GetSecond();
    }
}

void CColumn::LoadTempBufferElement(const CColumn& colSrc, int rSrc, LPVOID pData, SQLLEN* pLen)
{
    bool bThrowOnBadData = m_pRs->GetRecordsetParam().m_bThrowOnBadData;
    memset(pData, 0, m_rawFieldSize);

    // process date
    if (IsDate())
    {  // source column is either Char or Date
        ASSERT(colSrc.IsChar() || colSrc.IsDate());

        if (colSrc.IsChar())
        {
            CString sData = colSrc.GetString(rSrc);
            SetString(0, sData, bThrowOnBadData);
            LoadTempBufferDate(*this, 0, pData, pLen);
        }
        else
            LoadTempBufferDate(colSrc, rSrc, pData, pLen);
        return;
    }

    // process null in colSrc
    if (colSrc.IsNull(rSrc))
    {
        *pLen = m_nullLength;
        return;
    }

    switch (GetType())
    {
        case dftBinary:
        {  // source column is either Char or Binary
            ASSERT(colSrc.IsCharBin());
            *pLen = m_rawFieldSize;

            // binary source -- direct copy
            if (colSrc.IsBin())
            {
                colSrc.GetSData(rSrc, pData, *pLen);
                return;
            }

            // character source: use SetString() to convert
            CString sData = colSrc.GetString(rSrc);
            SetString(0, sData, bThrowOnBadData);

            if (IsNull(0))
                *pLen = m_nullLength;
            else
                GetSData(0, pData, *pLen);
            return;
        }

        case dftUnicode:
        {  // if colSrc is not Unicode, destination column was
            // converted to Ascii in ResetForBulkInsert(), so we won't be here
            ASSERT(colSrc.IsUnicode());

            *pLen = m_rawFieldSize;
            colSrc.GetSData(rSrc, pData, *pLen);
            return;
        }

        case dftAscii:
        {  // here, source may be anything!
            if (colSrc.GetType() == dftAscii)
            {  // direct copy
                *pLen = m_rawFieldSize;
                colSrc.GetSData(rSrc, pData, *pLen);
            }
            else
            {  // convert to CString, then CStringA
                CStringA sData(colSrc.GetString(rSrc));
                int length = min(sData.GetLength(), m_rawFieldSize);
                memcpy(pData, LPCSTR(sData), length);
                *pLen = length;
            }
            return;
        }
    }

    // if we came here, destination column is simple: int or float
    *pLen = m_rawFieldSize;

    if (colSrc.IsChar())
    {
        CString sData = colSrc.GetString(rSrc);

        // SetString() will try to convert string into any type.
        // If can't convert, will throw exception or generate null.
        SetString(0, sData, bThrowOnBadData);
    }
    else
    {
        switch (GetType())
        {
            case dftBit:
            case dftTinyInt:
            case dftSmallInt:
            case dftInt:
            case dftBigInt:
                SetInt(0, colSrc.GetInt64(rSrc), bThrowOnBadData);
                break;

            case dftReal:
            case dftFloat:
                SetDouble(0, colSrc.GetDouble(rSrc), bThrowOnBadData);
                break;
        }
    }

    // SetString(), SetInt() and SetDouble() may have set the field to null!
    if (IsNull(0))
        *pLen = m_nullLength;
    else
        memcpy(pData, GetData(0), m_rawFieldSize);
}

void CColumn::LoadTempBuffer(const CColumn& colSrc, int rStart, int count)
{
    if (CanCopyTempBufferDirect() && colSrc.GetType() == GetType())
    {
        memcpy(m_pTempData, colSrc.GetData(rStart), count * m_rawFieldSize);
        for (int i = 0; i < count; ++i)
            m_pTempLen[i] = colSrc.IsNull(rStart + i) ? m_nullLength : m_rawFieldSize;
        return;
    }

    for (int i = 0; i < count; ++i)
        LoadTempBufferElement(colSrc, rStart + i, m_pTempData + i * m_rawFieldSize, m_pTempLen + i);
}

void CColumn::Copy(const CColumn& column)
{
    m_fieldName = column.m_fieldName;
    m_rawODBCType = column.m_rawODBCType;
    m_origColSize = column.m_origColSize;
    ResetFormat();

    int rowCount = m_pRs->GetRowCount();
    if (rowCount == 0) return;

    m_isNull.Copy(column.m_isNull);
    if (!IsCharBin())
    {
        memcpy(m_data, column.m_data, rowCount * m_fieldSize);
        return;
    }

    int len = 0;
    for (int r = 0; r < rowCount; ++r) len += column.GetCharBin(r).m_length;
    m_pMb->AllocateBlock(len);

    for (int r = 0; r < rowCount; ++r)
        if (!IsNull(r))
        {
            const TCharBin& cb = column.GetCharBin(r);
            SetSData(r, cb.m_pStr, cb.m_length, false);
        }
}

void CColumn::ResetFormat()
{  // Allocate memory of correct type for current column.
    // Numeric and Money types are automatically converted into doubles;
    //   dates -- into COleDateTime
    ASSERT(m_dft == dftUnknown);
    int maxBuffer = m_pRs->GetRecordsetParam().m_maxCharBuffer;

    switch (m_rawODBCType)
    {
        case SQL_BIT:
            m_dft = dftBit;
            m_rawFieldSize = m_fieldSize = 1;
            m_bNullable = false;
            break;

        case SQL_TINYINT:
            m_dft = dftTinyInt;
            m_rawFieldSize = m_fieldSize = 1;
            break;

        case SQL_SMALLINT:
            m_dft = dftSmallInt;
            m_rawFieldSize = m_fieldSize = sizeof(short);
            break;

        case SQL_INTEGER:
            m_dft = dftInt;
            m_rawFieldSize = m_fieldSize = sizeof(int);
            break;

        case SQL_BIGINT:
            m_dft = dftBigInt;
            m_rawFieldSize = m_fieldSize = sizeof(INT64);
            break;

        case SQL_REAL:
            m_dft = dftReal;
            m_rawFieldSize = m_fieldSize = sizeof(float);
            break;

        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_NUMERIC:
        case SQL_DECIMAL:
            m_dft = dftFloat;
            m_rawFieldSize = m_fieldSize = sizeof(double);
            break;

        case SQL_DATE:
        case SQL_TIME:
        case SQL_TIMESTAMP:
            m_dft = dftDateTime;
            m_fieldSize = sizeof(COleDateTime);
            m_rawFieldSize = sizeof(TIMESTAMP_STRUCT);
            break;

        case SQL_LONGVARCHAR:
            maxBuffer = m_pRs->GetRecordsetParam().m_maxLongCharBuffer;
        case SQL_CHAR:
        case SQL_VARCHAR:
            m_dft = dftAscii;
            m_fieldSize = sizeof(TCharBin);
            m_rawFieldSize =
                m_origColSize == 0 || m_origColSize >= maxBuffer - 1 ? maxBuffer : m_origColSize + 1;
            break;

        case SQL_WLONGVARCHAR:
            maxBuffer = m_pRs->GetRecordsetParam().m_maxLongCharBuffer;
        case SQL_WCHAR:
        case SQL_WVARCHAR:
            m_dft = dftUnicode;
            m_fieldSize = sizeof(TCharBin);
            m_rawFieldSize =
                m_origColSize == 0 || m_origColSize >= maxBuffer / 2 - 1 ? maxBuffer : m_origColSize * 2 + 2;
            break;

        case SQL_LONGVARBINARY:
            maxBuffer = m_pRs->GetRecordsetParam().m_maxLongCharBuffer;
        case SQL_BINARY:
        case SQL_VARBINARY:
            m_dft = dftBinary;
            m_fieldSize = sizeof(TCharBin);
            m_rawFieldSize = m_origColSize == 0 || m_origColSize >= maxBuffer ? maxBuffer : m_origColSize;
            break;

        default:
            TRACE(_T("Unsupported data type encountered: ") + IntToStr(m_rawODBCType) +
                  _T(".  Converting to Unicode."));

            m_dft = dftUnicode;
            m_fieldSize = sizeof(TCharBin);
            m_rawFieldSize = maxBuffer;
    }

    delete m_pMb;
    m_pMb = NULL;

    if (IsCharBin()) m_pMb = new TMemoryBulk(m_pRs->GetRecordsetParam().m_bLocalMem);
    if (m_pRs->GetCapacity() > 0) Realloc(0);
}

void CColumn::ResetForBulkInsert(const CColumn& colSrc)
{
    if (IsDate())
    {  // source may be either char or date
        if (!colSrc.IsDate() && !colSrc.IsChar()) Throw(IDS_BAD_TYPE_SET, DBFieldTypeToStr(colSrc.GetType()));

        if (m_bBcpTempBuffer) m_rawFieldSize = sizeof(DBDATETIME);
    }

    if (!IsCharBin()) return;

    int length = colSrc.IsCharBin() ? colSrc.GetMaxRawFieldSize() : 24;

    if (!IsUnicode() || !colSrc.IsUnicode())
    {
        switch (GetType())
        {
            case dftBinary:
                // Source may be either char or binary.
                // Char to binary: 2 chars per one byte
                switch (colSrc.GetType())
                {
                    case dftUnicode:
                        length = (length + 2) / 4;
                        break;
                    case dftAscii:
                        length = (length + 1) / 2;
                        break;
                    case dftBinary:
                        break;
                    default:
                        Throw(IDS_BAD_TYPE_SET, DBFieldTypeToStr(colSrc.GetType()));
                }
                break;

            case dftUnicode:
                // Unicode will be sent as Ascii, unless source column is also Unicode.
                m_dft = dftAscii;
                m_rawFieldSize /= 2;

            case dftAscii:
                switch (colSrc.GetType())
                {
                    case dftUnicode:
                        length /= 2;
                        break;
                    case dftBinary:
                        length *= 2;
                        break;
                }
        }
    }

    m_rawFieldSize = min(length, m_rawFieldSize);
}

int CColumn::GetMaxRawFieldSize() const
{
    if (!IsCharBin()) return m_rawFieldSize;

    int maxLen = 1;

    for (int r = m_pRs->GetRowCount(); --r >= 0;) maxLen = max(maxLen, GetCharBin(r).m_length);

    if (IsUnicode() && maxLen % 2 == 1) ++maxLen;

    return maxLen;
}

int CColumn::GetMaxBoundFieldSize() const
{
    return IsCharBin() ? m_realFieldSize : m_rawFieldSize;
}
