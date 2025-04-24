#pragma once

#include "AFLibDefine.h"
#include "AFLibPointers.h"
#include "AFLibStrings.h"

/////////////////////////////////////////////////////////////////////////////
// CExcMessage exception
// Simple exception with a message

namespace AFLib {
class AFLIB CExcMessage : public CException
{
private:
    typedef CException super;
    DEFINE_COPY_AND_ASSIGN(CExcMessage);

protected:
    UINT m_idsHelp;     // help id
    CString m_message;  // exception message

public:
    explicit CExcMessage(LPCTSTR message);
    explicit CExcMessage(UINT idsHelp, LPCTSTR message = NULL);

    // prefix message with new string
    virtual void Prefix(LPCTSTR prefix);

    // prefix message with entire line
    virtual void PrefixLine(LPCTSTR prefix);

    // append message
    virtual void Postfix(LPCTSTR postfix);

    // load string, prefix
    void Prefix(UINT ids, AFLIB_NINE_STRINGS);

    // load string, prefix
    void PrefixLine(UINT ids, AFLIB_NINE_STRINGS);

    // load string, postfix
    void Postfix(UINT ids, AFLIB_NINE_STRINGS);

    virtual int ReportError(UINT nType = MB_OK, UINT nMessageID = 0);
    virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, LPUINT pnHelpContext = NULL);

    DECLARE_DYNAMIC(CExcMessage)
};
}  // namespace AFLib
