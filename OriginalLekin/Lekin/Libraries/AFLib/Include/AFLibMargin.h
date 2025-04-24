#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TMargin structure
// Holds margins for printing, plus the portrait/landscape switch.
// User can derive classes from TMargin to save more (or less) print setup information.

namespace AFLibGui {
class AFLIB TMargin : public CObject
{
private:
    typedef CObject super;
    DEFINE_COPY_AND_ASSIGN(TMargin);

    CString m_section;  // registry section to save the margin

    // initialize margins with the default value (1")
    void PrivateInit();

protected:
    TMargin();

public:
    int m_left;         // left margin
    int m_right;        // right margin
    int m_top;          // top margin
    int m_bottom;       // bottom margin
    bool m_bLandscape;  // landscape mode?
    bool m_bSelection;  // print selection only?

    explicit TMargin(LPCTSTR section);
    ~TMargin();

    // copies data from pMargin
    virtual void Copy(const TMargin* pMargin);

    // creates a clone (copy) of TMargin
    TMargin* Clone() const;

    // get registry section
    const CString& GetSection() const
    {
        return m_section;
    }

    // loads from registry (uses current values as default)
    virtual void LoadReg();

    // saves in registry
    virtual void SaveReg() const;

    // loads from PAGESETUPDLGW
    virtual void Load(LPPAGESETUPDLGW pPageSetup);

    // saves into PAGESETUPDLGW
    virtual void Save(LPPAGESETUPDLGW pPageSetup) const;

    // computes printable rect
    virtual CRect ComputeRect(CDC* pDC) const;

    DECLARE_DYNCREATE(TMargin)
};
}  // namespace AFLibGui
