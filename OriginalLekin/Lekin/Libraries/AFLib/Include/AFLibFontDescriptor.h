#pragma once

#include "AFLibArrays.h"
#include "AFLibGuiGlobal.h"

/////////////////////////////////////////////////////////////////////////////
// TFontDescriptor
// Font attributes

namespace AFLibGui {
class AFLIB TFontDescriptor
{
private:
    CString m_title;  // Title of the font (where it is used)
    CString m_face;   // Font typeface
    int m_size;       // Size of the font
    bool m_bBold;     // Use bold font?
    bool m_bItalic;   // Use italic font?

public:
    TFontDescriptor(LPCTSTR title, LPCTSTR face, int size, bool bBold = false, bool bItalic = false);

    // Read the structure from the registry
    void LoadRegistry(LPCTSTR section);

    // Save in the registry
    void SaveRegistry(LPCTSTR section) const;

    // Read the structure from a dilog box
    void LoadDlg(CWnd* pWnd, int minSize, int maxSize);

    // Show in a dilog box
    void SaveDlg(CWnd* pWnd) const;

    // create a font according to the settings; Bold, Italic, and Size may be overridden
    void CreateFont(
        CFont& font, CDC* pDC, bool bBold = false, bool bItalic = false, int pixelSize = -1) const;

    // build a CHARFORMAT for inserting into a RichEdit
    void FillCharacterFormat(CHARFORMAT2& cf,
        bool bBold = false,
        bool bItalic = false,
        bool bUnderline = false,
        COLORREF colorText = colorAuto,
        COLORREF colorBack = colorAuto) const;

    // get font title
    const CString& GetTitle() const
    {
        return m_title;
    }
};

class AFLIB TFontDescriptorArray : public AFLib::CSmartArray<TFontDescriptor>
{
public:
    // append new array to this; make copies of members
    void DeepAppend(const TFontDescriptorArray& arr);
};
}  // namespace AFLibGui
