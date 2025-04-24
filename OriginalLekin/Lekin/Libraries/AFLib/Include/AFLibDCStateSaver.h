#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TDCStateSaver
// Saves the state of a Device Context;
//   restores the state on a destructor or on Close()

namespace AFLibGui {
class AFLIB TDCStateSaver
{
private:
    DEFINE_COPY_AND_ASSIGN(TDCStateSaver);

    CDC* m_pDC;           // DC for which the state has been saved
    CBrush* m_pBrushOld;  // saved brush
    CFont* m_pFontOld;    // saved font
    CPalette* m_pPalOld;  // saved palette
    CPen* m_pPenOld;      // saved pen

public:
    const COLORREF m_colorText;  // saved text color
    const COLORREF m_colorBack;  // saved background color
    const int m_modeBack;        // saved background mode (TRANSPARENT/OPAQUE)
    const int m_align;           // saved align mode

    explicit TDCStateSaver(CDC* pDC);
    ~TDCStateSaver();

    // restore the state (called in destructor, but user can call explicitly)
    void Close();
};
}  // namespace AFLibGui
