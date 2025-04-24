#include "StdAfx.h"

#include "Input.h"
#include "StageBB.h"

extern int Position_Min(int*, int);
extern double pos(double);
extern int pos(int);
extern int in(int, const int*, int);
extern int Position_In(int, const int*, int);
extern int max(int, int);
extern int Metropolis_Test(int, int);
extern int Uniform(int, int);

// StageBB.cxx:
// Our current subproblem is Pm|rj|double sum(WjTj).

// constructor
// this should definitely contain initializations of members besides the memory allocation
// It is a friend function of class Graph
ParallelMachine::ParallelMachine(int StageIndex, const Graph* CurGraph)
{
    int i, j;

    NumMachines = CurGraph->NumMachines[StageIndex];
    NumJobs = CurGraph->NumJobs;

    jobs = new OpInfo[NumJobs];
    for (i = 0; i < NumJobs; i++)
    {
        jobs[i].stage = StageIndex;
        jobs[i].job = i;
        jobs[i].Weight = CurGraph->graph[StageIndex][i]->operation.Weight;
        jobs[i].Release = CurGraph->graph[StageIndex][i]->operation.Release;
        // jobs[i].Due = new int[NumJobs];
        for (j = 0; j < NumJobs; j++) jobs[i].Due[j] = CurGraph->graph[StageIndex][i]->operation.Due[j];
        jobs[i].Process = CurGraph->graph[StageIndex][i]->operation.Process;
    }

    RestrictedSet = new PartialSchedule[NumMachines];
    // RSetSize = 0;
    // objective = 0;

    OptSchedule = new PartialSchedule[NumMachines];
    // OptSize = 0;
    // OptValue = 0;
    OptSchedule2 = new PartialSchedule[NumMachines];
    // OptSize2 = 0;
    // OptValue2 = 0;
}

// destructor
ParallelMachine::~ParallelMachine()
{
    if (jobs) delete[] jobs;

    if (OptSchedule) delete[] OptSchedule;
    if (OptSchedule2) delete[] OptSchedule2;

    if (RestrictedSet) delete[] RestrictedSet;
}

// This function uses Extension of AATC rule to generate an initial
// feasible schedule for the parallel machine which serves as the initial
// upper bound as well as gives the initial restricted set of columns
// for the RMP.

//???Can the AATC rule gurantee active schedules? If not, do certain adjustment of the formula,
// since it is after all a dispatching rule

// If the decision time of k machines are the same, then, the k jobs with highest index can be
// scheduled once. This should be faster?

int ParallelMachine::Cal_Initial_Upperbound()
{
    int i, j, k;
    int* DecisionTime;
    double index;
    double IndexHigh;
    int* scheduled;
    int NumScheduled;
    int ThisMachine;
    double expo;
    double Pavg = 0.0;
    int ThisJob;
    int objective = 0;
    int* Has_Jobs_Num;

    scheduled = new int[NumJobs];
    for (j = 0; j < NumJobs; j++) scheduled[j] = FALSE;

    Has_Jobs_Num = new int[NumMachines];
    for (i = 0; i < NumMachines; i++) Has_Jobs_Num[i] = 0;

    DecisionTime = new int[NumMachines];
    for (i = 0; i < NumMachines; i++) DecisionTime[i] = 0;

    NumScheduled = 0;

    for (j = 0; j < NumJobs; j++) Pavg += jobs[j].Process;
    Pavg = Pavg / NumJobs;

    objective = 0;
    for (i = 0; i < NumMachines; i++) RestrictedSet[i].tardiness = 0;

    while (NumScheduled < NumJobs)
    {
        // Schedule the job with the highest index on the
        // machine with earliest available time

        IndexHigh = 0;  // reset the highest index;
        ThisMachine = Position_Min(DecisionTime, NumMachines);
        for (j = 0; j < NumJobs; j++)
        {
            if (!scheduled[j])
            {
                index = 0;
                for (k = 0; k < NumJobs; k++)
                {
                    if (jobs[j].Due[k] != INFINITY)
                    {
                        // try another way of computing "expo" get a better initial schedule or not
                        // also, the initial schedule can be enhanced by performing "test and choose" the best
                        // of the possible set of KPar to use . Ref. ~/DEC/optimize.cxx
                        expo = -pos(jobs[j].Due[k] - jobs[j].Process + jobs[j].Release -
                                    DecisionTime[ThisMachine]) /
                               KPar / Pavg;

                        // tried this one, worse than the above one
                        // if (jobs[j].Release < DecisionTime[ThisMachine])
                        //  expo = - pos(jobs[j].Due[k] - jobs[j].Process -
                        //  DecisionTime[ThisMachine])/KPar/Pavg;
                        // else
                        //  expo = - pos(jobs[j].Due[k] - jobs[j].Process - jobs[j].Release)/KPar/Pavg;

                        index += jobs[k].Weight * exp(expo) / jobs[j].Process;
                    }
                }
                if (index > IndexHigh)
                {
                    IndexHigh = index;
                    ThisJob = j;
                }
            }
        }

        // Append this job to the schedule

        RestrictedSet[ThisMachine].sequence[Has_Jobs_Num[ThisMachine]] = ThisJob;
        for (k = 0; k < NumJobs; k++)
        {
            if (jobs[ThisJob].Due[k] != INFINITY)
                RestrictedSet[ThisMachine].tardiness +=
                    jobs[ThisJob].Weight * pos(max(jobs[ThisJob].Release, DecisionTime[ThisMachine]) +
                                               jobs[ThisJob].Process - jobs[ThisJob].Due[k]);
        }

        scheduled[ThisJob] = TRUE;
        NumScheduled++;
        DecisionTime[ThisMachine] =
            max(jobs[ThisJob].Release, DecisionTime[ThisMachine]) + jobs[ThisJob].Process;
        Has_Jobs_Num[ThisMachine]++;
    }

    int size = 0;
    for (i = 0; i < NumMachines; i++)
    {
        RestrictedSet[i].length = Has_Jobs_Num[i];
        if (Has_Jobs_Num[i]) size++;
    }

    // Note it is possible that the initial schedule is the optimal schedule, so, copy it first
    OptSize = RSetSize = size;
    for (i = 0; i < OptSize; i++)
    {
        OptSchedule[i] = RestrictedSet[i];
        OptSchedule2[i] = RestrictedSet[i];
    }

    delete[] DecisionTime;
    delete[] scheduled;
    delete[] Has_Jobs_Num;

    for (i = 0; i < RSetSize; i++) objective += RestrictedSet[i].tardiness;
    OptValue = objective;

    return (objective);
}

// This function uses ATC rule to schedule the Jobs in JobSet

void ParallelMachine::Single_Machine_ATC(int* sequence, const int* JobSet, const int JobSize)
{
    int i, k;
    int t = 0;

    int NumScheduled = 0;

    double Pavg = 0.0;
    for (i = 0; i < JobSize; i++) Pavg += jobs[JobSet[i]].Process;
    Pavg /= JobSize;

    double index;
    double IndexHigh;
    double expo;
    int ThisJob;
    int* scheduled;

    scheduled = new int[NumJobs];
    for (i = 0; i < NumJobs; i++) scheduled[i] = FALSE;

    int DecisionTime = 0;
    while (NumScheduled < JobSize)
    {
        IndexHigh = 0;
        for (i = 0; i < JobSize; i++)
        {
            int j = JobSet[i];  // j is the job number

            if (!scheduled[j])
            {
                index = 0;
                for (k = 0; k < NumJobs; k++)
                {
                    if (jobs[j].Due[k] != INFINITY)
                    {
                        expo = -pos(jobs[j].Due[k] - jobs[j].Process + jobs[j].Release - DecisionTime) /
                               KPar / Pavg;

                        index += jobs[k].Weight * exp(expo) / jobs[j].Process;
                    }
                }
                if (index > IndexHigh)
                {
                    IndexHigh = index;
                    ThisJob = j;
                }
            }  // if ( ! scheduled)
        }      // for

        // Append this job to the schedule
        sequence[NumScheduled] = ThisJob;
        scheduled[ThisJob] = TRUE;
        DecisionTime = max(jobs[ThisJob].Release, DecisionTime) + jobs[ThisJob].Process;
        NumScheduled++;
    }  // while

    if (scheduled) delete[] scheduled;
}

// This function finds out index of the job with the maximum weighted tardiness in a given sequence
int ParallelMachine::Find_Max_Tardiness_Job_Index(const int* sequence, const int length)
{
    int MaxTardiness = -INFINITY;
    int MaxIndex = 0;

    int i, k;
    int start = jobs[sequence[0]].Release;
    for (i = 0; i < length; i++)
    {
        int tardiness = 0;
        for (k = 0; k < NumJobs; k++)
            if (jobs[sequence[i]].Due[k] != INFINITY)
                tardiness += jobs[sequence[i]].Weight *
                             pos(start + jobs[sequence[i]].Process - jobs[sequence[i]].Due[k]);

        if (tardiness > MaxTardiness)
        {
            MaxTardiness = tardiness;
            MaxIndex = i;
        }

        if (i < (length - 1)) start = max(start + jobs[sequence[i]].Process, jobs[sequence[i + 1]].Release);
    }
    return (MaxIndex);
}

// This function finds out index of the job with the maximum weighted tardiness in a given sequence
int ParallelMachine::Find_Second_Tardiness_Job_Index(
    const int* sequence, const int length, const int jobindex)
{
    int i, k;
    int MaxTardiness = -INFINITY;
    int MaxIndex = 0;

    int start = jobs[sequence[0]].Release;
    for (i = 0; i < length; i++)
    {
        int tardiness = 0;
        for (k = 0; k < NumJobs; k++)
            if (jobs[sequence[i]].Due[k] != INFINITY)
                tardiness += jobs[sequence[i]].Weight *
                             pos(start + jobs[sequence[i]].Process - jobs[sequence[i]].Due[k]);

        if (i == jobindex) tardiness = -1;
        if (tardiness > MaxTardiness)
        {
            MaxTardiness = tardiness;
            MaxIndex = i;
        }

        if (i < (length - 1)) start = max(start + jobs[sequence[i]].Process, jobs[sequence[i + 1]].Release);
    }
    return (MaxIndex);
}

// This function does the adjacent pairwise interchanges
// to the this single machine
int ParallelMachine::Single_Machine_Local_Search(int* sequence, int length)
{
    int i, j, k;

    int tardiness = Cal_Tardiness(sequence, length);

    int *TempSeq, *BestSeq;
    TempSeq = new int[length];
    BestSeq = new int[length];
    for (i = 0; i < length; i++)
    {
        TempSeq[i] = sequence[i];
        BestSeq[i] = sequence[i];
    }

    for (i = 0; i < (length - 1); i++)
    {
        TempSeq[i] = sequence[i + 1];
        TempSeq[i + 1] = sequence[i];

        int NewTardi = Cal_Tardiness(TempSeq, length);
        if (NewTardi < tardiness)
        {
            for (j = 0; j < length; j++) BestSeq[j] = TempSeq[j];
            tardiness = NewTardi;
        }
        for (k = 0; k < length; k++) TempSeq[k] = sequence[k];
    }
    for (i = 0; i < length; i++) sequence[i] = BestSeq[i];

    if (TempSeq) delete[] TempSeq;
    if (BestSeq) delete[] BestSeq;
    return (tardiness);
}

// This function does the adjacent pairwise interchanges
// to the this single machine
void ParallelMachine::Single_Machine_Local_Search_Again(int* sequence, int length, int index)
{}
// This function uses metropolis to search for improvement at
// this stage

int ParallelMachine::Stage_Local_Search(int ITERATION)
{
    // the Tabu structure used to avoid  moving a job back and forth
    // between two machines
    struct Tabu
    {
        int job;    // the job being moved
        int FromM;  // the machine moved from
        int ToM;    // the machine moved to

        int Is_Tabu(int j, int FM, int TM)
        {
            if (j == job && FM == ToM && TM == FromM)
                return (TRUE);
            else
                return (FALSE);
        };
    } TabuList;

    int AccBad = 0;
    int Bad = 0;

    // initialize
    TabuList.job = -1;
    TabuList.FromM = -1;
    TabuList.ToM = -1;

    int i;
    // flag: the machine has performed single machine swaps or not
    // the goal is that each machine needs to do swaps at least once
    int* touched;
    touched = new int[RSetSize];
    for (i = 0; i < RSetSize; i++) touched[i] = FALSE;

    for (int iter = 0; iter < ITERATION; iter++)
    {
        int MinT = INFINITY;
        int MaxT = -INFINITY;
        int MinIndex = 0;
        int MaxIndex = 0;
        for (i = 0; i < RSetSize; i++)
        {
            if (RestrictedSet[i].tardiness < MinT)
            {
                MinIndex = i;
                MinT = RestrictedSet[i].tardiness;
            }
            if (RestrictedSet[i].tardiness > MaxT)
            {
                MaxIndex = i;
                MaxT = RestrictedSet[i].tardiness;
            }
        }

        // balanced case, local search on each machine and then go back to the next iteration
        // since it is possible that after the local search no longer balanced among machines
        if (MaxIndex == MinIndex)
        {
            for (i = 0; i < RSetSize; i++)
                RestrictedSet[i].tardiness =
                    Single_Machine_Local_Search(RestrictedSet[i].sequence, RestrictedSet[i].length);
            // record the new schedule for this stage
            objective = 0;
            for (i = 0; i < RSetSize; i++) objective += RestrictedSet[i].tardiness;

            if (objective == 0)  // optimal for this stage already, no need to do more
                break;

            if (objective < OptValue)
            {  // record the new best schedule

                OptValue = objective;
                for (i = 0; i < RSetSize; i++) OptSchedule[i] = RestrictedSet[i];
            }
        }  // if

        else
        {
            // take the job with the max SUM(WjTj) out of this MaxIndex machine
            int MaxJobIndex = Uniform(0, RestrictedSet[MaxIndex].length - 1);
            int MaxJob = RestrictedSet[MaxIndex].sequence[MaxJobIndex];

            // check if this move is a Tabu.
            // If yes, generate another job and keep going
            while (TabuList.Is_Tabu(MaxJob, MaxIndex, MinIndex))
            {
                MaxJobIndex = Uniform(0, RestrictedSet[MaxIndex].length - 1);
                MaxJob = RestrictedSet[MaxIndex].sequence[MaxJobIndex];
            }

            // print out the move (for debugging)
            // cout << "Job" << MaxJob;
            // cout << "  FromM=" << MaxIndex;
            // cout << "  ToM= " << MinIndex << endl;

            // Overwrite the tabulist by the current move
            TabuList.job = MaxJob;
            TabuList.FromM = MaxIndex;
            TabuList.ToM = MinIndex;

            // adjust the sequence for the MaxIndex machine
            // it is easy to restore in neccessary later on since we know MaxJobIndex
            for (i = MaxJobIndex; i < (RestrictedSet[MaxIndex].length - 1); i++)
                RestrictedSet[MaxIndex].sequence[i] = RestrictedSet[MaxIndex].sequence[i + 1];

            RestrictedSet[MaxIndex].length--;
            int TardinessOnMaxM = RestrictedSet[MaxIndex].tardiness;
            RestrictedSet[MaxIndex].tardiness =
                Cal_Tardiness(RestrictedSet[MaxIndex].sequence, RestrictedSet[MaxIndex].length);

            // Put the taken out job to the MinIndex machine and
            // insert it at the position that gives the mininum total tardiness for this machine
            // all the operations are done on "JobSet"
            int* JobSet;
            JobSet = new int[NumJobs];
            int JobSize = RestrictedSet[MinIndex].length + 1;

            JobSet[0] = MaxJob;
            for (i = 1; i < JobSize; i++) JobSet[i] = RestrictedSet[MinIndex].sequence[i - 1];

            int InsertPoint = 0;
            int MinTardiness = Cal_Tardiness(JobSet, JobSize);
            for (i = 1; i < JobSize; i++)
            {
                int temp = JobSet[i];
                JobSet[i] = JobSet[i - 1];
                JobSet[i - 1] = temp;

                int TempTardiness = Cal_Tardiness(JobSet, JobSize);
                if (TempTardiness < MinTardiness)
                {
                    InsertPoint = i;
                    MinTardiness = TempTardiness;
                }
            }

            // The schedule with the best insertion point
            if (InsertPoint == 0)
            {
                JobSet[0] = MaxJob;
                for (i = 1; i < JobSize; i++) JobSet[i] = RestrictedSet[MinIndex].sequence[i - 1];
            }

            else
            {
                for (i = 0; i < InsertPoint; i++) JobSet[i] = RestrictedSet[MinIndex].sequence[i];
                JobSet[InsertPoint] = MaxJob;
                for (i = (InsertPoint + 1); i < JobSize; i++)
                    JobSet[i] = RestrictedSet[MinIndex].sequence[i - 1];
            }

            // apply the single machine local search to the MinIndex machine
            int TardinessOnMinM = Single_Machine_Local_Search(JobSet, JobSize);

            // to calculate the new objective value for this stage
            objective = 0;
            for (i = 0; i < RSetSize; i++) objective += RestrictedSet[i].tardiness;
            objective = objective - RestrictedSet[MinIndex].tardiness + TardinessOnMinM;

            // cout << " obj=" << objective << "\n";

            if (objective == 0)  // optimal for this stage already, no need to do more
                break;

            if (objective <= OptValue)
            {  // keep the new best schedule

                OptValue = objective;

                RestrictedSet[MinIndex].length++;
                for (i = 0; i < JobSize; i++) RestrictedSet[MinIndex].sequence[i] = JobSet[i];
                RestrictedSet[MinIndex].tardiness = TardinessOnMinM;

                for (i = 0; i < RSetSize; i++) OptSchedule[i] = RestrictedSet[i];

                touched[MinIndex] = TRUE;
            }
            else
            {  // do the metropolis test

                int ToAccept = Metropolis_Test(OptValue, objective);

                Bad++;

                if (ToAccept)
                {
                    AccBad++;

                    RestrictedSet[MinIndex].length++;
                    for (i = 0; i < JobSize; i++) RestrictedSet[MinIndex].sequence[i] = JobSet[i];
                    RestrictedSet[MinIndex].tardiness = TardinessOnMinM;

                    touched[MinIndex] = TRUE;
                }
                else
                {  // restore the MaxIndex Machine

                    RestrictedSet[MaxIndex].length++;

                    for (i = RestrictedSet[MaxIndex].length - 1; i > MaxJobIndex; i--)
                        RestrictedSet[MaxIndex].sequence[i] = RestrictedSet[MaxIndex].sequence[i - 1];
                    RestrictedSet[MaxIndex].sequence[MaxJobIndex] = MaxJob;

                    RestrictedSet[MaxIndex].tardiness = TardinessOnMaxM;
                }
            }

            if (JobSet) delete[] JobSet;

        }  // else

    }  // for iter

    // cout << "Bad=  " << Bad;
    // cout << "AccBad=  " << AccBad;
    // cout << AccBad/(double)Bad << endl;

    // if there are machines that haven't performed single machine swaps yet, then do it
    if (objective != 0)
    {
        for (i = 0; i < RSetSize; i++)
        {
            if (!touched[i])
                RestrictedSet[i].tardiness =
                    Single_Machine_Local_Search(RestrictedSet[i].sequence, RestrictedSet[i].length);
        }
        objective = 0;
        for (i = 0; i < RSetSize; i++) objective += RestrictedSet[i].tardiness;
        // cout << " obj=" << objective << "\n";

        if (objective < OptValue)
        {  // record the new best schedule

            OptValue = objective;
            for (i = 0; i < RSetSize; i++) OptSchedule[i] = RestrictedSet[i];
        }
    }  // if objective != 0

    if (touched) delete[] touched;
    return (OptValue);
}

// record the individual tardiness of each job given a schedule for the stage
int Tardiness[MAXJOB][MAXJOB];
int MaxTardyJob[MAXJOB];  // record the job giving the max tardiness
int MaxTardiness[MAXJOB];

int ParallelMachine::Stage_Local_Search_Again(int ITERATION)
{
    // the Tabu structure used to avoid moving a job back and forth
    // between two machines
    struct Tabu
    {
        int job;    // the job being moved
        int FromM;  // the machine moved from
        int ToM;    // the machine moved to

        int Is_Tabu(int j, int FM, int TM)
        {
            if (j == job && FM == ToM && TM == FromM)
                return (TRUE);
            else
                return (FALSE);
        };
    } TabuList;

    int AccBad = 0;
    int Bad = 0;

    int i, j, k;

    for (i = 0; i < NumJobs; i++) MaxTardiness[i] = 0;

    // compute the tardiness of each job and figure out the maxT^k_j
    for (i = 0; i < RSetSize; i++)
        Compute_Individual_Tardiness(RestrictedSet[i].sequence, RestrictedSet[i].length);

    // initialize
    TabuList.job = -1;
    TabuList.FromM = -1;
    TabuList.ToM = -1;

    // flag: the machine has performed single machine swaps or not
    // the goal is that each machine needs to do swaps at least once
    int* touched;
    touched = new int[RSetSize];
    for (i = 0; i < RSetSize; i++) touched[i] = FALSE;

    for (int iter = 0; iter < ITERATION; iter++)
    {
        for (i = 0; i < NumJobs; i++)
        {
            for (j = 0; j < NumJobs; j++)
            {
                if (Tardiness[j][i] > MaxTardiness[i])
                {
                    MaxTardiness[i] = Tardiness[j][i];
                    MaxTardyJob[i] = j;
                }
            }
        }

        int MTotal[MAXMACHINE];
        int JTotal[MAXJOB];
        for (i = 0; i < RSetSize; i++) MTotal[i] = 0;
        for (i = 0; i < NumJobs; i++) JTotal[i] = 0;

        for (i = 0; i < RSetSize; i++)
        {
            for (j = 0; j < RestrictedSet[i].length; j++)
            {
                for (k = 0; k < NumJobs; k++)
                {
                    if (RestrictedSet[i].sequence[j] == MaxTardyJob[k])
                        JTotal[RestrictedSet[i].sequence[j]] +=
                            jobs[k].Weight * Tardiness[RestrictedSet[i].sequence[j]][k];
                }

                MTotal[i] += JTotal[RestrictedSet[i].sequence[j]];
            }
        }

        int objective = 0;
        for (i = 0; i < NumJobs; i++) objective += JTotal[i];

        if (objective == 0) return objective;  // Optimal already

        if (objective < OptValue2)
        {
            OptValue2 = objective;
            for (i = 0; i < RSetSize; i++) OptSchedule2[i] = RestrictedSet[i];
        }

        int MinT = INFINITY;
        int MaxT = -INFINITY;
        int MinIndex = 0;
        int MaxIndex = 0;
        for (i = 0; i < RSetSize; i++)
        {
            if (MTotal[i] < MinT)
            {
                MinIndex = i;
                MinT = MTotal[i];
            }
            if (MTotal[i] > MaxT)
            {
                MaxIndex = i;
                MaxT = MTotal[i];
            }
        }

        int MaxJobIndex;
        int MaxJob;
        int temp = 0;
        // take the job with the max JTotal out of this MaxIndex machine
        for (i = 0; i < RestrictedSet[MaxIndex].length; i++)
        {
            if (JTotal[RestrictedSet[MaxIndex].sequence[i]] > temp)
            {
                temp = JTotal[RestrictedSet[MaxIndex].sequence[i]];
                MaxJobIndex = i;
            }
        }
        MaxJob = RestrictedSet[MaxIndex].sequence[MaxJobIndex];

        // check if this move is a Tabu.
        // If yes, generate another job and keep going
        while (TabuList.Is_Tabu(MaxJob, MaxIndex, MinIndex))
        {
            MaxJobIndex = Uniform(0, RestrictedSet[MaxIndex].length - 1);
            MaxJob = RestrictedSet[MaxIndex].sequence[MaxJobIndex];
        }

        // print out the move (for debugging)
        // cout << "Job" << MaxJob;
        // cout << "  FromM=" << MaxIndex;
        // cout << "  ToM= " << MinIndex << endl;

        // Overwrite the tabulist by the current move
        TabuList.job = MaxJob;
        TabuList.FromM = MaxIndex;
        TabuList.ToM = MinIndex;

        // adjust the sequence for the MaxIndex machine
        // it is easy to restore in neccessary later on since we know MaxJobIndex
        for (i = MaxJobIndex; i < (RestrictedSet[MaxIndex].length - 1); i++)
            RestrictedSet[MaxIndex].sequence[i] = RestrictedSet[MaxIndex].sequence[i + 1];

        RestrictedSet[MaxIndex].length--;
        // int TardinessOnMaxM = RestrictedSet[MaxIndex].tardiness;
        // RestrictedSet[MaxIndex].tardiness =
        // Cal_Tardiness(RestrictedSet[MaxIndex].sequence, RestrictedSet[MaxIndex].length);

        // Put the taken out job to the MinIndex machine and
        // insert it at the position that right after its release date
        // all the operations are done on "JobSet"
        int* JobSet;
        JobSet = new int[NumJobs];
        int JobSize = RestrictedSet[MinIndex].length + 1;

        int InsertPoint;
        int finish = jobs[RestrictedSet[MinIndex].sequence[0]].Release +
                     jobs[RestrictedSet[MinIndex].sequence[0]].Process;

        for (i = 0; i < RestrictedSet[MinIndex].length; i++)
        {
            if (jobs[MaxJob].Release < finish)
            {
                InsertPoint = i;
                break;
            }
            else if (i < RestrictedSet[MinIndex].length - 1)
                finish = max(finish, jobs[RestrictedSet[MinIndex].sequence[i + 1]].Release) +
                         jobs[RestrictedSet[MinIndex].sequence[i + 1]].Process;
            else if (i == RestrictedSet[MinIndex].length - 1)
                InsertPoint = i + 1;
        }

        // The schedule with the best insertion point
        if (InsertPoint == 0)
        {
            JobSet[0] = MaxJob;
            for (i = 1; i < JobSize; i++) JobSet[i] = RestrictedSet[MinIndex].sequence[i - 1];
        }

        else
        {
            for (i = 0; i < InsertPoint; i++) JobSet[i] = RestrictedSet[MinIndex].sequence[i];
            JobSet[InsertPoint] = MaxJob;
            for (i = (InsertPoint + 1); i < JobSize; i++) JobSet[i] = RestrictedSet[MinIndex].sequence[i - 1];
        }

        // reschedule the jobs on this machine
        Single_Machine_Local_Search_Again(JobSet, JobSize, MinIndex);
        Compute_Individual_Tardiness(JobSet, JobSize);
        Compute_Individual_Tardiness(RestrictedSet[MaxIndex].sequence, RestrictedSet[MaxIndex].length);

        // to calculate the new objective value for this stage

        for (i = 0; i < RestrictedSet[MaxIndex].length; i++)
        {
            for (k = 0; k < NumJobs; k++)
            {
                if (Tardiness[RestrictedSet[MaxIndex].sequence[i]][k] > MaxTardiness[k])
                    MaxTardiness[k] = Tardiness[RestrictedSet[MaxIndex].sequence[i]][k];
                MaxTardyJob[k] = RestrictedSet[MaxIndex].sequence[i];
            }
        }

        for (i = 0; i < JobSize; i++)
        {
            for (k = 0; k < NumJobs; k++)
            {
                if (Tardiness[JobSet[i]][k] > MaxTardiness[k])
                {
                    MaxTardiness[k] = Tardiness[JobSet[i]][k];
                    MaxTardyJob[k] = JobSet[i];
                }
            }
        }

        objective = 0;
        for (i = 0; i < NumJobs; i++) objective += jobs[MaxTardyJob[i]].Weight * MaxTardiness[i];

        // cout << " obj=" << objective << "\n";

        if (objective == 0)  // optimal for this stage already, no need to do more
            break;

        if (objective < OptValue2)
        {  // keep the new best schedule

            OptValue2 = objective;

            RestrictedSet[MinIndex].length++;
            for (i = 0; i < JobSize; i++) RestrictedSet[MinIndex].sequence[i] = JobSet[i];

            for (i = 0; i < RSetSize; i++) OptSchedule2[i] = RestrictedSet[i];

            touched[MinIndex] = TRUE;
        }
        else
        {  // do the metropolis test

            int ToAccept = Metropolis_Test(OptValue2, objective);

            Bad++;

            if (ToAccept)
            {
                AccBad++;

                RestrictedSet[MinIndex].length++;
                for (i = 0; i < JobSize; i++) RestrictedSet[MinIndex].sequence[i] = JobSet[i];

                touched[MinIndex] = TRUE;
            }
            else
            {  // restore the MaxIndex Machine

                RestrictedSet[MaxIndex].length++;

                for (i = RestrictedSet[MaxIndex].length - 1; i > MaxJobIndex; i--)
                    RestrictedSet[MaxIndex].sequence[i] = RestrictedSet[MaxIndex].sequence[i - 1];
                RestrictedSet[MaxIndex].sequence[MaxJobIndex] = MaxJob;
            }
        }

        if (JobSet) delete[] JobSet;

    }  // for iter

    // cout << "Bad=  " << Bad;
    // cout << "AccBad=  " << AccBad;
    // cout << double(AccBad)/Bad << endl;

    if (touched) delete[] touched;
    return (OptValue2);
}

void ParallelMachine::Compute_Individual_Tardiness(const int* sequence, const int length)
{
    int i, k;
    int start;

    start = jobs[sequence[0]].Release;
    for (i = 0; i < length; i++)
    {
        for (k = 0; k < NumJobs; k++)
        {
            if (jobs[sequence[i]].Due[k] != INFINITY)
                Tardiness[sequence[i]][k] = pos(start + jobs[sequence[i]].Process - jobs[sequence[i]].Due[k]);
            else
                Tardiness[sequence[i]][k] = 0;
        }

        if (i < (length - 1)) start = max(start + jobs[sequence[i]].Process, jobs[sequence[i + 1]].Release);
    }
}
// This function calculates the tardiness "(double sum of WjTj)" of a given sequence and its length

int ParallelMachine::Cal_Tardiness(const int* sequence, const int length)
{
    int i, k;
    int start;
    int tardiness = 0;

    start = jobs[sequence[0]].Release;
    for (i = 0; i < length; i++)
    {
        for (k = 0; k < NumJobs; k++)
            if (jobs[sequence[i]].Due[k] != INFINITY)
                tardiness += jobs[sequence[i]].Weight *
                             pos(start + jobs[sequence[i]].Process - jobs[sequence[i]].Due[k]);

        if (i < (length - 1)) start = max(start + jobs[sequence[i]].Process, jobs[sequence[i + 1]].Release);
    }

    return tardiness;
}

// This function calculates the objective function: sum of WjTj of a given sequence.
// Note NOT DOUBLE SUM
int ParallelMachine::Cal_Objective_Value(const int* sequence, const int length, const int* duedate)
{
    int start;
    int tardiness = 0;

    start = jobs[sequence[0]].Release;
    for (int i = 0; i < length; i++)
    {
        tardiness += jobs[sequence[i]].Weight * pos(start + jobs[sequence[i]].Process - duedate[sequence[i]]);
        if (i < (length - 1)) start = max(start + jobs[sequence[i]].Process, jobs[sequence[i + 1]].Release);
    }
    return tardiness;
}

// This function check weather or not a given partial schedule is an active schedule

// actually, it only checks if the last job violates the active schedule rule since
// the partial schedule without the last jos IS ALWAYS active schedule

// the way to do it is:  if r(last job) < finishtime (i), then
// if max(r(last job), finishtime(i-1)) + p(last job) < finishtime (i), ====> NOT active

int ParallelMachine::Is_Active_Schedule(const int* schedule, const int length)
{
    int active = TRUE;
    int* finish;
    int i;

    finish = new int[NumJobs];

    // calculate the finish time of each job

    finish[0] = jobs[schedule[0]].Release + jobs[schedule[0]].Process;
    i = 0;
    if (length > 2)
    {
        for (i = 1; i < (length - 1); i++)
            finish[i] = max(finish[i - 1], jobs[schedule[i]].Release) + jobs[schedule[i]].Process;
    }

    while (i > 0 && jobs[schedule[length - 1]].Release < finish[i]) i--;

    if (i == 0 && (jobs[schedule[length - 1]].Release + jobs[schedule[length - 1]].Process) < jobs[0].Release)
        active = FALSE;

    else if ((max(jobs[schedule[length - 1]].Release, finish[i - 1]) + jobs[schedule[length - 1]].Process) <
             jobs[i].Release)
        active = FALSE;

    if (finish) delete[] finish;

    return active;
}

// This function calls the routines to complete the algorithm for the stage
void ParallelMachine::Solve_The_Stage()
{
    int i, counter = 0;  // to record the number of nodes explored in B&B

    // use AATC to get an initial schedule
    objective = Cal_Initial_Upperbound();
    OptValue = objective;
    OptValue2 = INFINITY;

    // print info. for testing
    // printf(" \n The initial schedule by AATC rule is. objective= %d \n", objective);

    // use local search trying to get improvement over initial schedule

    for (counter = 0; counter < 10; counter++)
    {
        // with the local objective function "double sum of WjTj"
        OptValue = Stage_Local_Search(50);
        // cout << "OptValue=" << OptValue << endl;

        // the working schedule is always RestrictedSet
        RSetSize = OptSize;
        for (i = 0; i < RSetSize; i++) RestrictedSet[i] = OptSchedule[i];

        OptValue2 = Stage_Local_Search_Again(10);
        // cout << "OptValue2=" << OptValue2 << endl;

        objective = 0;
        for (i = 0; i < RSetSize; i++)
            objective += Cal_Tardiness(OptSchedule2[i].sequence, OptSchedule2[i].length);
        for (i = 0; i < RSetSize; i++) RestrictedSet[i] = OptSchedule2[i];
    }

    // assume always every machine is used
    RSetSize = OptSize = NumMachines;
    for (i = 0; i < RSetSize; i++) OptSchedule[i] = OptSchedule2[i];

    // print info. for testing
    //  printf(" \n value after local search for this stage = %d \n", OptValue);
    // for (int i=0; i< OptSize; i++){
    //        printf(" ====Machine %d:::", i);
    //    for (int l=0; l<OptSchedule[i].length; l++)
    //           printf("%d,  ", OptSchedule[i].sequence[l]);
    //   printf("\n");
    //  }
}

// It is different from Solve_The_Stage only from the point that
// no cal_initial_upperbound is called since we improve from the
// schedule returned from the Local Search procedure
void ParallelMachine::Reschedule_Process()
{
    // Solve_The_Stage();

    int i, counter = 0;  // to record the number of nodes explored in B&B

    OptValue2 = INFINITY;

    // print info. for testing
    // printf(" \n The initial schedule by AATC rule is. objective= %d \n", objective);

    // use local search trying to get improvement over initial schedule

    for (counter = 0; counter < 10; counter++)
    {
        // with the local objective function "double sum of WjTj"
        OptValue = Stage_Local_Search(50);
        // cout << "OptValue=" << OptValue << endl;

        // the working schedule is always RestrictedSet
        RSetSize = OptSize;
        for (i = 0; i < RSetSize; i++) RestrictedSet[i] = OptSchedule[i];

        OptValue2 = Stage_Local_Search_Again(10);
        // cout << "OptValue2=" << OptValue2 << endl;

        objective = 0;
        for (i = 0; i < RSetSize; i++)
            objective += Cal_Tardiness(OptSchedule2[i].sequence, OptSchedule2[i].length);
        for (i = 0; i < RSetSize; i++) RestrictedSet[i] = OptSchedule2[i];
    }

    // assume always every machine is used
    RSetSize = OptSize = NumMachines;
    for (i = 0; i < RSetSize; i++) OptSchedule[i] = OptSchedule2[i];

    // print info. for testing
    //  printf(" \n value after local search for this stage = %d \n", OptValue);
    // for (int i=0; i< OptSize; i++){
    //        printf(" ====Machine %d:::", i);
    //    for (int l=0; l<OptSchedule[i].length; l++)
    //           printf("%d,  ", OptSchedule[i].sequence[l]);
    //   printf("\n");
    //  }
}
