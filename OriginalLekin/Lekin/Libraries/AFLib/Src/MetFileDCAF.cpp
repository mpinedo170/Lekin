#include "StdAfx.h"

#include "AFLibMetaFileDCAF.h"

#include "AFLibStringBuffer.h"

using AFLibGui::CMetaFileDCAF;
using namespace AFLibIO;

CMetaFileDCAF::CMetaFileDCAF(
    HDC hDCRef, LPCTSTR strApp, LPCTSTR strTitle, const CSize& sz, LPCTSTR lpszFilename)
{
    m_bDeleteRefDC = false;

    TStringBuffer title;
    title += strApp;
    title.AppendCh(0, 1);
    title += strTitle;
    title.AppendCh(0, 2);

    bool bDeleteRefDC = hDCRef == NULL;
    if (bDeleteRefDC) hDCRef = ::GetDC(::GetDesktopWindow());

    CRect rectMM(0, 0, sz.cx * ::GetDeviceCaps(hDCRef, HORZSIZE) * 100 / ::GetDeviceCaps(hDCRef, HORZRES),
        sz.cy * ::GetDeviceCaps(hDCRef, VERTSIZE) * 100 / ::GetDeviceCaps(hDCRef, VERTRES));

    Attach(::CreateEnhMetaFile(hDCRef, lpszFilename, rectMM, title));
    SetAttribDC(hDCRef);
    m_bDeleteRefDC = bDeleteRefDC;
}

CMetaFileDCAF::~CMetaFileDCAF()
{
    ReleaseAttribDC();
}

void CMetaFileDCAF::SetAttribDC(HDC hDC)
{
    ReleaseAttribDC();
    super::SetAttribDC(hDC);
}

void CMetaFileDCAF::ReleaseAttribDC()
{
    HDC hDC = m_hAttribDC;
    super::ReleaseAttribDC();
    if (!m_bDeleteRefDC) return;

    ASSERT(hDC != NULL);
    ::ReleaseDC(::GetDesktopWindow(), hDC);
    m_bDeleteRefDC = false;
}
