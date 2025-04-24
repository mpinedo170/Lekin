#pragma once

class TOperationArray;
class TJobArray;

const double BigM = 9999999;
const double Small = 0.000001;
const double pi = 3.14159265358;

bool equals(LPCTSTR str1, LPCTSTR str2);

void CheckInt(int val, int mn = 0, int mx = 9999);
void CheckID(LPCTSTR s, UINT ids2);
void CheckStatus(LPCTSTR s);

void ThrowNotExist(LPCTSTR name, UINT ids2);
void ThrowBadOp(LPCTSTR name, UINT ids2);
void ThrowDupID(LPCTSTR s, UINT ids2);
void ThrowTooMany(int mx, UINT ids2);

void ReportDupID(CDataExchange* pDX, UINT idc, UINT ids2);
void ReportTooMany(int mx, UINT ids2);

bool IsObjMax(TObjective obj);
bool IsObjSum(TObjective obj);
bool IsObjWeighted(TObjective obj);

// multiprocessing
bool CheckOtherInstance();
enum
{
    mpSpecial = 1,
    mpDetached = 2,
    mpHide = 4
};
bool MakeProcess(LPCTSTR command, LPCTSTR dir, int mpFlags, HANDLE& hProcess, HANDLE& hThread);

// invalidation
bool InvalidateJobs(TJobArray& arrJobsToInvalidate, TOperationArray& arrOpOrphan);
bool InvalidateAll();
void DeleteEmptyJobs();

// misc
CString GetVersionInfo(int count);
void LekinBeep(bool bOk = false);
bool CheckHack();
