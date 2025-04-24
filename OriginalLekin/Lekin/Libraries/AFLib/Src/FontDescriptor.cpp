#include "StdAfx.h"

#include "AFLibFontDescriptor.h"

#include "AFLibExcBreak.h"
#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibResource.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringTokenizer.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;
using AFLibGui::TFontDescriptor;
using AFLibGui::TFontDescriptorArray;

TFontDescriptor::TFontDescriptor(LPCTSTR title, LPCTSTR face, int size, bool bBold, bool bItalic) :
    m_title(title),
    m_face(face),
    m_size(size),
    m_bBold(bBold),
    m_bItalic(bItalic)
{}

void TFontDescriptor::LoadRegistry(LPCTSTR section)
{
    CString s = DynamicGetProfileString(section, m_title, strEmpty);
    TStringTokenizer sp(s, chrSemicolon);
    if (sp.IsEof()) return;

    try
    {
        m_face = sp.ReadStr();
        m_size = sp.ReadInt();
        CString attr = sp.ReadStr();
        m_bBold = attr.Find(_T('B')) >= 0;
        m_bItalic = attr.Find(_T('I')) >= 0;
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

void TFontDescriptor::SaveRegistry(LPCTSTR section) const
{
    TStringBuffer buffer;

    buffer += m_face;
    buffer += chrSemicolon;
    buffer.AppendInt(m_size);
    buffer += chrSemicolon;

    if (m_bBold) buffer += _T('B');
    if (m_bItalic) buffer += _T('I');
    DynamicWriteProfileString(section, m_title, buffer);
}

void TFontDescriptor::LoadDlg(CWnd* pWnd, int minSize, int maxSize)
{
    CString face;
    if (pWnd->GetDlgItemText(IDC_FONT_FACE, face) <= 0) face = m_face;

    int size = m_size;
    if (pWnd->GetDlgItem(IDC_FONT_SIZE) != NULL)
    {
        CString s;
        pWnd->GetDlgItemText(IDC_FONT_SIZE, s);
        size = StrToInt(s);

        if (size < minSize || size > maxSize)
        {
            pWnd->GetDlgItem(IDC_FONT_SIZE)->SetFocus();
            ThrowMessage(LocalAfxString(IDS_BAD_FONT));
        }
    }

    m_face = face;
    m_size = size;

    if (pWnd->GetDlgItem(IDC_FONT_BOLD) != NULL) m_bBold = pWnd->IsDlgButtonChecked(IDC_FONT_BOLD) != 0;

    if (pWnd->GetDlgItem(IDC_FONT_ITALIC) != NULL) m_bItalic = pWnd->IsDlgButtonChecked(IDC_FONT_ITALIC) != 0;
}

void TFontDescriptor::SaveDlg(CWnd* pWnd) const
{
    if (pWnd->GetDlgItem(IDC_FONT_FACE) != NULL)
        pWnd->GetDlgItem(IDC_FONT_FACE)->SendMessage(CB_SELECTSTRING, WPARAM(-1), LPARAM(LPCTSTR(m_face)));

    pWnd->SetDlgItemInt(IDC_FONT_SIZE, m_size);
    pWnd->CheckDlgButton(IDC_FONT_BOLD, m_bBold ? 1 : 0);
    pWnd->CheckDlgButton(IDC_FONT_ITALIC, m_bItalic ? 1 : 0);
}

void TFontDescriptor::CreateFont(CFont& font, CDC* pDC, bool bBold, bool bItalic, int pixelSize) const
{
    if (pixelSize == -1)
        CreateLogicalFont(font, pDC, m_face, m_size, m_bBold | bBold, m_bItalic | bItalic);
    else
        CreateAbsoluteFont(font, m_face, pixelSize, m_bBold | bBold, m_bItalic | bItalic);
}

void TFontDescriptor::FillCharacterFormat(
    CHARFORMAT2& cf, bool bBold, bool bItalic, bool bUnderline, COLORREF colorText, COLORREF colorBack) const
{
    memset(&cf, 0, sizeof(CHARFORMAT2));
    cf.cbSize = sizeof(CHARFORMAT2);
    cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_FACE | CFM_SIZE | CFM_UNDERLINE | CFM_COLOR | CFM_BACKCOLOR;

    cf.dwEffects = (m_bBold | bBold ? CFE_BOLD : 0) | (m_bItalic | bItalic ? CFE_ITALIC : 0) |
                   (bUnderline ? CFE_UNDERLINE : 0);

    if (colorText == colorAuto)
        cf.dwEffects |= CFE_AUTOCOLOR;
    else
        cf.crTextColor = colorText;

    if (colorBack == colorAuto)
        cf.dwEffects |= CFE_AUTOBACKCOLOR;
    else
        cf.crBackColor = colorBack;

    cf.yHeight = m_size * 20;
    _tcsncpy_s(cf.szFaceName, LF_FACESIZE, m_face, LF_FACESIZE - 1);
}

void TFontDescriptorArray::DeepAppend(const TFontDescriptorArray& arr)
{
    for (int i = 0; i < arr.GetSize(); ++i) Add(new TFontDescriptor(*arr[i]));
}
