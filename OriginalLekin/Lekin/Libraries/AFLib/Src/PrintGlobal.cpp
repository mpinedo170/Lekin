#include "StdAfx.h"

#include "AFLibGuiGlobal.h"

#include "AFLibGlobal.h"

using namespace AFLib;
using namespace AFLibGui;

namespace AFLibGui {
bool IsDevModeUnicode(LPCVOID pDevMode)
{
    const DEVMODEA* pDevModeA = reinterpret_cast<const DEVMODEA*>(pDevMode);

    if (pDevModeA->dmSize <= 0 || pDevModeA->dmSize >= sizeof(DEVMODEW)) return true;

    const DEVMODEW* pDevModeW = reinterpret_cast<const DEVMODEW*>(pDevMode);
    return pDevModeW->dmSize > sizeof(DEVMODEA) && pDevModeW->dmSize <= sizeof(DEVMODEW);
}

bool IsDevNamesUnicode(LPDEVNAMES pDevNames)
{
    return pDevNames->wDriverOffset < sizeof(DEVNAMES) || pDevNames->wDeviceOffset < sizeof(DEVNAMES) ||
           pDevNames->wOutputOffset < sizeof(DEVNAMES);
}
}  // namespace AFLibGui

namespace {
template <class DM_SRC, class DM_DST, class STR, class CH>
HGLOBAL PrivateDevMode(HGLOBAL hDevMode, const DM_SRC&, const DM_DST&, const STR&, CH)
{
    if (hDevMode == NULL) return NULL;

    LPCVOID pData = GlobalLock(hDevMode);
    bool bUnicode = IsDevModeUnicode(pData);
    HGLOBAL hDevModeNew = NULL;

    if (bUnicode == (sizeof(CH) > 1))
    {
        const DM_DST* pDevMode = reinterpret_cast<const DM_DST*>(pData);
        hDevModeNew = AllocBlock(pDevMode, pDevMode->dmSize + pDevMode->dmDriverExtra);
    }
    else
    {
        const DM_SRC* pDevMode = reinterpret_cast<const DM_SRC*>(pData);
        int sizeTotal = pDevMode->dmSize + pDevMode->dmDriverExtra;
        int sizeStrucNew = sizeof(DM_DST) + pDevMode->dmSize - sizeof(DM_SRC);

        hDevModeNew = AllocBlock(NULL, sizeStrucNew + pDevMode->dmDriverExtra);

        DM_DST* pDevModeNew = reinterpret_cast<DM_DST*>(GlobalLock(hDevModeNew));
        STR deviceName(pDevMode->dmDeviceName);
        STR formName(pDevMode->dmFormName);

        memcpy(pDevModeNew->dmDeviceName, (const CH*)deviceName,
            min(deviceName.GetLength(), CCHDEVICENAME - 1) * sizeof(CH));
        memcpy(&pDevModeNew->dmSpecVersion, &pDevMode->dmSpecVersion,
            offsetof(DM_SRC, dmFormName) - offsetof(DM_SRC, dmSpecVersion));
        memcpy(pDevModeNew->dmFormName, (const CH*)formName,
            min(deviceName.GetLength(), CCHFORMNAME - 1) * sizeof(CH));
        memcpy(&pDevModeNew->dmLogPixels, &pDevMode->dmLogPixels, sizeTotal - offsetof(DM_SRC, dmLogPixels));
        pDevModeNew->dmSize = sizeStrucNew;

        GlobalUnlock(hDevModeNew);
    }

    GlobalUnlock(hDevMode);
    return hDevModeNew;
}

template <class STR, class CH_SRC, class CH_DST>
HGLOBAL PrivateDevNames(HGLOBAL hDevNames, const STR&, CH_SRC, CH_DST)
{
    if (hDevNames == NULL) return NULL;

    LPDEVNAMES pDevNames = LPDEVNAMES(GlobalLock(hDevNames));
    bool bUnicode = IsDevNamesUnicode(pDevNames);
    HGLOBAL hDevNamesNew = NULL;

    if (bUnicode == (sizeof(CH_DST) > 1))
    {
        int offset = max(pDevNames->wDriverOffset, max(pDevNames->wDeviceOffset, pDevNames->wOutputOffset));
        int size = offset + strlen(LPCSTR(pDevNames) + offset) + 1;
        hDevNamesNew = AllocBlock(pDevNames, size);
    }
    else
    {
        STR driver((const CH_SRC*)pDevNames + pDevNames->wDriverOffset);
        STR device((const CH_SRC*)pDevNames + pDevNames->wDeviceOffset);
        STR output((const CH_SRC*)pDevNames + pDevNames->wOutputOffset);

        hDevNamesNew = AllocBlock(
            NULL, sizeof(DEVNAMES) +
                      (driver.GetLength() + device.GetLength() + output.GetLength() + 3) * sizeof(CH_DST));
        LPDEVNAMES pDevNamesNew = LPDEVNAMES(GlobalLock(hDevNamesNew));

        pDevNamesNew->wDefault = pDevNames->wDefault;
        pDevNamesNew->wDriverOffset = sizeof(DEVNAMES) / sizeof(CH_DST);
        pDevNamesNew->wDeviceOffset = pDevNamesNew->wDriverOffset + driver.GetLength() + 1;
        pDevNamesNew->wOutputOffset = pDevNamesNew->wDeviceOffset + device.GetLength() + 1;

        memcpy((CH_DST*)pDevNamesNew + pDevNamesNew->wDriverOffset, (const CH_DST*)driver,
            (driver.GetLength() + 1) * sizeof(CH_DST));
        memcpy((CH_DST*)pDevNamesNew + pDevNamesNew->wDeviceOffset, (const CH_DST*)device,
            (device.GetLength() + 1) * sizeof(CH_DST));
        memcpy((CH_DST*)pDevNamesNew + pDevNamesNew->wOutputOffset, (const CH_DST*)output,
            (output.GetLength() + 1) * sizeof(CH_DST));
        GlobalUnlock(hDevNamesNew);
    }

    GlobalUnlock(hDevNames);
    return hDevNamesNew;
}

const DEVMODEA devModeA;
const DEVMODEW devModeW;
}  // namespace

namespace AFLibGui {
HGLOBAL DevMode2W(HGLOBAL hDevMode)
{
    return PrivateDevMode(hDevMode, devModeA, devModeW, strSpaceW, chrSpaceW);
}

HGLOBAL DevMode2A(HGLOBAL hDevMode)
{
    return PrivateDevMode(hDevMode, devModeW, devModeA, strSpaceA, chrSpaceA);
}

HGLOBAL DevNames2W(HGLOBAL hDevNames)
{
    return PrivateDevNames(hDevNames, strSpaceW, chrSpaceA, chrSpaceW);
}

HGLOBAL DevNames2A(HGLOBAL hDevNames)
{
    return PrivateDevNames(hDevNames, strSpaceA, chrSpaceW, chrSpaceA);
}
}  // namespace AFLibGui
