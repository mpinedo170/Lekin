// Input.cxx:
// In this file, we randomly generate the instances for testing the algorithm.
// Build up the initial disjunctive graph based on the generated data and also
// fill in the data of the job infos.

#include "StdAfx.h"

#include "Def.h"
#include "Input.h"

extern FILE* fp;

// We read the test instances from the data file which is in the
// uniform format
void Data::InitializeMachine(char* FileName)
{
    int i;
    char word[80];

    NumStages = 0;
    NumJobs = 0;
    machines = new int[MAX_STAGES];
    for (i = 0; i < MAX_STAGES; i++) machines[i] = 0;

    ifstream InputFile(FileName, ios::in);

    if (!InputFile)
    {
        cerr << "Input File could not be opened" << endl;
        exit(1);
    }

    // read in the data line by line from the file

    InputFile >> word;
    /* if ( _tcscmp(word, "Flexible") != 0){
             cout << "Not a Flexible Flow Shop!" << endl;
             exit(1);
} */
    while (1)
    {
        while (!InputFile.eof() && _tcscmp(word, "Workcenter:") != 0 && _tcscmp(word, "Machine:") != 0)
            InputFile >> word;
        if (!_tcscmp(word, "Workcenter:"))
        {
            NumStages++;
            InputFile >> word;  // workcenter name
            InputFile >> word;  // read in next word
        }
        else if (!_tcscmp(word, "Machine:"))
        {
            machines[NumStages - 1]++;
            InputFile >> word;  // machine name
            InputFile >> word;  // read in next word
        }
        else if (InputFile.eof())
            return;
    }
    if (NumJobs == 0)
    {
        cout << "0 jobs! \n";
        exit(1);
    }
    if (NumStages == 0)
    {
        cout << "0 stages! \n";
        exit(1);
    }
    for (i = 0; i < NumStages; i++)
        if (machines[i] == 0)
        {
            cout << " 0 machine at stage " << i << endl;
            exit(1);
        }
    InputFile.close();
}

void Data::InitializeJob(char* FileName)
{
    int j, l;
    char word[80];
    char ww[80];

    ReleaseTime = new int[31];
    DueTime = new int[31];
    for (j = 0; j < 31; j++) DueTime[j] = 0;
    weight = new int[31];
    ProcessTime = new int*[31];
    for (j = 0; j < 31; j++) ProcessTime[j] = new int[NumStages];

    ifstream InputFile(FileName, ios::in);

    if (!InputFile)
    {
        cerr << "Input File could not be opened" << endl;
        exit(1);
    }

    InputFile >> word >> word;
    if (_tcscmp(word, "Flow") != 0)
    {
        cout << "Not a Flexible Flow Shop!" << endl;
        exit(1);
    }

    while (!InputFile.eof())
    {
        while (_tcscmp(word, "Job:") != 0) InputFile >> word;
        NumJobs++;
        InputFile >> word;
        InputFile >> word >> ReleaseTime[NumJobs - 1];
        InputFile >> word >> DueTime[NumJobs - 1];
        InputFile >> word >> weight[NumJobs - 1];
        while (_tcscmp(word, "Oper:") != 0) InputFile >> word;
        for (l = 0; l < NumStages; l++)
        {
            InputFile >> word;
            // parse word to abstract processing time
            int k = 0;
            while (word[k] != '\0' && word[k] != ';') k++;
            if (word[k] == ';')
            {
                k++;
                int ii = 0;
                ww[ii] = word[k];
                k++;
                while (word[k] != '\0' && word[k] != ';')
                {
                    ii++;
                    ww[ii] = word[k];
                    k++;
                }
                ww[ii + 1] = '\0';
                int proc = atoi(ww);
                if (proc <= 0)
                {
                    printf("Zero processing times are not supported.\n");
                    exit(1);
                }
                ProcessTime[NumJobs - 1][l] = proc;
            }
            InputFile >> word;
        }
    }
    InputFile.close();
}

// constructor for Data object
Data::Data()
{}

// destructor for Data object
Data::~Data()
{
    if (machines) delete[] machines;
    if (ReleaseTime) delete[] ReleaseTime;
    if (DueTime) delete[] DueTime;
    if (weight) delete[] weight;

    for (int i = 0; i < NumJobs; i++)
        if (ProcessTime[i]) delete[] ProcessTime[i];
    delete[] ProcessTime;
}
