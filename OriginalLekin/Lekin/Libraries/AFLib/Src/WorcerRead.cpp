#include "StdAfx.h"

#include "WorcerRead.h"

#include "AFLibFileParserAbstract.h"

using AFLibPrivate::TWorcerRead;
using namespace AFLibIO;

TWorcerRead::TWorcerRead(TFileParserAbstract* pFp) : super(pFp)
{}

bool TWorcerRead::RunStage()
{
    TFileParserAbstract* pFp = static_cast<TFileParserAbstract*>(GetWorc());

    while (true)
    {
        if (pFp->m_bEof || IsFinishRaised()) return false;

        int nEol = 0;
        int nSkip = 0;

        if (pFp->FindDelimiter(pFp->m_remainder, nEol, nSkip))
        {
            pFp->m_nEol = nEol;
            pFp->m_nSkip = nSkip;
            return true;
        }

        if (pFp->m_remainder.Read(pFp->m_file, pFp->m_remainder.GetGrowBy() - 16, pFp->m_ttf) <= 0)
        {
            pFp->m_nEol = pFp->FindLastSegment(pFp->m_remainder);
            pFp->m_nSkip = pFp->m_remainder.GetLength() - pFp->m_nEol;
            pFp->m_bReachedEof = true;
            return false;
        }
    }
}
