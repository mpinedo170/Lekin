#pragma once

#include "AFLibDefine.h"
#include "AFLibTempTT.h"

/////////////////////////////////////////////////////////////////////////////
// CProTT property page
// A property page that supports tool tips

namespace AFLibGui {
class AFLIB CProTT : public CTempTT<CPropertyPage>
{
private:
    typedef CTempTT<CPropertyPage> super;
    DEFINE_COPY_AND_ASSIGN(CProTT);

public:
    explicit CProTT(UINT nIDTemplate);

protected:
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CProTT)
};
}  // namespace AFLibGui
