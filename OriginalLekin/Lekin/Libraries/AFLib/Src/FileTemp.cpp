#include "StdAfx.h"

#include "AFLibFileTemp.h"

#include "AFLibExcBreak.h"
#include "AFLibIOGlobal.h"

using AFLibIO::TFileTemp;

TFileTemp::TFileTemp(LPCTSTR name, bool bTemporary, int shareFlags) :
    CFile(bTemporary ? GetTmpFileName(name) : name,
        CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary | shareFlags)
{
    m_name = GetFilePath();
}

TFileTemp::~TFileTemp()
{
    CloseAndDelete();
}

void TFileTemp::CloseAndDelete()
{
    try
    {
        if (m_hFile != hFileNull) super::Close();
        Remove(m_name);
    }
    catch (AFLibThread::CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }
}

void TFileTemp::ReleaseWritingRights()
{
    super::Close();

    CFileException e;
    if (!super::Open(m_name, CFile::modeRead | CFile::shareDenyWrite, &e))
        AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
}
