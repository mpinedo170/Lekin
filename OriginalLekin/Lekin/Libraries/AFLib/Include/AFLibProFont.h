#pragma once

#include "AFLibCmbFont.h"
#include "AFLibFontDescriptor.h"
#include "AFLibProTT.h"

class CSpinButtonCtrl;

/////////////////////////////////////////////////////////////////////////////
// CProFont property page -- a property page that contains a "font" group box

namespace AFLibGui {
class AFLIB CProFont : public CProTT
{
private:
    typedef CProTT super;
    DEFINE_COPY_AND_ASSIGN(CProFont);

    int m_sel;           // selection in the "Font type" combobox
    CCmbFont m_cmbFace;  // font combobox

    // read font typeface, size and boldness into m_arrFont
    bool ReadFontData();

protected:
    CComboBox m_cmbFontType;       // Font Type combobox
    CSpinButtonCtrl* m_pSpinSize;  // Font Size spin button

    int m_minFontSize;  // min allowed font size
    int m_maxFontSize;  // max allowed font size

public:
    explicit CProFont(UINT nIDTemplate);
    ~CProFont();

    // array of font descriptions (pointers into property sheet)
    TFontDescriptorArray m_arrFont;

    virtual BOOL OnKillActive();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelChangeFontType();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CProFont)
};
}  // namespace AFLibGui
