#pragma once

#include "AFLibLCBDraw.h"

/////////////////////////////////////////////////////////////////////////////
// CCmbOwner
// Owner-draw combobox

namespace AFLibGui {
class AFLIB CCmbOwner : public CComboBox
{
private:
    typedef CComboBox super;
    DEFINE_COPY_AND_ASSIGN(CCmbOwner);

public:
    CCmbOwner();

    // calls MeasureItem and sets item height accordingly
    void ResetItemHeight();

    // add empty string, set data
    int AddData(LPVOID data);

    // get data for selected member
    LPVOID GetData();

    // find and select string
    void SelString(LPCTSTR data);

    // find an item by data value, select
    void SelData(LPVOID data);

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);

protected:
    TLCBDraw m_draw;  // Drawing information

    virtual void PreSubclassWindow();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

    virtual void OnInit();           // called in Subclass and Create
    virtual void GetItemText();      // get item text
    virtual void Draw(CDC* pDC);     // DrawItem wrapper
    virtual int Measure(int index);  // MeasureItem wrapper

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CCmbOwner)
};
}  // namespace AFLibGui
