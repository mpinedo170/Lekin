#include "StdAfx.h"

#include "AFLibExcBreak.h"

#include "AFLibStrings.h"
#include "Local.h"

using namespace AFLib;
using AFLibThread::CExcBreak;

IMPLEMENT_DYNAMIC(CExcBreak, super)

CExcBreak::CExcBreak(LPCTSTR message) :
    super(IsEmptyStr(message) ? AFLibPrivate::LocalAfxString(IDS_BREAK) : message)
{}
