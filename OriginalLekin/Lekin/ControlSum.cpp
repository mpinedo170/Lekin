#define _WIN32_WINNT 0x0502  // Win NT version (XP SP1)
#define _WIN32_IE 0x0601     // Internet Explorer 6.0 SP1

#include <afxwin.h>

#include "ControlSum.h"

static TControlData data = {"Hi, it's me, LEKIN! How are ya?"};

void ControlSum(CFile& F, LPWORD& pBuffer, TControlInfo* pInfo, bool bReplace)
{
    int size = int(F.GetLength() - F.GetPosition());
    int sizeWords = (size + sizeof(WORD) - 1) / sizeof(WORD);

    pBuffer = new WORD[sizeWords];
    if (pBuffer == NULL) throw new CFileException();

    pBuffer[sizeWords - 1] = 0;
    if (F.Read(pBuffer, size) != DWORD(size)) throw new CFileException();

    int control = 0;
    int sizeTest = sizeof(data.m_signature) / sizeof(WORD);

    for (int i = 0; i < sizeWords;)
    {
        if (pInfo == NULL && i < sizeWords - sizeTest &&
            memcmp(&pBuffer[i], &data, sizeTest * sizeof(WORD)) == 0)
        {
            i += sizeTest;
            pInfo = (TControlInfo*)&pBuffer[i];
            i += sizeof(TControlInfo) / sizeof(WORD);
        }
        else
        {
            control += pBuffer[i];
            ++i;
        }
    }

    if (pInfo == NULL) throw new CFileException();

    if (bReplace)
    {
        pInfo->m_size = size;
        pInfo->m_control = control;
    }
    else
    {
        if (pInfo->m_size != size || pInfo->m_control != control) throw new CFileException();
    }
}
