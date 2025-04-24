#include "StdAfx.h"

#include "AFLibSpnPrcDataUnit.h"

#include "AFLibGuiGlobal.h"
#include "AFLibMathGlobal.h"

using AFLibGui::TSpnPrcDataUnit;
using namespace AFLibMath;

TSpnPrcDataUnit::TSpnPrcDataUnit(LPCTSTR key,
    UINT idc,
    UINT idcSpin,
    double& data,
    double init,
    double zero,
    double low,
    double high,
    UINT idsHelp) :
    super(key, idc, data, init, zero, low, high, idsHelp),
    m_idcSpin(idcSpin)
{}

void TSpnPrcDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    super::DDX(pDX, bUndefOk);
    if (m_idcSpin == 0) return;

    int low = Rint(max(double(INT_MIN), m_low* Const100));
    int high = Rint(min(double(INT_MAX), m_high* Const100));
    DDX_SpinMinMax(pDX, m_idcSpin, low, high);
}
