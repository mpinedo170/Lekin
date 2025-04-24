#include "StdAfx.h"

#include "AFLibRecordsetAF.h"

#include "AFLibExcBreak.h"
#include "AFLibMathGlobal.h"
#include "AFLibMemoryBulk.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"
#include "AFLibThreadSafeConst.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using AFLibDB::CColumn;
using AFLibMath::IsNan;
using AFLibPrivate::LocalAfxString;
using AFLibThread::CExcBreak;
using namespace AFLib;
using namespace AFLibIO;

int CColumn::GetSData(int nRow, LPVOID pBuffer, SQLINTEGER& nBytes) const
{
    ASSERT(IsCharBin());
    ASSERT(nRow >= 0 && nRow < m_pRs->GetRowCount());
    ASSERT(nBytes >= 0);

    const TCharBin& cb = GetCharBin(nRow);
    nBytes = min(nBytes, SQLINTEGER(cb.m_length));

    memcpy(pBuffer, cb.m_pStr, nBytes);
    return cb.m_length;
}

void CColumn::SetData(int nRow, LPCVOID pNewData)
{
    ASSERT(!IsCharBin());
    ASSERT(nRow >= 0 && nRow < m_pRs->GetRowCount());

    m_isNull[nRow] = false;
    memcpy(GetDataNC(nRow), pNewData, m_fieldSize);
}

void CColumn::SetSData(int nRow, LPCVOID pNewData, int length, bool bEmptyAsNull)
{
    ASSERT(IsCharBin());
    ASSERT(nRow >= 0 && nRow < m_pRs->GetRowCount());
    m_isNull[nRow] = true;

    TCharBin& cbActual = GetCharBinNC(nRow);
    cbActual.m_length = 0;
    cbActual.m_pStr = NULL;

    if (pNewData == NULL || length <= 0)
    {
        if (!bEmptyAsNull) m_isNull[nRow] = false;
        return;
    }

    m_isNull[nRow] = false;
    cbActual.m_length = length;
    cbActual.m_pStr = m_pMb->Allocate(length);
    memcpy(cbActual.m_pStr, pNewData, length);
}

void CColumn::SetBool(int nRow, bool data)
{
    switch (GetType())
    {
        case dftBit:
        case dftTinyInt:
        {
            BYTE d2 = data ? 1 : 0;
            SetData(nRow, &d2);
            return;
        }

        default:
            Throw(IDS_BAD_TYPE_SET, LocalAfxString(IDS_BOOL));
    }
}

void CColumn::SetInt(int nRow, INT64 data, bool bThrowOnBadData)
{
    switch (GetType())
    {
        case dftBit:
            if (data != 0 && data != 1) break;

        case dftTinyInt:
        {
            BYTE d2 = BYTE(data);
            if (d2 != data) break;
            SetData(nRow, &d2);
            return;
        }

        case dftSmallInt:
        {
            short d2 = short(data);
            if (d2 != data) break;
            SetData(nRow, &d2);
            return;
        }

        case dftInt:
        {
            int d2 = int(data);
            if (d2 != data) break;
            SetData(nRow, &d2);
            return;
        }

        case dftBigInt:
            SetData(nRow, &data);
            return;

        case dftReal:
        case dftFloat:
            SetDouble(nRow, double(data), bThrowOnBadData);
            return;

        default:
            Throw(IDS_BAD_TYPE_SET, LocalAfxString(IDS_INT));
    }

    if (bThrowOnBadData)
        Throw(IDS_OUT_OF_RANGE, IntToStr(data));
    else
        SetNull(nRow);
}

void CColumn::SetDouble(int nRow, double data, bool bThrowOnBadData)
{
    if (GetType() != dftReal && GetType() != dftFloat) Throw(IDS_BAD_TYPE_SET, LocalAfxString(IDS_FLOAT));

    if (IsNan(data))
    {
        SetNull(nRow);
        return;
    }

    if (GetType() == dftFloat)
    {
        SetData(nRow, &data);
        return;
    }

    if (data < -FLT_MAX || data > FLT_MAX)
    {
        if (bThrowOnBadData)
            Throw(IDS_OUT_OF_RANGE, DoubleToStr(data));
        else
            SetNull(nRow);
    }
    else
    {
        float d2 = (float)data;
        SetData(nRow, &d2);
        return;
    }
}

void CColumn::SetDateTime(int nRow, const COleDateTime& data)
{
    if (!IsDate()) Throw(IDS_BAD_TYPE_SET, LocalAfxString(IDS_DATE));

    if (data.GetStatus() == COleDateTime::valid && data != odtZero)
        SetData(nRow, &data);
    else
        SetNull(nRow);
}

void CColumn::SetBinary(int nRow, const CByteArray& data)
{
    if (!IsBin()) Throw(IDS_BAD_TYPE_SET, LocalAfxString(IDS_BINARY));

    SetSData(nRow, data.GetData(), data.GetSize(), true);
}

void CColumn::SetString(int nRow, LPCTSTR data, bool bThrowOnBadData)
{  // Output to column data using the right cast.
    switch (GetType())
    {
        case dftBit:
        {
            SAFE_CONST_STR(strNo, _T("0NF"));
            TStringSpacer sp(data);
            if (sp.IsEof()) break;

            TCHAR ch = ToUpper(sp.GetCurrent());
            BYTE bData = strNo.Find(ch) >= 0 ? 1 : 0;
            SetData(nRow, &bData);
            return;
        }

        case dftTinyInt:
        case dftSmallInt:
        case dftInt:
        case dftBigInt:
            try
            {
                TStringSpacer sp(data);
                if (sp.IsEof()) break;

                INT64 data2 = sp.ReadInt64();
                if (!sp.IsEof()) break;

                SetInt(nRow, data2, bThrowOnBadData);
                return;
            }
            catch (CExcBreak*)
            {
                throw;
            }
            catch (CException* pExc)
            {
                pExc->Delete();
                break;
            }

        case dftReal:
        case dftFloat:
            try
            {
                TStringSpacer sp(data);
                if (sp.IsEof()) break;

                double data2 = sp.ReadDouble();
                if (!sp.IsEof()) break;

                SetDouble(nRow, data2, bThrowOnBadData);
                return;
            }
            catch (CExcBreak*)
            {
                throw;
            }
            catch (CException* pExc)
            {
                pExc->Delete();
                break;
            }

        case dftDateTime:
        {
            COleDateTime data2;

            if (!data2.ParseDateTime(data)) break;

            SetDateTime(nRow, data2);
            return;
        }

        case dftAscii:
        case dftUnicode:
#ifdef _UNICODE
            SetStringW(nRow, data);
#else
            SetStringA(nRow, data);
#endif

            return;

        case dftBinary:
        {
            CByteArray buffer;
            CString data2(data);

            data2.Trim();
            data2.MakeUpper();

            LPCTSTR s = data2;
            bool bOdd = data2.GetLength() % 2 == 1;
            int number = 0;

            for (; *s != 0; ++s)
            {
                int digit = 0;

                if (isdigit(*s))
                    digit = *s - _T('0');
                else if (*s >= _T('A') && *s <= _T('F'))
                    digit = *s - _T('A') + 10;
                else
                    break;

                number = number * 16 + digit;
                bOdd = !bOdd;
                if (bOdd) continue;

                buffer.Add(number);
                number = 0;
            }

            SetBinary(nRow, buffer);
            return;
        }
    }

    if (bThrowOnBadData)
        Throw(IDS_STR_CONV_ERR, data);
    else
        SetNull(nRow);
}

void CColumn::SetStringA(int nRow, LPCSTR data)
{
    switch (GetType())
    {
        case dftAscii:
            SetSData(nRow, data, strlen(data), false);
            return;

        case dftUnicode:
        {
            CStringW data2(data);
            SetSData(nRow, LPCWSTR(data2), data2.GetLength() * 2, false);
            return;
        }
    }

    Throw(IDS_STR_AW_ERR, _T(__FUNCTION__));
}

void CColumn::SetStringW(int nRow, LPCWSTR data)
{
    switch (GetType())
    {
        case dftAscii:
        {
            CStringA data2(data);
            SetSData(nRow, LPCSTR(data2), data2.GetLength(), false);
            return;
        }

        case dftUnicode:
            SetSData(nRow, data, wcslen(data) * 2, false);
            return;
    }

    Throw(IDS_STR_AW_ERR, _T(__FUNCTION__));
}

void CColumn::SetNull(int nRow)
{
    ASSERT(nRow >= 0 && nRow < m_pRs->GetRowCount());
    if (IsNull(nRow)) return;

    if (IsCharBin())
        SetSData(nRow, NULL, 0, true);
    else
    {
        LPVOID pData = GetDataNC(nRow);
        memset(pData, 0, m_fieldSize);
        m_isNull[nRow] = true;
    }
}

bool CColumn::GetBool(int nRow) const
{
    switch (GetType())
    {
        case dftBit:
        case dftTinyInt:
            return *reinterpret_cast<LPCBYTE>(GetData(nRow)) != 0;
    }

    Throw(IDS_BAD_TYPE_GET, LocalAfxString(IDS_BOOL));
    return false;
}

int CColumn::GetInt(int nRow) const
{
    switch (GetType())
    {
        case dftBit:
        case dftTinyInt:
            return *reinterpret_cast<LPCBYTE>(GetData(nRow));
        case dftSmallInt:
            return *reinterpret_cast<LPCSHORT>(GetData(nRow));
        case dftInt:
            return *reinterpret_cast<LPCINT>(GetData(nRow));
    }

    Throw(IDS_BAD_TYPE_GET, LocalAfxString(IDS_INT));
    return false;
}

INT64 CColumn::GetInt64(int nRow) const
{
    return GetType() == dftBigInt ? *reinterpret_cast<LPCINT64>(GetData(nRow)) : INT64(GetInt(nRow));
}

double CColumn::GetDouble(int nRow) const
{
    switch (GetType())
    {
        case dftReal:
            return *reinterpret_cast<const float*>(GetData(nRow));

        case dftFloat:
            return *reinterpret_cast<const double*>(GetData(nRow));

        case dftBit:
        case dftTinyInt:
        case dftSmallInt:
        case dftInt:
            return GetInt(nRow);

        case dftBigInt:
            return double(GetInt64(nRow));
    }

    Throw(IDS_BAD_TYPE_GET, LocalAfxString(IDS_FLOAT));
    return false;
}

const COleDateTime& CColumn::GetDateTime(int nRow) const
{
    if (!IsDate()) Throw(IDS_BAD_TYPE_GET, LocalAfxString(IDS_DATE));
    return *reinterpret_cast<const COleDateTime*>(GetData(nRow));
}

void CColumn::GetBinary(int nRow, CByteArray& data) const
{
    if (!IsBin()) Throw(IDS_BAD_TYPE_GET, LocalAfxString(IDS_BINARY));

    const TCharBin& cb = GetCharBin(nRow);
    data.SetSize(cb.m_length);
    memcpy(data.GetData(), cb.m_pStr, cb.m_length);
}

CString CColumn::GetString(int nRow) const
{
    if (IsNull(nRow)) return strEmpty;

    int precision = 20;

    switch (GetType())
    {
        case dftBit:
        case dftTinyInt:
        case dftSmallInt:
        case dftInt:
        case dftBigInt:
            return IntToStr(GetInt64(nRow));

        case dftDateTime:
        {
            const COleDateTime& odt = GetDateTime(nRow);
            return odt.Format();
        }

        case dftReal:
            precision = 12;

        case dftFloat:
            return DoubleToStr(GetDouble(nRow), precision, false);

        case dftAscii:
        case dftUnicode:
#ifdef _UNICODE
            return GetStringW(nRow);
#else
            return GetStringA(nRow);
#endif

        case dftBinary:
        {
            const TCharBin& cb = GetCharBin(nRow);
            if (cb.m_length == 0) return strEmpty;

            SAFE_STATIC_SB(data);
            for (int i = 0; i < cb.m_length; ++i) data.AppendHex(cb.m_pStr[i], 2);
            return LPCTSTR(data);
        }
    }

    ASSERT(false);
    return strEmpty;
}

CStringA CColumn::GetStringA(int nRow) const
{
    if (!IsChar()) Throw(IDS_STR_AW_ERR, _T(__FUNCTION__));

    const TCharBin& cb = GetCharBin(nRow);
    if (cb.m_length == 0) return strEmptyA;

    if (GetType() == dftAscii)
        return CStringA(reinterpret_cast<LPCSTR>(cb.m_pStr), cb.m_length);
    else
        return CStringA(reinterpret_cast<LPCWSTR>(cb.m_pStr), cb.m_length / 2);
}

CStringW CColumn::GetStringW(int nRow) const
{
    if (!IsChar()) Throw(IDS_STR_AW_ERR, _T(__FUNCTION__));

    const TCharBin& cb = GetCharBin(nRow);
    if (cb.m_length == 0) return strEmptyW;

    if (GetType() == dftAscii)
        return CStringW(reinterpret_cast<LPCSTR>(cb.m_pStr), cb.m_length);
    else
        return CStringW(reinterpret_cast<LPCWSTR>(cb.m_pStr), cb.m_length / 2);
}

CString CColumn::GetFmtString(int nRow, const CColumn* pColumn) const
{
    if (IsNull(nRow)) return strNull;
    if (pColumn == NULL) pColumn = this;

    CString s = GetString(nRow);

    if (pColumn->IsDate() || pColumn->IsChar())
        s = QuoteStringSQL(s);
    else if (pColumn->IsBin())
    {
        if (s.IsEmpty()) return strNull;
        s = _T("0x") + s;
    }

    return s;
}

int CColumn::GetRawString(int nRow, LPSTR buffer, int bufLen) const
{
    switch (GetType())
    {
        case dftAscii:
        {
            SQLINTEGER nBytes = bufLen - 1;
            int nBytesTotal = GetSData(nRow, buffer, nBytes);

            buffer[nBytes] = 0;
            return nBytesTotal;
        }

        case dftUnicode:
        {
            const TCharBin& cb = GetCharBin(nRow);
            int len = cb.m_length / 2;

            int lenConv = ::WideCharToMultiByte(_AtlGetConversionACP(), 0,
                reinterpret_cast<LPCWSTR>(cb.m_pStr), min(len, bufLen - 1), buffer, bufLen, NULL, NULL);

            buffer[lenConv] = 0;
            return len;
        }
    }

    Throw(IDS_STR_AW_ERR, _T(__FUNCTION__));
    return 0;
}

int CColumn::GetRawString(int nRow, LPWSTR buffer, int bufLen) const
{
    switch (GetType())
    {
        case dftAscii:
        {
            const TCharBin& cb = GetCharBin(nRow);
            int len = cb.m_length;

            int lenConv = ::MultiByteToWideChar(_AtlGetConversionACP(), 0,
                reinterpret_cast<LPCSTR>(cb.m_pStr), min(len, bufLen - 1), buffer, bufLen);

            buffer[lenConv] = 0;
            return len;
        }

        case dftUnicode:
        {
            SQLINTEGER nBytes = (bufLen - 1) * 2;
            int nBytesTotal = GetSData(nRow, buffer, nBytes);

            buffer[nBytes / 2] = 0;
            return nBytesTotal / 2;
        }
    }

    Throw(IDS_STR_AW_ERR, _T(__FUNCTION__));
    return 0;
}
