#pragma once

#include "AFLibDefine.h"

class CDataExchange;

namespace AFLibIO {
class TStringParserGeneral;
}

/////////////////////////////////////////////////////////////////////////////
// TQuantity structure
// Number or percentage

namespace AFLibMath {
class AFLIB TQuantity
{
private:
    // check against limits
    bool PrivateCheck(bool bInt, double low, double high, double lowPercent, double highPercent) const;

    // helper for Load(), LoadNoThrow
    void PrivateLoad(LPCTSTR s, bool bThrow);

public:
    double m_value;   // value
    bool m_bPercent;  // is it a percentage amount?

    TQuantity();
    TQuantity(double value, bool bPercent);

    // save in a string
    void Save(AFLibIO::TStringBuffer& sb, int precision = 6, bool bForceSign = false) const;

    // load from a string parser
    void Load(AFLibIO::TStringParserGeneral& sp);

    // load from a string
    void Load(LPCTSTR s)
    {
        PrivateLoad(s, true);
    }

    // load from a string, do not throw exceptions
    void LoadNoThrow(LPCTSTR s);

    // check if % is from 0 to 100
    void CheckBasic() const;

    // fully check data integrity
    void Check(bool bInt, double low, double high, double lowPercent, double highPercent) const;

    // set the value to Nan
    void SetNan();

    // check if the value is Nan
    bool IsNan() const;

    // get value; min & max are ignored if not m_bPercent
    double GetDbl(double aMin, double aMax) const;

    // get value, round to the closest integer
    int GetInt(int aMin, int aMax) const;

    // exchange data with a dialog control
    void DDX(CDataExchange* pDX, UINT idc, bool bUndefOk);

    // verify data (call check and Fail() if necessary)
    void DDV(CDataExchange* pDX, bool bInt, double low, double high, double lowPercent, double highPercent);

    static const TQuantity Qua0;    // "frequently used" quantity -- 0
    static const TQuantity Qua100;  // "frequently used" quantity -- 100%
};
}  // namespace AFLibMath
