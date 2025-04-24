#pragma once

#include "AFLibArrays.h"
#include "AFLibStringBuffer.h"

class CRichEditCtrl;
struct CHARFORMAT2;

namespace AFLibIO {
class TStringWriter;
class TStringSpacer;
}  // namespace AFLibIO

namespace AFLibGui {
class CImageAF;
class TFontDescriptor;
};  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// TSuperTextGuts
// Class to simplify copying TSuperText

namespace AFLibPrivate {
class AFLIB TSuperTextGuts
{
protected:
    const AFLibGui::TFontDescriptor* m_pFdc;  // font generator
    int m_growBy;                             // grow-by size for the contents
    DWORD m_attributeNew;                     // attribute for the coming text
    DWORD m_attributeOld;                     // last attribute
    int m_tab;                                // current tab number
    int m_lineSkipCount;                      // lines to skip on page break
    double m_padding;                         // tab padding (in m_hFont's)
    int m_realLength;                         // index of the last non-whitespace symbol
};
}  // namespace AFLibPrivate

/////////////////////////////////////////////////////////////////////////////
// TSuperText
// Holds Unicode text with simple attributes: bold, color, etc.
// May have custom-drawable characters.
// Computes tabulations, draws text or inserts it into a rich-edit ctrl.

namespace AFLibGui {
class AFLIB TSuperText : public AFLibPrivate::TSuperTextGuts
{
private:
    typedef AFLibPrivate::TSuperTextGuts super;

    static const CPoint m_ptSizeOnly;

    // text flow breaks
    enum TBrk
    {
        brkNone,
        brkTab,
        brkEol,
        brkPage
    };

    // character attributes
    enum
    {
        atBrk = 0x00000003,
        atSpecial = 0x00000004,
        atBold = 0x00000008,
        atItalic = 0x00000010,
        atUnderline = 0x00000020,
        atTab = 0x0000FF00,
        atColorBack = 0x00FF0000,
        atColorText = 0xFF000000,
        atSaved = ~(atBrk | atTab)
    };

    // TNextText::m_attribute looks as follows:
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |txt color index|bkg color index|   tab number  | 0 |u|i|B|s|brk|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // 8 bit per color => max 256 text & back colors
    // 0x0000FF00 is an invalid attribute since tab number
    // is not saved with attributes.

    // for processing tabs extending over one cell
    struct TExtraTab
    {
        int m_tabFrom;  // from column
        int m_tabTo;    // to column
        int m_width;    // computed width

        // key for sorting; sorts by 1. width in columns, 2. first column
        UINT GetGLKey() const
        {
            return UINT(m_tabTo - m_tabFrom) * 0x10000 + UINT(m_tabFrom);
        }
    };

protected:
    // data localized for drawing in a Windows DC
    class TDrawCache;

private:
    // text iterator
    class TNextText
    {
    private:
        const TSuperText* m_pHost;       // host TSuperText
        const TDrawCache* m_pDrawCache;  // used TDrawCache (may be null)
        CPoint m_ptOrigin;               // point to start drawing from
        bool m_bSizeOnly;                // no drawing, only checking the size
        int m_indexEnd;                  // end of buffer
        int m_index;                     // current position

        void DrawBackground(CDC* pDC);
        void Underline(CDC* pDC);

    public:
        // the following members are reset by Parse()
        DWORD m_attribute;              // attribute
        TBrk m_brk;                     // break state
        COLORREF m_colorText;           // foreground color
        COLORREF m_colorBack;           // background color
        AFLibIO::TStringBufferW m_str;  // text
        int m_tabNew;                   // tab to jump to

        int m_width;  // text width; reset in Draw()

        // the following members are modified in Move()
        int m_tab;           // current tab
        int m_wCurTab;       // total tab width
        CPoint m_ptCurrent;  // current drawing point

        TNextText(const TSuperText* pHost,
            const TDrawCache* pDrawCache,
            const CPoint& ptOrigin,
            int indexStart,
            int indexEnd);

        // parse until the next break or attribute change
        void Parse(CDC* pDC);

        // draw the parsed piece
        void Draw(CDC* pDC);

        // move on
        void Move();

        // add the current tab width to the array
        bool ComputeSimpleTab(AFLib::CIntArray& arrTabWidth);

        // fill char format based on current attribute
        void FillCharacterFormat(CHARFORMAT2& cf);

        // set text color on pDC according to current attributes
        void ResetColorText(CDC* pDC);

        // parsed to the end?
        bool IsEof() const
        {
            return m_index >= m_indexEnd;
        }

        // current index
        int GetIndex()
        {
            return m_index;
        }

        // custom-drawable string?
        bool IsSpecial() const
        {
            return (m_attribute & atSpecial) != 0;
        }

        // end-of-line?
        bool IsBrk() const
        {
            return m_brk > brkNone;
        }

        // bold font?
        bool IsBold() const
        {
            return (m_attribute & atBold) != 0;
        }

        // italic font?
        bool IsItalic() const
        {
            return (m_attribute & atItalic) != 0;
        }

        // underlined font?
        bool IsUnderline() const
        {
            return (m_attribute & atUnderline) != 0;
        }
    };

    // number of standard fonts (Reg, Bold, Italic, Bold Italic)
    static const int nFonts = 4;

    // invalid attribute
    static const DWORD BadAttribute = 0x0000FF00;

    // default number of linebreaks in case of page break
    static const int DefaultLineSkipCount = 1;

    // max number of colors
    static const int MaxColorIndex = 0xFF;

    // max supported number of tabs
    static const int MaxTabIndex = 0xFE;

    // default tab padding
    static const double DefaultPadding;

    // Font holder
    // Simplifies creating custom fonts for user-drawable chars
    class TFontFamily
    {
    private:
        // font desriptor
        const TFontDescriptor* m_pFdc;

        // font size in pixels
        int m_pixelSize;

        // actual fonts
        mutable CFont* m_arrFont[nFonts];

    public:
        TFontFamily(const TFontDescriptor* pFdc, int pixelSize);
        ~TFontFamily();

        // compare parameters
        bool IsSame(const TFontDescriptor* pFdc, int pixelSize) const;

        // get one of fonts (create if necessary)
        CFont* GetFont(CDC* pDC, bool bBold, bool bItalic) const;
    };

    CWordArray m_arrContent;               // main contents
    AFLib::CDoubleArray m_arrMinTabWidth;  // min tab width (in hFont's)

    // get number of skipped lines for a break
    int GetSkipCount(TBrk brk) const
    {
        return brk >= brkPage ? m_lineSkipCount : 1;
    }

    // extract break from attribute
    static TBrk GetBrk(DWORD attribute)
    {
        return TBrk(attribute & atBrk);
    }

    // add a 2-byte word to the buffer
    void AddC(WORD word)
    {
        m_arrContent.Add(word);
    }

    // get 2-word (4-byte) attribute at index
    DWORD GetAttribute(int index) const
    {
        return *reinterpret_cast<LPCDWORD>(m_arrContent.GetData() + index);
    }
    DWORD& GetAttribute(int index)
    {
        return *reinterpret_cast<LPDWORD>(m_arrContent.GetData() + index);
    }

    // add zero and attribute (LO and HI)
    void AddA(DWORD attribute);

    // clears data
    void PrivateClear();

    // clear drawing cache
    void PrivateClearCache() const;

    // add a breaking symbol
    void AddBrk(TBrk br);

    // Page info
    struct TPage
    {
        int m_index;        // start of the page in buffer
        DWORD m_attribute;  // starting attribute

        TPage()
        {
            Clear();
        }
        TPage(int index, DWORD attribute) : m_index(index), m_attribute(attribute & atSaved)
        {}

        // make an invalid page
        void Clear()
        {
            m_index = -1;
            m_attribute = 0;
        }

        // is page valid?
        bool IsStarted()
        {
            return m_index >= 0;
        }

        // first page
        static const TPage m_pageZero;
    };

protected:
    // cached drawing data
    class AFLIB TDrawCache
    {
    private:
        const TSuperText* m_pHost;  // host super-text

        int m_xLogPix;                                    // device resolution (x)
        int m_yLogPix;                                    // device resolution (y)
        bool m_bTooManyTabs;                              // too many tabs to fit well into rich edit
        int m_maxTabWidth;                                // maximal tab width
        int m_hFont;                                      // font height
        int m_hHeader;                                    // header height
        AFLib::CIntArray m_arrTab;                        // tab start points
        CSize m_szTotal;                                  // total size of the text (in pixels)
        CArray<TPage> m_arrPage;                          // page start points
        TPage m_pageNice;                                 // end of page at a page break
        TPage m_pageHard;                                 // end of page, limited by pageHeight
        int m_hPageExtra;                                 // space between m_pageEndHard and m_pageEndNice
        COLORREF m_colorTextDefault;                      // starting CDC foreground color
        AFLib::CSmartArray<TFontFamily> m_arrFontFamily;  // fonts
        LPVOID m_pExtraData;  // data that may be used in classes derived from TSuperText

        TDrawCache(const TSuperText* pHost, CDC* pDC);

        // compute everything
        void Init(CDC* pDC);

        // does this Draw Cache fit a given CD?
        bool Fits(CDC* pDC) const;

        // clear pagination variables (NOT m_arrPage)
        void ClearPaging();

        // underline a segment of text
        void Underline(CDC* pDC, const CPoint& point, int width) const;

        // draw a string of special symbols
        int DrawSpecialStr(
            CDC* pDC, LPCWSTR str, const CPoint& point, bool bBold, bool bItalic, bool bUnderline) const;

        // draw a text segment
        void DrawSegment(CDC* pDC, const CPoint& point, const TPage& pageStart, int indexEnd) const;

        // append sAdd to wndRich, clear sAdd
        static void FlushRich(CRichEditCtrl& wndRich, AFLibIO::TStringBufferW& sAdd, CHARFORMAT2& cf);

        // upload up to length
        void UploadRich(CDC* pDC, COLORREF colorBackDefault, CRichEditCtrl& wndRich, int length) const;

        // find the end of the next page; returns false if no more pages
        bool ProcessNextPage(int pageHeight);

        friend TSuperText;
        friend TNextText;

    public:
        ~TDrawCache();

        // add a font family to the list (can only be called from InitFontFamilies)
        int AddFontFamily(const TFontDescriptor* pFdc, int pixelSize = -1);

        // get font height (in pixels)
        int GetHFont() const
        {
            return m_hFont;
        }

        // create a font according to the DC characteristics and attributes
        CFont* GetFont(int family, CDC* pDC, bool bBold, bool bItalic) const;

        // allocate "extra-data" pointer
        LPVOID AllocateExtraData(int amount);

        // get the extra data
        LPCVOID GetExtraData() const
        {
            return m_pExtraData;
        }
    };

private:
    // end of header
    TPage m_pageHeader;

    // syncronization object for m_arrDrawCache
    CCriticalSection* m_pSync;

    // draw cache for different DC's
    mutable AFLib::CSmartArray<TDrawCache> m_arrDrawCache;

    // fill the drawing cache
    TDrawCache* Compute(CDC* pDC) const;

    // read next token; returns true if it is an attribute
    bool ReadNextToken(int& index, int indexEnd, DWORD& attribute, AFLibIO::TStringBufferW& str) const;

    // for RTF conversion
    static DWORD CALLBACK StreamOutFunc(DWORD dwCookie, LPBYTE pbBuff, long cb, LPLONG pcb);

    // translate, if the attribute's text and/or back 'ci' is zero
    static void TranslateAttribute(DWORD& attribute, BYTE ciText, BYTE ciBack);

    // upload up to length
    void PrivateUploadRich(CRichEditCtrl& wndRich, int length) const;

    // mark header at the specified location
    void PrivateMarkHeader(int index);

    // helper for IsPaged() and GetPageCount()
    int PrivateGetPageCount(CDC* pDC) const;

protected:
    // add a regular Unicode character
    void AddChar(WCHAR ch);

    // add an ASCII character
    void AddChar(CHAR ch)
    {
        AddChar(WCHAR(BYTE(ch)));
    }

    // add a special character (virtual -- some may translate into regular)
    virtual void AddSpecial(WCHAR ch);

    // draw a special symbol (override for custom drawing)
    virtual int DrawSpecial(CDC* pDC,
        const TDrawCache* pDrawCache,
        WCHAR ch,
        const CPoint& point,
        bool bBold,
        bool bItalic) const;

    // initialize m_arrFontFamily; override if new fonts are needed
    virtual void InitFontFamilies(TDrawCache* pDrawCache) const;

    // convert a special symbol to a Unicode string
    virtual void SpecialToUnicode(WCHAR ch, AFLibIO::TStringBufferW& buffer) const;

    // convert an index into a color, according to pDC's properties; may return colorAuto
    virtual COLORREF GetRealColor(CDC* pDC, BYTE ci) const;

    // in DrawSpecial(): if true, compute size only
    static bool IsSizeOnly(const CPoint& point)
    {
        return &point == &m_ptSizeOnly;
    }

    // set attribute bit
    void SetAttrBit(bool bSet, DWORD mask);

    // get attribute bit
    bool GetAttrBit(DWORD mask) const
    {
        return (m_attributeNew & mask) != 0;
    }

    // get length of text
    int GetLength() const
    {
        return m_arrContent.GetSize();
    }

    // get length of text, ignore trailing whitespace
    int GetRealLength() const
    {
        return m_realLength;
    }

public:
    explicit TSuperText(const TFontDescriptor* pFdc = NULL);
    TSuperText(const TSuperText& superText);
    virtual ~TSuperText();

    TSuperText& operator=(const TSuperText& superText);

    // empty the buffers
    virtual void Clear();

    // set the font descriptor (must be done before computing)
    void SetFontDescriptor(const TFontDescriptor* pFdc);

    // set the grow-by parameter for the buffer
    void SetGrowBy(int growBy);

    // save in a stream
    virtual void Save(AFLibIO::TStringWriter& sw) const;

    // load from a stream
    virtual void Load(AFLibIO::TStringSpacer& sp);

    // convert to Unicode text
    CStringW ToUnicode() const;

    // convert to RTF
    CString ToRtf() const;

    // copy to clipboard: CF_TEXT, CF_UNICODETEXT, and RTF
    void Copy() const;

    // add an Ascii character
    void Add(CHAR ch);

    // add a Unicode character
    void Add(WCHAR ch);

    // add an Ascii string
    void Add(LPCSTR s);

    // add a Unicode string
    void Add(LPCWSTR s);

    // add another TSuperText
    void Add(const TSuperText& superText);

    // set the Bold attribute for the following text
    void SetBold(bool bBold)
    {
        SetAttrBit(bBold, atBold);
    }

    // set the Italic attribute for the following text
    void SetItalic(bool bItalic)
    {
        SetAttrBit(bItalic, atItalic);
    }

    // set the Underline attribute for the following text
    void SetUnderline(bool bUnderline)
    {
        SetAttrBit(bUnderline, atUnderline);
    }

    // set color index for the following text
    void SetCIText(BYTE ci);

    // set background color index for the following text
    void SetCIBack(BYTE ci);

    // jump to a tab
    void SetTab(int tab);

    // set how many lines are skipped at page break
    void SetLineSkipCount(int lineSkipCount);

    // set tab padding
    void SetPadding(double padding);

    // set minimal tab width
    void SetMinTabWidth(int tab, double width);

    // is buffer empty?
    bool IsEmpty()
    {
        return GetLength() == 0;
    }

    // will the following text be Bold?
    bool GetBold() const
    {
        return GetAttrBit(atBold);
    }

    // will the following text be Italic?
    bool GetItalic() const
    {
        return GetAttrBit(atItalic);
    }

    // will the following text be Underlined?
    bool GetUnderline() const
    {
        return GetAttrBit(atUnderline);
    }

    // get the current tab number
    int GetTab() const
    {
        return m_tab;
    }

    // get the number of skipped lines at page break
    int GetLineSkipCount()
    {
        return m_lineSkipCount;
    }

    // get the tab padding coefficient
    double GetPadding() const
    {
        return m_padding;
    }

    // get the color index for the following text
    BYTE GetCIText() const;

    // get the background color index for the following text
    BYTE GetCIBack() const;

    // get minimal tab width
    double GetMinTabWidth(int tab) const;

    // clear drawing cache
    virtual void ClearCache() const;

    // get the size of the full text; returns the height of font
    int GetExtent(CDC* pDC, CSize& sz) const;

    // draw a text segment
    void DrawSegment(CDC* pDC, const CPoint& point, const TPage& pageStart, int indexEnd) const;

    // draw full text
    void Draw(CDC* pDC, const CPoint& point) const;

    // draw a single page
    void DrawPage(CDC* pDC, const CPoint& point, int page) const;

    // upload the text into a Rich Edit control
    void UploadRich(CRichEditCtrl& wndRich) const;

    // upload the text into a Rich Edit control, but assume we are alone in the control
    void UploadClearRich(CRichEditCtrl& wndRich, bool bReturnCursor = true) const;

    // draw the whole text in a bitmap
    void CreateImage(CImageAF& img) const;

    // every page will have a header -- from the beginning to the current point
    void MarkHeader();

    // start paginating
    void StartPaging(CDC* pDC) const;

    // find the end of the next page; returns false if no more pages
    bool ProcessNextPage(CDC* pDC, int pageHeight) const;

    // has pagination been finished?
    bool IsPaged(CDC* pDC) const;

    // the number of pages
    int GetPageCount(CDC* pDC) const;

    // class for manipulating the Bold flag:
    // constructor sets it to new value,
    // destructor restores the old value

    class AFLIB TBoldManipulator
    {
    private:
        // object to which the manipulator is being applied
        TSuperText& m_host;

        // old value of the Bold flag
        bool m_bBoldSaved;

    public:
        TBoldManipulator(TSuperText& host, bool bBold);
        ~TBoldManipulator();
    };

    // class for manipulating the Italic flag:
    // constructor sets it to new value,
    // destructor restores the old value

    class AFLIB TItalicManipulator
    {
    private:
        // object to which the manipulator is being applied
        TSuperText& m_host;

        // old value of the Italic flag
        bool m_bItalicSaved;

    public:
        TItalicManipulator(TSuperText& host, bool bItalic);
        ~TItalicManipulator();
    };
};
}  // namespace AFLibGui
