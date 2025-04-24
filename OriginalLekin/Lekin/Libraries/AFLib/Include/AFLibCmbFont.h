#pragma once

#include "AFLibCmbOwner.h"

/////////////////////////////////////////////////////////////////////////////
// CCmbFont
// Font combobox

namespace AFLibGui {
class AFLIB CCmbFont : public CCmbOwner
{
private:
    typedef CCmbOwner super;
    DEFINE_COPY_AND_ASSIGN(CCmbFont);

    // font enumeration function -- needed for initialization
    static int CALLBACK EnumFontsProc(
        const LOGFONT* lplf, const TEXTMETRIC* lptm, DWORD dwType, LPARAM lpData);

public:
    CCmbFont();

protected:
    virtual void Draw(CDC* pDC);
    virtual void OnInit();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CCmbFont)
};
}  // namespace AFLibGui
