#include "StdAfx.h"

#include "AFLibFileKiller.h"

#include "AFLibIOGlobal.h"
#include "AFLibStrings.h"
#include "AFLibThread.h"

using namespace AFLib;
using AFLibIO::TFileKiller;

TFileKiller::TFileKiller(LPCTSTR path, bool bKillThis)
{
    m_path = GetFullPath(path);
    m_bKillThis = bKillThis;
    if (m_path.IsEmpty()) return;
    AppendSlash(m_path);
}

TFileKiller::~TFileKiller()
{}

void TFileKiller::PrivateRun(const CString& path, double start, double step, bool bKillThis)
{
    CStringArray arrDir;
    CStringArray arrFile;
    CFileFind FF;
    bool bMore = FF.FindFile(path + strStar) != 0;

    while (bMore)
    {
        bMore = FF.FindNextFile() != 0;
        if (FF.IsDots()) continue;
        (FF.IsDirectory() ? arrDir : arrFile).Add(FF.GetFilePath());
    }

    if (!arrDir.IsEmpty() || !arrFile.IsEmpty())
    {
        double newStep = step / (arrDir.GetSize() + arrFile.GetSize());

        for (int i = 0; i < arrDir.GetSize(); ++i)
        {
            CString& s = arrDir[i];
            AppendSlash(s);
            PrivateRun(s, start + i * newStep, newStep, true);
        }

        for (int i = 0; i < arrFile.GetSize(); ++i)
        {
            CString& s = arrFile[i];
            SetFileAttributes(s, 0);
            DeleteFile(s);
            Callback(s, start + (arrDir.GetSize() + i + 1) * newStep);
        }
    }

    FF.Close();

    if (bKillThis)
    {
        CString s = path;
        RemoveSlash(s);
        SetFileAttributes(s, 0);
        RemoveDirectory(s);
    }

    Callback(path, start + step);
}

bool TFileKiller::Run()
{
    if (m_path.IsEmpty()) return false;
    PrivateRun(m_path, 0, 1, m_bKillThis);

    CFileFind FF;
    CString s = m_path;
    RemoveSlash(s);
    return FF.FindFile(s) == 0;
}

void TFileKiller::Callback(LPCTSTR path, double progress)
{
    AFLibThread::CThread::Break();
}
