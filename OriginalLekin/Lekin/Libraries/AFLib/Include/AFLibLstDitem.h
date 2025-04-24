#pragma once

#include "AFLibLstOwner.h"

/////////////////////////////////////////////////////////////////////////////
// CLstDitem list box
// List of Ditems

namespace AFLibGui {
class TDitem;
class TDitemArray;

class AFLIB CLstDitem : public CLstOwner
{
private:
    typedef CLstOwner super;
    DEFINE_COPY_AND_ASSIGN(CLstDitem);

public:
    CLstDitem();

    // add ditem to the list
    int AddDitem(TDitem* pDitem);

    // add all ditems to the list
    void AddDitemArray(TDitemArray& arrExs, bool bSelect);

    // get scan for given number
    TDitem* GetDitem(int index);

protected:
    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibGui
