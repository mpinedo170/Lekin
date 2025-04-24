#include "StdAfx.h"

#include "AFLibQuantity.h"

#include "AFLibExcBreak.h"
#include "AFLibExcMessage.h"
#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringParser.h"
#include "Local.h"

using namespace AFLibIO;
using namespace AFLib;
using namespace AFLibPrivate;
using namespace AFLibMath;

const TQuantity TQuantity::Qua0(0, false);
const TQuantity TQuantity::Qua100(Const100, true);

namespace {
const double EPS = 1e-6;
}

TQuantity::TQuantity()
{
    m_value = 0;
    m_bPercent = false;
}

TQuantity::TQuantity(double value, bool bPercent)
{
    m_value = value;
    m_bPercent = bPercent;
}

void TQuantity::Save(TStringBuffer& sb, int precision, bool bForceSign) const
{
    if (IsNan()) return;

    sb.AppendDouble(m_value, precision, bForceSign);
    if (m_bPercent) sb += chrPercent;
}

void TQuantity::Load(TStringParserGeneral& sp)
{
    m_bPercent = false;
    m_value = sp.ReadDouble();

    if (sp.TestSymbol(strPercent)) m_bPercent = true;
}

void TQuantity::PrivateLoad(LPCTSTR s, bool bThrow)
{
    if (IsEmptyStr(s))
    {
        SetNan();
        return;
    }

    TStringParser sp(s);
    Load(sp);
    if (bThrow) sp.TestEofHard();
}

void TQuantity::LoadNoThrow(LPCTSTR s)
{
    try
    {
        PrivateLoad(s, false);
    }
    catch (AFLibThread::CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }
}

void TQuantity::CheckBasic() const
{
    if (m_bPercent && (m_value < 0 || m_value > Const100)) ThrowMessage(LocalAfxString(IDS_BAD_PERCENT));
}

double TQuantity::GetDbl(double aMin, double aMax) const
{
    ASSERT(!IsNan());
    return m_bPercent ? aMin + (aMax - aMin) * m_value / Const100 : m_value;
}

int TQuantity::GetInt(int aMin, int aMax) const
{
    ASSERT(!IsNan());
    return Rint(GetDbl(aMin, aMax + EPS));
}

void TQuantity::SetNan()
{
    m_value = Nan;
    m_bPercent = false;
}

bool TQuantity::IsNan() const
{
    return AFLibMath::IsNan(m_value);
}

void TQuantity::DDX(CDataExchange* pDX, UINT idc, bool bUndefOk)
{
    if (!pDX->m_bSaveAndValidate)
    {
        TStringBuffer sb;
        Save(sb);
        pDX->m_pDlgWnd->SetDlgItemText(idc, sb);
        return;
    }

    CString s;
    DDX_Text(pDX, idc, s);

    try
    {
        Load(s);
        if (!bUndefOk && IsNan()) LocalThrowMessageExpected(IDS_NUM);
    }
    catch (CException* pExc)
    {
        pExc->ReportError();
        pExc->Delete();
        pDX->Fail();
    }
}

bool TQuantity::PrivateCheck(bool bInt, double low, double high, double lowPercent, double highPercent) const
{
    if (m_bPercent) return m_value >= lowPercent && m_value <= highPercent;
    if (bInt && floor(m_value + EPS) != m_value) return false;
    return m_value >= low && m_value <= high;
}

void TQuantity::Check(bool bInt, double low, double high, double lowPercent, double highPercent) const
{
    if (PrivateCheck(bInt, low, high, lowPercent, highPercent)) return;

    CString sInt;
    if (bInt) sInt = LocalAfxString(IDS_QUA_INT);

    ThrowMessage(LocalAfxString(IDS_QUA_ERR_FORMAT, sInt, DoubleToStr(low), DoubleToStr(high),
        DoubleToStr(lowPercent), DoubleToStr(highPercent)));
}

void TQuantity::DDV(
    CDataExchange* pDX, bool bInt, double low, double high, double lowPercent, double highPercent)
{
    if (!pDX->m_bSaveAndValidate) return;

    try
    {
        Check(bInt, low, high, lowPercent, highPercent);
    }
    catch (CException* pExc)
    {
        pExc->ReportError();
        pExc->Delete();
        pDX->Fail();
    }
}
