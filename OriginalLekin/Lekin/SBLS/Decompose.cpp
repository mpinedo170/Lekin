#include "StdAfx.h"

#include "Input.h"
#include "StageBB.h"
#include "def.h"

extern time_t BeginTime;
extern int GlobalValue;

int pos(int s);
void sb_ls();
int Metropolis_Test(int, int);
void sb_ls_partial(int);

// Pass the input data to stephan's local search program
// NOTE: all his program starts from 1 instead of from 0 as in mine
void Pass_To_sbls(const Data& I)
{
    STAGES = I.NumStages;
    JOB_NR = I.NumJobs;
    int i, j;

    for (i = 0; i < I.NumStages; i++) MACHINES_ON_STAGE[i + 1] = I.machines[i];

    for (j = 0; j < I.NumJobs; j++)
    {
        JOBS[j + 1].Release_Date = I.ReleaseTime[j];
        JOBS[j + 1].Due_Date = I.DueTime[j];
        JOBS[j + 1].Weight = I.weight[j];

        for (i = 0; i < I.NumStages; i++) JOBS[j + 1].Processing_Times[i + 1] = I.ProcessTime[j][i];
    }
}

void Save(const Data& I, PartialSchedule** BestSchedule)
{
    FILE* foutput;
    if ((foutput = fopen("_user.seq", "w")) == NULL)
    {
        cout << "_user.seq file open error";
        exit(1);
    }

    fprintf(foutput, "Schedule:        SB-LS / sum(wT)\n");

    for (int l = 0; l < I.NumStages; l++)
    {
        for (int i = 0; i < I.machines[l]; i++)
        {
            fprintf(foutput, "   Machine:        Wkc%03d.%03d\n", l, i);
            for (int j = 0; j < BestSchedule[l][i].length; j++)
                fprintf(foutput, "      Oper:        Job%03d\n", BestSchedule[l][i].sequence[j]);
        }
    }
    fclose(foutput);
}

void Decomposition_Heuristic(const Data& I)
{
    bool bSaved = false;

    int ThisStage;
    PartialSchedule** CompleteSchedule;
    PartialSchedule** BestSchedule;
    int* thesize;  // record the size of the final schedule of every stage
    int* BestSize;
    int BestValue = INFINITY;
    int i, j;
    int OptValue;
    int IsScheduled[MAXSTAGE];  // flag a stage is scheduled or not
    int ActualBList[MAXSTAGE];  // record the actual bottleneck sequence scheduled
    PartialSchedule* ThisSchedule;

    for (i = 0; i < I.NumStages; i++) IsScheduled[i] = FALSE;

    ThisSchedule = new PartialSchedule[MAXMACHINE];
    int ThisSize;

    Graph disgraphob(I.NumStages, I.NumJobs);
    Graph* disgraph = &disgraphob;
    disgraph->Buildup_Graph(I);

    CompleteSchedule = new PartialSchedule*[I.NumStages];
    for (i = 0; i < I.NumStages; i++) CompleteSchedule[i] = new PartialSchedule[MAXMACHINE];

    thesize = new int[I.NumStages];
    BestSchedule = new PartialSchedule*[I.NumStages];
    for (i = 0; i < I.NumStages; i++) BestSchedule[i] = new PartialSchedule[MAXMACHINE];

    BestSize = new int[I.NumStages];

    disgraph->Detect_Bottleneck_Stage();

    Pass_To_sbls(I);

    ParallelMachine* Stage = NULL;

    int iteration, cycle, cl, Cycle;

    for (iteration = 0; iteration < I.NumStages; iteration++)
    {
        int Biggest = -INFINITY;
        for (int iter = 0; iter < I.NumStages; iter++)
        {
            if (!IsScheduled[iter])
            {
                ThisStage = iter;
                for (i = 0; i < I.NumJobs; i++) disgraph->Cal_Completion_Time(i);

                for (i = 0; i < I.NumJobs; i++)
                {
                    disgraph->Cal_Release_Time(ThisStage, i);
                    disgraph->Cal_Due_Dates(ThisStage, i);
                }
                if (Stage) delete Stage;
                Stage = new ParallelMachine(ThisStage, disgraph);
                Stage->Solve_The_Stage();

                if (Stage->OptValue > Biggest)
                {
                    Biggest = Stage->OptValue;
                    ActualBList[iteration] = iter;
                    ThisSize = Stage->OptSize;
                    for (int kk = 0; kk < Stage->OptSize; kk++) ThisSchedule[kk] = Stage->OptSchedule[kk];
                }
                // disgraph->Reset(iter);
            }
        }

        IsScheduled[ActualBList[iteration]] = TRUE;
        ThisStage = ActualBList[iteration];

        disgraph->Fix_A_Stage(disgraph->graph[ThisStage], ThisSchedule, ThisSize);

        for (i = 0; i < ThisSize; i++)
        {
            CompleteSchedule[ThisStage][i] = ThisSchedule[i];
            BestSchedule[ThisStage][i] = ThisSchedule[i];
        }

        thesize[ThisStage] = ThisSize;
        BestSize[ThisStage] = ThisSize;

        // ReSchedule processes
        // for every reschedule, we do 3 cycles

        for (cycle = 0; cycle < 10; cycle++)
        {
            for (i = 0; i <= iteration; i++)
            {
                disgraph->Reset(ActualBList[i]);
                for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Completion_Time(j);
                for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Release_Time(ActualBList[i], j);
                for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Due_Dates(ActualBList[i], j);

                if (Stage) delete Stage;
                Stage = new ParallelMachine(ActualBList[i], disgraph);
                // Stage->Reschedule_Process();
                Stage->Solve_The_Stage();
                disgraph->Fix_A_Stage(disgraph->graph[ActualBList[i]], Stage->OptSchedule, Stage->OptSize);

                for (j = 0; j < Stage->OptSize; j++)
                {
                    CompleteSchedule[ActualBList[i]][j] = Stage->OptSchedule[j];
                    thesize[ActualBList[i]] = Stage->OptSize;
                    BestSchedule[ActualBList[i]][j] = Stage->OptSchedule[j];
                    BestSize[ActualBList[i]] = Stage->OptSize;
                }
            }  // for  reschedule process
        }      // for 3 cycles
    }          // for iterations

    // Now Calculate the initial objective value for the whole flexible flow shop
    OptValue = 0;
    for (j = 0; j < I.NumJobs; j++)
    {
        disgraph->Cal_Completion_Time(j);
        OptValue +=
            disgraph->jobs[j].weight * pos(disgraph->jobs[j].CompleteTime - disgraph->jobs[j].DueTime);
    }
    BestValue = OptValue;

    // global reoptimization cycles of the SB procedure
    // reschedule the stages in the order of the bottleneck sequence
    for (cl = 0; cl < I.NumStages * 5; cl++)
    {
        i = cl % I.NumStages;
        disgraph->Reset(ActualBList[i]);

        for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Completion_Time(j);
        for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Release_Time(ActualBList[i], j);
        for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Due_Dates(ActualBList[i], j);

        if (Stage) delete Stage;
        Stage = new ParallelMachine(ActualBList[i], disgraph);
        // Stage->Reschedule_Process();
        Stage->Solve_The_Stage();

        disgraph->Fix_A_Stage(disgraph->graph[ActualBList[i]], Stage->OptSchedule, Stage->OptSize);

        // calculate if the new result is better than OptValue
        int TempValue = 0;
        for (j = 0; j < I.NumJobs; j++)
        {
            disgraph->Cal_Completion_Time(j);
            TempValue +=
                disgraph->jobs[j].weight * pos(disgraph->jobs[j].CompleteTime - disgraph->jobs[j].DueTime);
        }

        if (TempValue < OptValue && TempValue < BestValue)
        {  // put in the new schedule for this stage
            OptValue = BestValue = TempValue;
            for (j = 0; j < Stage->OptSize; j++)
            {
                CompleteSchedule[ActualBList[i]][j] = Stage->OptSchedule[j];
                BestSchedule[ActualBList[i]][j] = Stage->OptSchedule[j];
                thesize[ActualBList[i]] = Stage->OptSize;
                BestSize[ActualBList[i]] = Stage->OptSize;
            }
        }
        else
        {  // do the metropolis test
            disgraph->Reset(ActualBList[i]);
            disgraph->Fix_A_Stage(
                disgraph->graph[ActualBList[i]], CompleteSchedule[ActualBList[i]], thesize[ActualBList[i]]);
        }
    }  // for cl

    time_t SBTime = clock();
    int TimeOfSeconds = (int)((double)(SBTime - BeginTime) / CLOCKS_PER_SEC);

    // cout << "SB time is "<<TimeOfSeconds<<" sec. \n";
    cout << "Initial Value = " << OptValue << endl;

    if (!bSaved)
    {
        Save(I, BestSchedule);
        bSaved = true;
    }

    if (OptValue == 0)
    {
        cout << "0 tardiness. Optimal!" << endl;

        // release memory
        for (i = 0; i < I.NumStages; i++)
            if (CompleteSchedule[i]) delete[] CompleteSchedule[i];
        if (CompleteSchedule) delete[] CompleteSchedule;

        if (thesize) delete[] thesize;

        for (i = 0; i < I.NumStages; i++)
            if (BestSchedule[i]) delete[] BestSchedule[i];
        if (BestSchedule) delete[] BestSchedule;

        if (BestSize) delete[] BestSize;
        // if (disgraph) delete disgraph;
        if (Stage) delete Stage;

        return;  // no need to run SBLS anymore
    }

    // we start 5 iterations of SB and LS, to try to find better schedule

    for (Cycle = 0; Cycle < 20; Cycle++)
    {
        // Now that we have the initial starting point for the flexible flow shop
        // Connect to stephan's global local search program to get further improvement

        int ll = 0;
        for (i = 0; i < I.NumStages; i++)
        {
            for (j = 0; j < thesize[i]; j++)
            {
                for (ll = 0; ll < CompleteSchedule[i][j].length; ll++)
                    MACHINES[i + 1][j + 1].Best_Job_Order[ll + 1] = CompleteSchedule[i][j].sequence[ll] + 1;

                MACHINES[i + 1][j + 1].Best_Job_Order[ll + 1] = 0;
            }
        }

        // call the global local search function
        BEST_RESULT = BestValue;
        ACCEPT = BAD_ITER = 0;

        // cout << "Before = " << BestValue << endl;

        sb_ls();

        // cout << "After = " << BEST_RESULT << endl;
        if (BEST_RESULT == 0) break;

        // get back the schedule from the local search
        for (i = 0; i < I.NumStages; i++)
        {
            for (j = 0; j < thesize[i]; j++)
            {
                CompleteSchedule[i][j].length = 0;
                for (ll = 0; MACHINES[i + 1][j + 1].Best_Job_Order[ll + 1] != 0; ll++)
                {
                    CompleteSchedule[i][j].sequence[ll] = MACHINES[i + 1][j + 1].Best_Job_Order[ll + 1] - 1;
                    CompleteSchedule[i][j].length++;
                }
            }
        }

        // If this is the so far best schedule, then, store it to BestSchedule
        if (BEST_RESULT < BestValue)
        {
            BestValue = BEST_RESULT;
            for (i = 0; i < I.NumStages; i++)
            {
                for (j = 0; j < thesize[i]; j++) BestSchedule[i][j] = CompleteSchedule[i][j];

                BestSize[i] = thesize[i];
            }

            cout << " New Value = " << BestValue << endl;
        }

        // have to change disgraph accordingly since now we
        // have a new schedule for the flow shop after the local search
        for (i = 0; i < I.NumStages; i++)
        {
            disgraph->Reset(i);
            disgraph->Fix_A_Stage(disgraph->graph[i], CompleteSchedule[i], thesize[i]);
        }

        // reschedule each stage according to the bottleneck stage order
        // for (cl=0; cl<I.NumStages; cl++){
        // i = cl;

        for (cl = 0; cl < I.NumStages * 5; cl++)
        {
            i = cl % I.NumStages;
            disgraph->Reset(ActualBList[i]);

            for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Completion_Time(j);
            for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Release_Time(ActualBList[i], j);
            for (j = 0; j < I.NumJobs; j++) disgraph->Cal_Due_Dates(ActualBList[i], j);

            disgraph->Fix_A_Stage(
                disgraph->graph[ActualBList[i]], CompleteSchedule[ActualBList[i]], thesize[ActualBList[i]]);

            if (Stage) delete Stage;
            Stage = new ParallelMachine(ActualBList[i], disgraph);

            Stage->Solve_The_Stage();
            // Stage->Reschedule_Process(); //no cal_initial_schedule here

            disgraph->Reset(ActualBList[i]);
            disgraph->Fix_A_Stage(disgraph->graph[ActualBList[i]], Stage->OptSchedule, Stage->OptSize);

            // calculate if the new result is better than OptValue
            int TempValue = 0;
            for (j = 0; j < I.NumJobs; j++)
            {
                disgraph->Cal_Completion_Time(j);
                TempValue += disgraph->jobs[j].weight *
                             pos(disgraph->jobs[j].CompleteTime - disgraph->jobs[j].DueTime);
            }

            if (TempValue < BestValue)
            {  // put in the new schedule for this stage

                BestValue = OptValue = TempValue;
                for (j = 0; j < Stage->OptSize; j++)
                {
                    CompleteSchedule[ActualBList[i]][j] = Stage->OptSchedule[j];
                    BestSchedule[ActualBList[i]][j] = Stage->OptSchedule[j];
                    thesize[ActualBList[i]] = Stage->OptSize;
                    BestSize[ActualBList[i]] = Stage->OptSize;
                }
            }
            else
            {
                disgraph->Reset(ActualBList[i]);
                disgraph->Fix_A_Stage(disgraph->graph[ActualBList[i]], CompleteSchedule[ActualBList[i]],
                    thesize[ActualBList[i]]);
            }
        }  // for cl

    }  // the iterations of SB and LS

    cout << "\n\nTotal Weighted Tardiness =    " << BestValue;

    if (BestValue < GlobalValue)
    {
        GlobalValue = BestValue;

        // write BestSchedule in the output file "_user.seq"
        Save(I, BestSchedule);
    }

    // release memory
    for (i = 0; i < I.NumStages; i++)
        if (CompleteSchedule[i]) delete[] CompleteSchedule[i];

    if (CompleteSchedule) delete[] CompleteSchedule;

    if (thesize) delete[] thesize;

    for (i = 0; i < I.NumStages; i++)
        if (BestSchedule[i]) delete[] BestSchedule[i];

    if (BestSchedule) delete[] BestSchedule;

    if (BestSize) delete[] BestSize;

    // if (disgraph) delete disgraph;
    if (Stage) delete Stage;
}
