#pragma once

/////////////////////////////////////////////////////////////////////////////
// Database global functions and constants

namespace AFLibDB {
class CDatabaseAF;
class TRecordsetParam;

// Supported field types
enum TDBFieldType
{
    dftUnknown,
    dftBit,
    dftTinyInt,
    dftSmallInt,
    dftInt,
    dftBigInt,
    dftReal,
    dftFloat,
    dftDateTime,
    dftAscii,
    dftUnicode,
    dftBinary
};

// convert column type into string
AFLIB CString DBFieldTypeToStr(TDBFieldType dft);

// get raw ODBC type for given field type
AFLIB int DBFieldTypeToODBCType(TDBFieldType dft);

// get raw C type for given field type
AFLIB int DBFieldTypeToCType(TDBFieldType dft);

// get SQL Server BCP type for given field type
AFLIB int DBFieldTypeToBCPType(TDBFieldType dft);

// get the title of the SQL Server Native Client driver
AFLIB const CString& GetSQLServerDriver();

// Build query from parts
AFLIB CString CreateQuery(LPCTSTR table, const CString& fields, const CString& filter, const CString& order);

// Get default recordset parameters
AFLIB const TRecordsetParam& GetDefaultRecordsetParam();

// Set default recordset parameters
AFLIB void SetDefaultRecordsetParam(const TRecordsetParam& param);

// Enable or disable smart rows (value by default)
AFLIB void EnableDefaultSmartRows(bool bEnable);

// Set number of rows fetched in one bulk (value by default)
AFLIB void SetDefaultBulkRows(int nBulk);
}  // namespace AFLibDB
