#include "StdAfx.h"

#include "Constants.h"
#include "Input.h"

FILE* fp;   // result file with schedules
FILE* fp2;  // result only has time and value

extern void Randomize();

void Int_To_String(int i, char* string)
{
    if (i >= 0 && i < 10)
    {
        string[0] = '0' + i;
        string[1] = '\0';
    }
    else if (i < 100)
    {
        string[0] = '0' + (i / 10);
        string[1] = '0' + (i % 10);
        string[2] = '\0';
    }
    else if (i < 1000)
    {
        string[0] = '0' + (i / 100);
        string[1] = '0' + (i / 10 % 10);
        string[2] = '0' + (i % 10);
        string[3] = '\0';
    }
}

void Decomposition_Heuristic(const Data&);
//#define CLOCKS_PER_SEC 1000000.0

void Input_Files(int, int);

char InFiles[90][32];

void Input_Files(int from, int to)
{
    int i, j;
    char name[30], number[5];

    j = 0;
    for (i = from; i <= to; i++)
    {
        _tcscpy(name, "../AllData/BBData/");
        Int_To_String(i, number);

        strcat(name, number);
        _tcscpy(InFiles[j], name);
        j++;
    }
}

time_t BeginTime;

// in the current lekin system,
// max number of workcenters is 10;
// max number of machines at each workcenter is 5;
// max number of jobs is 30;
// max number of characters in a name is 30;

void GetMaxValues(int& MaxJob, int& MaxWkc, int& MaxMch);
int GlobalValue = INFINITY;

// Now we suppose that NumStages,NumJobs and fpar are command line arguments
// read in by main.
int main(int argc, char* argv[])
{
    Randomize();

    int i1, i2, i3;
    GetMaxValues(i1, i2, i3);

    Data instance;

    if ((fp = fopen("_user.mch", "r")) == NULL)
    {
        cout << "_user.mch file open error";
        exit(1);
    }
    instance.InitializeMachine("_user.mch");
    fclose(fp);

    if ((fp = fopen("_user.job", "r")) == NULL)
    {
        cout << "_user.job file open error";
        exit(1);
    }
    instance.InitializeJob("_user.job");
    fclose(fp);

    BeginTime = clock();

    int run;

    cout << "Number of Times to Run (Choose 1 ~ 5) ";
    cin >> run;

    cout << "You Get the Best Solution from " << run << " Run(s) " << endl;

    if (run < 1 || run > 5)
    {
        cout << "\n 1 ~ 5 Runs ONLY" << endl;
        exit(1);
    }

    for (int ii = 1; ii <= run; ii++)
    {
        switch (ii)
        {
            case 1:
                cout << "\n First Run: \n \n";
                break;
            case 2:
                cout << "\n \n Second Run: \n \n";
                break;
            case 3:
                cout << "\n \n Third Run: \n \n";
                break;
            case 4:
                cout << "\n \n Fourth Run: \n \n";
                break;
            case 5:
                cout << "\n \n Fifth Run: \n \n";
                break;
        }
        Decomposition_Heuristic(instance);
    }

    cout << "\nBest Solution Value in " << run << " Run(s) = " << GlobalValue << endl;
    time_t EndTime = clock();
    int TimeOfSeconds = (int)((double)(EndTime - BeginTime) / CLOCKS_PER_SEC);

    cout << "\n Total Running Time = " << TimeOfSeconds << " sec. \n";
    return 0;
}
