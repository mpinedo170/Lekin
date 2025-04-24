#include "StdAfx.h"

#include "AFLibDitemArray.h"

#include "AFLibGlobal.h"

using AFLibGui::TDitemArray;

void TDitemArray::ChangeDirs(LPCTSTR oldPath, LPCTSTR newPath)
{
    for (int i = 0; i < GetSize(); ++i)
    {
        TDitem* pDitem = GetAt(i);
        CString dir = pDitem->GetDir();
        ASSERT(AFLib::StringStartsWith(dir, oldPath));
        dir = newPath + dir.Mid(_tcslen(oldPath));
        pDitem->SetDir(dir);
    }
}
