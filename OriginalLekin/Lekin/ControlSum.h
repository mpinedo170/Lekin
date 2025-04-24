#pragma once

struct TControlInfo
{
    int m_size;
    int m_control;
};

struct TControlData
{
    BYTE m_signature[32];
    TControlInfo m_info;
};

void ControlSum(CFile& F, LPWORD& pBuffer, TControlInfo* pInfo, bool bReplace);
