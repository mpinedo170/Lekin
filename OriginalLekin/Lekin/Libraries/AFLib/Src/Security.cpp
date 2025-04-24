#include "StdAfx.h"

#include "AFLibGlobal.h"

#include "Local.h"

using AFLib::ThrowMessage;
using AFLibPrivate::LocalAfxString;

namespace {
CSecurityDesc CreateUnsecureSecurityDesc(bool bDir)
{
    try
    {
        CDacl dacl;
        CSid sid = Sids::World();

        if (bDir)
        {
            dacl.AddAllowedAce(sid, GENERIC_ALL, OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE);
            dacl.AddAllowedAce(sid, GENERIC_ALL, CONTAINER_INHERIT_ACE);
        }
        else
            dacl.AddAllowedAce(sid, GENERIC_ALL);

        CSecurityDesc sd;
        sd.SetDacl(dacl);
        sd.MakeSelfRelative();
        return sd;
    }
    catch (const CAtlException&)
    {
        ThrowMessage(LocalAfxString(IDS_NO_EVERYONE));
    }

    return CSecurityDesc();
}
}  // namespace

namespace AFLib {
void CreateUnsecureDir(LPCTSTR dir)
{
    CString path = dir;
    AFLibIO::CheckDirectoryThrow(path, true);
    if (IsWin95()) return;

    CSecurityDesc sd = CreateUnsecureSecurityDesc(true);
    if (!SetFileSecurity(
            path, DACL_SECURITY_INFORMATION, const_cast<SECURITY_DESCRIPTOR*>(sd.GetPSECURITY_DESCRIPTOR())))
        ThrowMessage(LocalAfxString(IDS_CANNOT_SET_SECURITY, dir));
}

void CreateUnsecureRegKey(HKEY hKeyRoot, LPCTSTR path)
{
    CRegKey keyResult = OpenRegSubKey(hKeyRoot, path, true, KEY_ALL_ACCESS);
    if (IsWin95()) return;

    CSecurityDesc sd = CreateUnsecureSecurityDesc(true);
    if (keyResult.SetKeySecurity(DACL_SECURITY_INFORMATION,
            const_cast<SECURITY_DESCRIPTOR*>(sd.GetPSECURITY_DESCRIPTOR())) != ERROR_SUCCESS)
        ThrowMessage(LocalAfxString(IDS_CANNOT_SET_SECURITY, GetRootRegKeyName(hKeyRoot) + strBSlash + path));
}
}  // namespace AFLib
