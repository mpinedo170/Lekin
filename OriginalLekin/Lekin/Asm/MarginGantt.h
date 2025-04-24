#pragma once

/////////////////////////////////////////////////////////////////////////////
// TMarginGantt structure
// holds margins for CUSUM printing

class TMarginGantt : public TMargin
{
private:
    typedef TMargin super;

    void Init();

protected:
    TMarginGantt();

public:
    int m_nX;         // number of pages in one row
    int m_nY;         // number of pages in one column
    bool m_bNoSplit;  // do not split workcenters

    TMarginGantt(LPCTSTR section);
    virtual void Copy(const TMargin* pMargin);

    virtual void LoadReg();
    virtual void SaveReg();

    DECLARE_DYNCREATE(TMarginGantt)
};
