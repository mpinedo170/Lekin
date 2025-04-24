#include "StdAfx.h"

#include "..\Basic.h"
#include "Parameter.h"

CUIntArray Awkc, Amch, Aop;
int jobID = 1;

static LPCSTR ID(int id)
{
    static CStringA s;
    s.Format("%05d", id);
    return s;
}

bool ConvertWkc(ostream& os)
{
    Awkc.Add(1);
    Amch.Add(0);
    Aop.Add(0);

    bool bResult = false;
    CFile F1("_user.mch", CFile::modeRead);
    CArchive ar(&F1, CArchive::load);

    CString token, data;

    ReadNext(ar, token, data);  // Ordinary / Flexible
    ReadNext(ar, token, data);
    while (token == "Workcenter")
    {
        int wkcID = Awkc.GetSize();
        os << "Workcenter: " << ID(wkcID) << " ()\n";
        os << "  task:  \n";
        os << "Machines\n";

        CString setup;
        ReadNext(ar, token, data);  // Setup?
        if (token == "Setup")
        {
            LPCSTR ss = data;
            while (true)
            {
                CString x = NextToken(ss, ';');
                if (x.IsEmpty()) break;
                CString y = NextToken(ss, ';');
                CString z = NextToken(ss, ' ');
                setup += "        (" + x + "," + y + ")  : " + z + "\n";
                bResult = true;
            }
        }

        ReadNext(ar, token, data);  // Machine
        while (token == "Machine")
        {
            int mchID = Amch.Add(wkcID);
            if (mchID > iMaxMachine)
            {
                CString msg;
                msg.Format("Shifting Bottleneck supports only %d machines", iMaxMachine);
                AfxMessageBox(msg);
                exit(1);
            }

            os << "#" << ID(mchID) << " -- \n";
            os << "  batch size   : 1\n";
            os << "  speed        : 1\n";
            os << "  task         : \n";

            ReadNext(ar, token, data);  // Release
            CString temp = data;
            ReadNext(ar, token, data);  // Status
            os << "  init status  : " << data << "\n";
            os << "  avail. time  : " << temp << "\n";
            os << "  user         : \n";
            os << "  setup default: 0\n";
            os << setup << ";\n";

            do ReadNext(ar, token, data);
            while (!token.IsEmpty() && token != "Machine" && token != "Workcenter");
        }
        Awkc.Add(Amch.GetSize());
        os << "\n";
    }
    return bResult;
}

void ConvertJob(ostream& os)
{
    CFile F1("_user.job", CFile::modeRead);
    CArchive ar(&F1, CArchive::load);

    CString token, data;

    ReadNext(ar, token, data);  // Shop
    ReadNext(ar, token, data);
    while (token == "Job")
    {
        CString job = data;
        CString prior = "-";

        os << "Job: " << ID(jobID) << " ()\n";
        ReadNext(ar, token, data);  // Release
        os << "  release : " << data << "\n";
        ReadNext(ar, token, data);  // Due
        os << "  duedate : " << data << "\n";
        ReadNext(ar, token, data);  // Weight
        os << "  weight  : " << data << "/0\n";
        os << "Operations\n";

        ReadNext(ar, token, data);  // Oper
        while (token == "Oper")
        {
            LPCSTR ss = data;
            int opID = Aop.Add(jobID);
            os << "#" << ID(opID) << " -- \n";

            int wkcID, proc;
            TCHAR stat;
            if (_stscanf(data, "Wkc%d;%d;%c", &wkcID, &proc, &stat) < 3) return;

            os << "  process time   : " << proc << "\n";
            os << "  release time   : -\n";
            os << "  prior operation: " << prior << "\n";
            os << "  process machine: ";
            for (int i = Awkc[wkcID];; i++)
            {
                os << i;
                if (i >= int(Awkc[wkcID + 1]) - 1) break;
                os << ", ";
            }
            os << "\n  machine status : " << stat << "\n";
            os << "  user           : 0,0,9208370,1\n";
            prior = IntToStr(opID);
            ReadNext(ar, token, data);
        }
        jobID++;
        os << "\n";
    }
}

void ConvertSeq(istream& is, LPCSTR head)
{
    CFile F1("_user.seq", CFile::modeWrite | CFile::modeCreate);
    CArchive ar(&F1, CArchive::store);

    WriteToken(ar, 0, "Schedule", CString("General SB Routine / ") + head);
    while (true)
    {
        CHAR temp[1024];
        is.getline(temp, 1024);
        CString temp2(temp);
        LPCTSTR ss = temp;
        CString tok = NextToken(ss, chrSpace);
        if (tok != "Machine") break;

        int mchID = StrToInt(NextToken(ss, ':'));
        if (mchID < 0) break;

        int wkcID = Amch[mchID] - 1;
        WriteToken(ar, 2, "Machine", IntToStr(wkcID) + "." + IntToStr(mchID - Awkc[wkcID]));
        while (ss)
        {
            int opID = StrToInt(NextToken(ss, ','));
            WriteToken(ar, 4, "Oper", IntToStr(Aop[opID] - 1));
        }
    }
}
