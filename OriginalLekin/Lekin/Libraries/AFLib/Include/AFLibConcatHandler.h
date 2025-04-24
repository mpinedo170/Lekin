#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TConcatHandler
// Both TStringSpacer and TStringWriter use a flag whether to
//   concatenate strings in certain situations.  This class is used
//   to capture common functionality

namespace AFLibPrivate {
class AFLIB TConcatHandler
{
private:
    DEFINE_COPY_AND_ASSIGN(TConcatHandler);

    bool m_bConcatOn;  // flag on/off

protected:
    TConcatHandler();
    virtual ~TConcatHandler();

public:
    // set the flag on or off
    void SetConcat(bool bOn);

    // get flag value
    bool GetConcat();

    // check whether a character is "safe" to output without quotes
    virtual bool IsChrSafe(TCHAR ch);

    // manipulator class: destructor restores the original value of the flag
    class AFLIB TConcatManipulator
    {
    private:
        DEFINE_COPY_AND_ASSIGN(TConcatManipulator);

        TConcatHandler& m_host;
        bool m_bConcatOnSaved;

    public:
        TConcatManipulator(TConcatHandler& host, bool bConcatOn);
        ~TConcatManipulator();
    };
};
}  // namespace AFLibPrivate
