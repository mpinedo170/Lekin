#include "StdAfx.h"

#include "AFLibLstDragCompanion.h"

#include "AFLibDragField.h"

using AFLibGui::CLstDragCompanion;
using AFLibGui::TDragField;

CLstDragCompanion::CLstDragCompanion()
{
    m_bFocusDetected = m_bDragEnabled = true;
    m_pDf = NULL;
}

BEGIN_MESSAGE_MAP(CLstDragCompanion, super)
ON_WM_DESTROY()
END_MESSAGE_MAP()

void CLstDragCompanion::SetDragField(TDragField* pDf)
{
    if (pDf == m_pDf) return;

    if (m_pDf != NULL) m_pDf->RemoveCompanion(this);

    m_pDf = pDf;
    if (m_pDf != NULL) m_pDf->AddCompanion(this);
}

bool CLstDragCompanion::StartDrag(const CPoint& pt)
{
    SendSelchange();
    if (m_pDf != NULL) m_pDf->StartDrag(this);
    return false;
}

BOOL CLstDragCompanion::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        switch (pMsg->wParam)
        {
            case VK_ESCAPE:
                m_pDf->CancelDrag();
            case VK_RETURN:
                return true;
        }
    }
    return super::PreTranslateMessage(pMsg);
}

void CLstDragCompanion::OnDestroy()
{
    SetDragField(NULL);
    super::OnDestroy();
}
