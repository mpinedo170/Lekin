#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CMetaFileDCAF
// A MetaFile DC with simplified construction

namespace AFLibGui {
class AFLIB CMetaFileDCAF : public CMetaFileDC
{
private:
    typedef CMetaFileDC super;
    DEFINE_COPY_AND_ASSIGN(CMetaFileDCAF);

    bool m_bDeleteRefDC;  // delete reference DC when done?

public:
    CMetaFileDCAF(HDC hDCRef, LPCTSTR strApp, LPCTSTR strTitle, const CSize& sz, LPCTSTR lpszFilename = NULL);
    ~CMetaFileDCAF();

    virtual void SetAttribDC(HDC hDC);
    virtual void ReleaseAttribDC();
};
}  // namespace AFLibGui
