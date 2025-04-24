#include "StdAfx.h"

#include "AFLibSpinInfinity.h"

#include "AFLibGlobal.h"

using namespace AFLib;
using AFLibGui::CSpinInfinity;

#ifdef _UNICODE
const WCHAR CSpinInfinity::strInfinity[] = {chrInfinityW, 0};
#else
const CHAR CSpinInfinity::strInfinity[] = "oxo";
#endif

IMPLEMENT_DYNAMIC(CSpinInfinity, super)

CSpinInfinity::CSpinInfinity()
{}

BEGIN_MESSAGE_MAP(CSpinInfinity, super)
ON_NOTIFY_REFLECT(UDN_DELTAPOS, OnUdnDeltaPos)
END_MESSAGE_MAP()

void CSpinInfinity::OnUdnDeltaPos(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    *pResult = 0;

    int lower = 0;
    int upper = 0;
    GetRange(lower, upper);

    int pos = pNMUpDown->iPos + pNMUpDown->iDelta;
    pos = min(max(pos, lower), upper);

    CWnd* pWnd = GetBuddy();
    if (pWnd == NULL) return;

    pWnd->SetWindowText(pos < upper ? IntToStr(pos) : strInfinity);
}
