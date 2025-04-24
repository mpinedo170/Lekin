#pragma once

#include "ID.h"

class CBtnStyle : public CBtnOwner
{
private:
    typedef CBtnOwner super;

    int m_idc;

public:
    TID m_style;
    CBtnStyle(TType ty, UINT idc);

protected:
    DECLARE_MESSAGE_MAP()

    virtual void Draw(CDC* pDC);
    afx_msg void OnClicked();
};
