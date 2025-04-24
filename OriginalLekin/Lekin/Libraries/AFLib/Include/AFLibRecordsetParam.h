#include "AFLib3State.h"

/////////////////////////////////////////////////////////////////////////////
// TRecordsetParam
// Recordset parameters gathered in one class to simplify recordset construction

namespace AFLibDB {
class AFLIB TRecordsetParam
{
public:
    AFLib::T3State m_bSmartRows;       // "smart rows" shift when rows are being deleted from recordset
    AFLib::T3State m_bThrowOnBadData;  // when Bulk Inserting, throw exception on bad data
    AFLib::T3State m_bLocalMem;        // use thread-local memory where possible
    int m_nBulk;                       // number of rows fetched in one bulk
    int m_maxCharBuffer;               // max buffer size for char & binary fields
    int m_maxLongCharBuffer;           // max buffer size for long char & binary fields

    TRecordsetParam();

    // set default values
    void Default();

    // change "this" according to given parameters
    void Change(const TRecordsetParam& param);
};
}  // namespace AFLibDB
