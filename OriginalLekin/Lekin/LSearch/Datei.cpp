/************************************************************************/
/*                                                                      */
/*                                                                      */
/*        Prozeduren zum Einlesen und Abspeichern von Dateien           */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "Datei.h"

#include "Def.h"

/*****************          Rahmenbedingungen           *****************/

// Lese Auftragsdaten aus de Datei in einen Array.
void ReadMachineFile()
{
    FILE* file;
    char line[300], text[100], c;
    int line_pos, text_pos, mistake;

    // Datei oeffnen
    if ((file = fopen("_user.mch", "r")) == NULL)
    {
        printf("Can't open file _user.mch!!!\n");
        exit(1);
    }

    text_pos = 0;
    while ((c = fgetc(file)) != '\n')
    {
        text[text_pos] = c;
        ++text_pos;
    }
    text[text_pos] = '\0';

    if (_tcscmp(text, "Ordinary:") != 0)
    {
        printf("Local Search doesn't handle flexible machine environments!!!!\n");
        exit(1);
    }

    while (fgets(line, 100, file) != NULL)
    {
        line_pos = 0;
        text_pos = 0;
        c = line[line_pos];

        if (c != ' ')
        {
            text[text_pos] = c;
            ++text_pos;
        }
        ++line_pos;

        while ((c = line[line_pos]) != ':')
        {
            if (c != ' ')
            {
                text[text_pos] = c;
                ++text_pos;
            }
            ++line_pos;
        }
        text[text_pos] = '\0';

        if (_tcscmp(text, "Workcenter") == 0)
            MACHINES.Add(new TMachine);
        else if (_tcscmp(text, "Release:") == 0)
        {
            text_pos = 0;
            while ((c = line[line_pos]) != '\n')
            {
                if ((c != ' ') && (c != ':'))
                {
                    text[text_pos] = c;
                    ++text_pos;
                }
                ++line_pos;
                c = line[line_pos];
            }
            text[text_pos] = '\0';
            if (!MACHINES.IsEmpty()) MACHINES[MACHINES.GetSize() - 1]->m_release = atoi(text);
        }
    }

    // Datei schlieáen
    if ((mistake = fclose(file)) != 0)
    {
        printf("Can't close file!!\n");
        exit(1);
    }
}

void ReadJobFile()
{
    FILE* file;
    char line[300], text[100], c;
    int text_pos, line_pos, mistake;

    // Datei oeffnen
    if ((file = fopen("_user.job", "r")) == NULL)
    {
        printf("Can't open file _user.job!!\n");
        exit(1);
    }

    while ((c = fgetc(file)) != ':')
        ;
    while ((c = fgetc(file)) == ' ')
        ;

    text_pos = 0;
    while (c != '\n')
    {
        text[text_pos] = c;
        ++text_pos;
        c = fgetc(file);
    }
    text[text_pos] = '\0';

    // Verify correct machine environment
    if ((_tcscmp(text, "Flow") != 0) && (_tcscmp(text, "Job") != 0))
    {
        printf("Not a flow or a job shop environment!!\nStop program!!\n");
        exit(1);
    }

    while (fgets(line, 100, file) != NULL)
    {
        if (_tcscmp(line, "\n") == 0) continue;

        line_pos = 0;
        text_pos = 0;
        c = line[line_pos];

        while (c != ':')
        {
            if (c != ' ')
            {
                text[text_pos] = c;
                ++text_pos;
            }
            ++line_pos;
            c = line[line_pos];
        }
        text[text_pos] = '\0';

        // read in job name
        if (_tcscmp(text, "Job") == 0)
            JOBS.Add(new TJob);
        else if (_tcscmp(text, "Release") == 0)
        {
            text_pos = 0;
            while ((c = line[line_pos]) != '\n')
            {
                if ((c != ' ') && (c != ':'))
                {
                    text[text_pos] = c;
                    ++text_pos;
                }
                ++line_pos;
                c = line[line_pos];
            }
            text[text_pos] = '\0';
            if (!JOBS.IsEmpty()) JOBS[JOBS.GetSize() - 1]->m_release = atoi(text);
        }
        else if (_tcscmp(text, "Due") == 0)
        {
            text_pos = 0;
            while ((c = line[line_pos]) != '\n')
            {
                if ((c != ' ') && (c != ':'))
                {
                    text[text_pos] = c;
                    ++text_pos;
                }
                ++line_pos;
                c = line[line_pos];
            }
            text[text_pos] = '\0';
            if (!JOBS.IsEmpty()) JOBS[JOBS.GetSize() - 1]->m_due = atoi(text);
        }
        else if (_tcscmp(text, "Weight") == 0)
        {
            text_pos = 0;
            while ((c = line[line_pos]) != '\n')
            {
                if ((c != ' ') && (c != ':'))
                {
                    text[text_pos] = c;
                    ++text_pos;
                }
                ++line_pos;
                c = line[line_pos];
            }
            text[text_pos] = '\0';
            if (!JOBS.IsEmpty()) JOBS[JOBS.GetSize() - 1]->m_weight = atoi(text);
        }
        else if (_tcscmp(text, "Oper") == 0)
        {
            TNetWorkingPlanNode planNode;

            c = line[line_pos];
            text_pos = 0;

            // read in machine number
            while (c != ';')
            {
                if ((c != ' ') && (c != ':'))
                {
                    text[text_pos] = c;
                    ++text_pos;
                }
                ++line_pos;
                c = line[line_pos];
            }
            text[text_pos] = '\0';

            // look for corresponding machine number
            _stscanf(text + 3, "%d", &planNode.m_machineID);

            // read in processing time
            text_pos = 0;
            ++line_pos;
            c = line[line_pos];
            while (c != ';')
            {
                if ((c != ' ') && (c != ':'))
                {
                    text[text_pos] = c;
                    ++text_pos;
                }
                ++line_pos;
                c = line[line_pos];
            }
            text[text_pos] = '\0';
            planNode.m_processingTime = atoi(text);

            while ((c = line[line_pos]) != '\n') ++line_pos;

            if (!JOBS.IsEmpty())
            {
                TJob* pJob = JOBS[JOBS.GetSize() - 1];
                pJob->m_plan.Add(planNode);
            }
        }
    }

    int MaxJob, MaxWkc, MaxMch;
    GetMaxValues(MaxJob, MaxWkc, MaxMch);

    if (MACHINES.GetSize() == 1)
    {
        printf("Only one machine -> Single machine problem!!\nStop Program!!\n");
        exit(1);
    }

    if (MACHINES.GetSize() > MaxWkc)
    {
        printf("Too many machines (max. number = %d)!!\nStop Program!!\n", MaxWkc);
        exit(1);
    }

    if (JOBS.GetSize() < 1)
    {
        printf("Only one job!!\nStop Program!!\n");
        exit(1);
    }

    if (JOBS.GetSize() > MaxJob)
    {
        printf("Too many jobs (max. number = %d)!!\nStop Program!!\n", MaxJob);
        exit(1);
    }

    // Datei schlieáen
    if ((mistake = fclose(file)) != 0)
    {
        printf("Can't close file _user.job!!\n");
        exit(1);
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Schreibe Ablaufplan in Datei "user.seq".                    */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void WriteScheduleFile()
{
    FILE* file;
    int mistake, machine_nr, job_nr, op_nr;

    // Datei oeffnen
    if ((file = fopen("_user.seq", "w")) == NULL)
    {
        printf("Error with file _user.seq!!\nExit program!!\n");
        exit(1);
    }

    fputs("Schedule: Local Search / ", file);
    switch (OBJECTIVE_FUNCTION)
    {
        case W_T:
            fputs("sum(wT)\n", file);
            break;
        case W_C:
            fputs("sum(wC)\n", file);
            break;
        case S_T:
            fputs("sum(T)\n", file);
            break;
        case S_C:
            fputs("sum(C)\n", file);
            break;
        case M_S:
            fputs("Cmax\n", file);
            break;
    }

    for (machine_nr = 0; machine_nr < MACHINES.GetSize(); ++machine_nr)
    {
        TMachine* pMachine = MACHINES[machine_nr];
        fprintf(file, "Machine: Wkc%03d\n", machine_nr);

        for (op_nr = 0; op_nr < pMachine->GetOperCount(); ++op_nr)
        {
            job_nr = pMachine->m_bestJobOrder[op_nr].m_jobID;
            fprintf(file, "  Oper: Job%03d\n", job_nr);
        }
    }

    // Datei schlieáen
    if ((mistake = fclose(file)) != 0)
    {
        printf("Can't close file _user.seq\n");
        exit(1);
    }
}
