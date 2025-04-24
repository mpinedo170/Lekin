#pragma once

#include "AFLibBtnOwner.h"

/////////////////////////////////////////////////////////////////////////////
// CBtnImg
// Bitmap button.  Bitmap is loaded using the control ID

namespace AFLibGui {
class CImageAF;

class AFLIB CBtnImg : public CBtnOwner
{
private:
    typedef CBtnOwner super;
    DEFINE_COPY_AND_ASSIGN(CBtnImg);

public:
    CImageAF* m_pImgRegular;   // appearance when button is active
    CImageAF* m_pImgDisabled;  // appearance when button is disabled

    CBtnImg();
    ~CBtnImg();

protected:
    virtual void OnInit();
    virtual void Draw(CDC* pDC);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CBtnImg)
};
}  // namespace AFLibGui
