#pragma once

#include "AFLibWorcer.h"

namespace AFLibIO {
class TFileParserAbstract;
}

/////////////////////////////////////////////////////////////////////////////
// TWorcerRead
// Worcer class for TFileParserAbstract

namespace AFLibPrivate {
class TWorcerRead : public AFLibThread::TWorcer
{
private:
    typedef AFLibThread::TWorcer super;

protected:
    virtual bool RunStage();

public:
    explicit TWorcerRead(AFLibIO::TFileParserAbstract* pFp);
};
}  // namespace AFLibPrivate
