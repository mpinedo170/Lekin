#pragma once

#include "AFLibExcMessage.h"

/////////////////////////////////////////////////////////////////////////////
// CExcMessage exception
// Simple exception with a message

namespace AFLib {
class AFLIB CExcMsgBox : public CExcMessage
{
private:
    typedef CExcMessage super;

protected:
    UINT m_nType;  // type of message box

public:
    CExcMsgBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);

    // get the type of message box
    int GetMBType()
    {
        return m_nType;
    }

    virtual int ReportError(UINT nType = -1, UINT nMessageID = 0);

    DECLARE_DYNAMIC(CExcMsgBox)
};
}  // namespace AFLib
