#include "StdAfx.h"

#include "Input.h"
#include "StageBB.h"

extern int Position_Min(int, int);
extern int pos(int);
extern int max(int, int);

// Graph.cxx:
// This file is where all the operations on the disjunctive graph are.
// The major operations are :
// Build up the disjunctive graph using the data info.
// calculating local release times and due times via adjusted longest path algo,
// fixing disjunctive arcs once a stage is scheduled

// constructor for Graph Object
Graph::Graph(int nums, int numj)
{
    int i;

    NumMachines = new int[nums];

    jobs = new Job[numj];

    graph = new Node**[nums];
    for (i = 0; i < nums; i++)
    {
        graph[i] = new Node*[numj];
        for (int j = 0; j < numj; j++) graph[i][j] = new Node;
    }

    source = new Source(numj);
    if (!source->ConNext[0])
    {
        cout << " stupid allocation 2";
        exit(1);
    }
    sink = new Sink*[numj];
    for (i = 0; i < numj; i++) sink[i] = new Sink;

    IsScheduled = new int[nums];
    BottleList = new int[nums];
}

// destructor for Graph object
Graph::~Graph()
{
    if (NumMachines) delete[] NumMachines;

    if (jobs) delete[] jobs;

    for (int i = 0; i < NumStages; i++)
        if (graph[i]) delete[] graph[i];
    delete[] graph;

    if (sink) delete[] sink;

    if (IsScheduled) delete[] IsScheduled;
    if (BottleList) delete[] BottleList;
}

// This function resets the field PathLength to INFINITY of those nodes needed
// This is needed in the longest path algorithm
void Graph::Reset(int from, int to)
{
    int i, j;

    for (i = from; i <= to; i++)
        for (j = 0; j < NumJobs; j++)
        {
            if (i == NumStages)
                sink[j]->PathLength = INFINITY;
            else
                graph[i][j]->PathLength = INFINITY;
        }
}

// This is used when doing the rescheduling, you need to toggle the
// already scheduled stage in order to schedule it again. this is Reset
void Graph::Reset(int StageIndex)
{
    Node* node;

    IsScheduled[StageIndex] = FALSE;

    for (int j = 0; j < NumJobs; j++)
    {
        node = graph[StageIndex][j];
        node->PathLength = INFINITY;
        node->ExistDisNext = FALSE;
        node->ExistDisPre = FALSE;
    }
}

// This function calculates the release time of a given node at current
// status by calculating the longest path from source to this node

int Graph::Cal_Release_Time(int StageIndex, int JobIndex)
{
    Node_Index* QUEUE;
    Node* ThisNode;
    int ThisStage;
    int ThisJob;

    // Note that every time before calling this function, the
    //"PathLength" field of all the nodes must be reset to INFINITY

    Reset(0, StageIndex);

    // If none of the stages is scheduled, the completion time is just the sum of processing time over all
    // stages

    int isscheduled = FALSE;
    for (int j = 0; j <= StageIndex; j++)
        if (IsScheduled[j])
        {
            isscheduled = TRUE;
            break;
        }
    if (!isscheduled)
    {
        int length = 0;

        length += source->ConNext[JobIndex]->length;
        for (int j = 1; j <= StageIndex; j++) length += graph[j - 1][JobIndex]->ConNext->length;

        graph[StageIndex][JobIndex]->operation.Release = length;
        return (1);
    }

    // in the case of partially scheduled disjunctive graph
    QUEUE = new Node_Index[(NumStages + 1) * NumJobs * NumJobs + 1];
    int head = 0;  // The pointer to the front of the QUEUE, which is the one should be processed currently
    int QLength = 0;

    for (int i = 0; i < NumJobs; i++)
    {
        graph[0][i]->PathLength = source->ConNext[i]->neglength;
        QLength++;
        QUEUE[QLength - 1].StageIndex = 0;
        QUEUE[QLength - 1].JobIndex = i;
    }

    while (head < QLength && QLength > 0)
    {
        ThisStage = QUEUE[head].StageIndex;
        ThisJob = QUEUE[head].JobIndex;
        ThisNode = graph[ThisStage][ThisJob];
        head++;  // remove the node from the front of the QUEUE

        int temp;
        if (ThisNode->ExistDisNext)
        {  // two arcs go out of this node
            temp = ThisNode->PathLength + ThisNode->DisNext->neglength;
            if (graph[ThisStage][ThisNode->DisNext->to]->PathLength > temp)
            {
                graph[ThisStage][ThisNode->DisNext->to]->PathLength = temp;
                if (!in(ThisStage, ThisNode->DisNext->to, QUEUE, head, QLength))
                {
                    QLength++;
                    QUEUE[QLength - 1].StageIndex = ThisStage;
                    QUEUE[QLength - 1].JobIndex = ThisNode->DisNext->to;
                }
            }
        }

        // the common part, conjunctive arc points to next stage
        if (ThisNode->ConNext->to <= StageIndex)
        {
            temp = ThisNode->PathLength + ThisNode->ConNext->neglength;
            if (graph[ThisNode->ConNext->to][ThisJob]->PathLength > temp)
            {
                graph[ThisNode->ConNext->to][ThisJob]->PathLength = temp;
                if (!in(ThisNode->ConNext->to, ThisJob, QUEUE, head, QLength))
                {
                    QLength++;
                    QUEUE[QLength - 1].StageIndex = ThisNode->ConNext->to;
                    QUEUE[QLength - 1].JobIndex = ThisJob;
                }
            }
        }
    }

    graph[StageIndex][JobIndex]->operation.Release = -graph[StageIndex][JobIndex]->PathLength;
    if (QUEUE) delete[] QUEUE;
    return (1);
}

// This function calculates the set of due dates of the node

// Note that it's possible there is no path from node to one of the sink nodes
// in this case, this local due date is set to INFINITY

void Graph::Cal_Due_Dates(int StageIndex, int JobIndex)
{
    Node_Index* QUEUE;
    Node* ThisNode;
    int ThisStage;
    int ThisJob;
    int scheduled = FALSE;
    int i;

    // The special case is that if StageIndex==NumStages-1 (i.e. the last stage,)
    // then, the local due dates are known without any calculations of longest path
    if (StageIndex == (NumStages - 1))
    {  // last stage
        for (i = 0; i < NumJobs; i++) graph[StageIndex][JobIndex]->operation.Due[i] = INFINITY;
        graph[StageIndex][JobIndex]->operation.Due[JobIndex] =
            max(jobs[JobIndex].DueTime, jobs[JobIndex].CompleteTime);

        return;
    }

    // Note that every time before calling this function, the
    //"PathLength" field of all the nodes must be reset to INFINITY

    Reset(StageIndex, NumStages - 1);

    // In the case no subsequent stage is ever scheduled, we have only 1
    // path to its own sink node. This is handled seperating from the general case
    int minlength = 0;
    for (i = StageIndex; i < NumStages; i++)
        if (!IsScheduled[i])
            minlength += graph[i][JobIndex]->ConNext->length;
        else
        {
            minlength = 0;
            scheduled = TRUE;
            break;
        }

    if (!scheduled)
    {
        for (i = 0; i < NumJobs; i++) graph[StageIndex][JobIndex]->operation.Due[i] = INFINITY;

        graph[StageIndex][JobIndex]->operation.Due[JobIndex] =
            max(jobs[JobIndex].DueTime, jobs[JobIndex].CompleteTime) - minlength +
            graph[StageIndex][JobIndex]->operation.Process;
        return;
    }

    // in the case of partially scheduled disjunctive graph
    QUEUE = new Node_Index[(NumStages + 1) * NumJobs * NumJobs + 1];
    int head = 0;
    int QLength = 1;

    QUEUE[QLength - 1].StageIndex = StageIndex;
    QUEUE[QLength - 1].JobIndex = JobIndex;
    graph[StageIndex][JobIndex]->PathLength = 0;

    while (head < QLength && QLength > 0)
    {
        ThisStage = QUEUE[head].StageIndex;
        ThisJob = QUEUE[head].JobIndex;
        ThisNode = graph[ThisStage][ThisJob];
        head++;

        int temp;
        if (ThisNode->ExistDisNext)
        {  // two arcs go out of this node
            temp = ThisNode->PathLength + ThisNode->DisNext->neglength;
            if (graph[ThisStage][ThisNode->DisNext->to]->PathLength > temp)
            {
                graph[ThisStage][ThisNode->DisNext->to]->PathLength = temp;
                if (!in(ThisStage, ThisNode->DisNext->to, QUEUE, head, QLength))
                {
                    QLength++;
                    QUEUE[QLength - 1].StageIndex = ThisStage;
                    QUEUE[QLength - 1].JobIndex = ThisNode->DisNext->to;
                }
            }
        }

        // the common part, conjunctive arc points to next stage
        if (ThisNode->ConNext->to < NumStages)
        {
            temp = ThisNode->PathLength + ThisNode->ConNext->neglength;
            if (graph[ThisNode->ConNext->to][ThisJob]->PathLength > temp)
            {
                graph[ThisNode->ConNext->to][ThisJob]->PathLength = temp;
                if (!in(ThisNode->ConNext->to, ThisJob, QUEUE, head, QLength))
                {
                    QLength++;
                    QUEUE[QLength - 1].StageIndex = ThisNode->ConNext->to;
                    QUEUE[QLength - 1].JobIndex = ThisJob;
                }
            }
        }
    }

    for (i = 0; i < NumJobs; i++)
    {
        if (graph[NumStages - 1][i]->PathLength == INFINITY)
            graph[StageIndex][JobIndex]->operation.Due[i] = INFINITY;
        else
            graph[StageIndex][JobIndex]->operation.Due[i] =
                max(jobs[i].DueTime, jobs[i].CompleteTime) + graph[NumStages - 1][i]->PathLength -
                graph[NumStages - 1][i]->operation.Process +
                graph[StageIndex][JobIndex]->operation.Process;  // note minlength is "-"
    }
    if (QUEUE) delete[] QUEUE;
}

// This function calculates the completion time of job i in current disjunctive graph,
// which is the longest path from source node to sink[JobIndex] node
// We only need to find out the longest path to "NumStage-1" node and then just add one term
void Graph::Cal_Completion_Time(int JobIndex)
{
    Node_Index* QUEUE;
    Node* ThisNode;
    int ThisStage;
    int ThisJob;

    // Note that every time before calling this function, the
    //"PathLength" field of all the nodes must be reset to INFINITY
    // since there is one and only one path to a sink node, so, it's equivalent to find out
    // the longest path from source to node(StageIndex, JobIndex), and then...

    int StageIndex = NumStages - 1;
    Reset(0, StageIndex);

    // If none of the stages is scheduled, the completion time is just the sum of processing time over all
    // stages

    int isscheduled = FALSE;
    for (int j = 0; j <= StageIndex; j++)
        if (IsScheduled[j])
        {
            isscheduled = TRUE;
            break;
        }
    if (!isscheduled)
    {
        int length = 0;

        length += source->ConNext[JobIndex]->length;
        for (int j = 1; j <= NumStages; j++) length += graph[j - 1][JobIndex]->ConNext->length;

        jobs[JobIndex].CompleteTime = length;
        return;
    }

    // in the case of partially scheduled disjunctive graph
    QUEUE = new Node_Index[(NumStages + 1) * NumJobs * NumJobs + 1];
    int head = 0;
    int QLength = 0;

    for (int i = 0; i < NumJobs; i++)
    {
        graph[0][i]->PathLength = source->ConNext[i]->neglength;
        QLength++;
        QUEUE[QLength - 1].StageIndex = 0;
        QUEUE[QLength - 1].JobIndex = i;
    }

    while (head < QLength && QLength > 0)
    {
        ThisStage = QUEUE[head].StageIndex;
        ThisJob = QUEUE[head].JobIndex;
        ThisNode = graph[ThisStage][ThisJob];
        head++;

        int temp;
        if (ThisNode->ExistDisNext)
        {  // two arcs go out of this node
            temp = ThisNode->PathLength + ThisNode->DisNext->neglength;
            if (graph[ThisStage][ThisNode->DisNext->to]->PathLength > temp)
            {
                graph[ThisStage][ThisNode->DisNext->to]->PathLength = temp;
                if (!in(ThisStage, ThisNode->DisNext->to, QUEUE, head, QLength))
                {
                    QLength++;
                    QUEUE[QLength - 1].StageIndex = ThisStage;
                    QUEUE[QLength - 1].JobIndex = ThisNode->DisNext->to;
                }
            }
        }

        // the common part, conjunctive arc points to next stage
        if (ThisNode->ConNext->to < NumStages)
        {
            temp = ThisNode->PathLength + ThisNode->ConNext->neglength;
            if (graph[ThisNode->ConNext->to][ThisJob]->PathLength > temp)
            {
                graph[ThisNode->ConNext->to][ThisJob]->PathLength = temp;
                if (!in(ThisNode->ConNext->to, ThisJob, QUEUE, head, QLength))
                {
                    QLength++;
                    QUEUE[QLength - 1].StageIndex = ThisNode->ConNext->to;
                    QUEUE[QLength - 1].JobIndex = ThisJob;
                }
            }
        }
    }

    jobs[JobIndex].CompleteTime =
        -graph[StageIndex][JobIndex]->PathLength + graph[StageIndex][JobIndex]->operation.Process;
    if (QUEUE) delete[] QUEUE;
}

// This function reads the data about the stages and the jobs to build up
// the disjunctive graph for the algorithm. Data is read from the data files

// A lot of the initialization of members is done in this function

// since D is not to be changed, so, "const" is used here
void Graph::Buildup_Graph(const Data& D)
{
    int i, j, k;

    NumStages = D.NumStages;
    NumJobs = D.NumJobs;
    for (i = 0; i < NumStages; i++) NumMachines[i] = D.machines[i];

    // fill in the data about the jobs
    for (i = 0; i < NumJobs; i++)
    {
        jobs[i].ReleaseTime = D.ReleaseTime[i];
        jobs[i].weight = D.weight[i];
        jobs[i].DueTime = D.DueTime[i];
        for (j = 0; j < NumStages; j++) jobs[i].ProcessTime[j] = D.ProcessTime[i][j];
    }

    // build up the graph
    for (i = 0; i < NumJobs; i++)
    {
        source->ConNext[i]->to = 0;
        source->ConNext[i]->length = jobs[i].ReleaseTime;
        source->ConNext[i]->neglength = -jobs[i].ReleaseTime;
    }

    for (i = 0; i < NumJobs; i++)
    {
        sink[i]->ConPre->from = NumStages - 1;
        sink[i]->ConPre->length = jobs[i].ProcessTime[NumStages - 1];
        sink[i]->ConPre->neglength = -jobs[i].ProcessTime[NumStages - 1];
    }

    for (i = 0; i < NumStages; i++)
    {
        for (j = 0; j < NumJobs; j++)
        {
            graph[i][j]->StageIndx = i;
            graph[i][j]->JobIndx = j;

            graph[i][j]->PathLength = 0;

            graph[i][j]->operation.stage = i;
            graph[i][j]->operation.job = j;
            graph[i][j]->operation.Weight = jobs[j].weight;
            graph[i][j]->operation.Release = 0;
            for (k = 0; k < NumJobs; k++) graph[i][j]->operation.Due[k] = 0;
            graph[i][j]->operation.Process = jobs[j].ProcessTime[i];

            // Note, seperate the special cases for the source and sinks
            // source is considered stage -1 and sinks stage NumStages

            graph[i][j]->ConPre->from = i - 1;
            graph[i][j]->ConPre->to = i;
            if (i == 0)
            {  // first stage

                graph[i][j]->ConPre->length = jobs[j].ReleaseTime;
                graph[i][j]->ConPre->neglength = -jobs[j].ReleaseTime;
            }
            else
            {
                graph[i][j]->ConPre->length = jobs[j].ProcessTime[i - 1];
                graph[i][j]->ConPre->length = -jobs[j].ProcessTime[i - 1];
            }

            graph[i][j]->ConNext->from = i;
            graph[i][j]->ConNext->to = i + 1;
            graph[i][j]->ConNext->length = jobs[j].ProcessTime[i];
            graph[i][j]->ConNext->neglength = -jobs[j].ProcessTime[i];

            // graph[i][j] ->DisNext = NULL;
            graph[i][j]->ExistDisNext = FALSE;
            // graph[i][j] ->DisPre = NULL;
            graph[i][j]->ExistDisPre = FALSE;
        }
        IsScheduled[i] = FALSE;
    }
}

// This function fixes the orietation of the disjunctive arcs at a stage
// once it's been solved by the B&B algorithm by converting the optimal
// schedule to the orientations of the DisArcs on the graph
void Graph::Fix_A_Stage(Node** stage, PartialSchedule* schedule, int size)
{
    Node* node;

    for (int i = 0; i < size; i++)
    {
        if (schedule[i].length == 0) continue;

        // seperate the first job in the partialschedule

        node = new Node;

        node = stage[schedule[i].sequence[0]];

        IsScheduled[node->StageIndx] = TRUE;

        if (schedule[i].length == 1)
        {  // only one job in this partialschedule

            node->ExistDisNext = FALSE;
            node->ExistDisPre = FALSE;
        }
        else
        {  // more than one job in this partialschedule

            node->ExistDisNext = TRUE;
            node->DisNext->from = schedule[i].sequence[0];
            node->DisNext->to = schedule[i].sequence[1];
            node->DisNext->length = node->operation.Process;
            node->DisNext->neglength = -node->DisNext->length;
            node->ExistDisPre = FALSE;

            for (int j = 1; j < schedule[i].length - 1; j++)
            {  // the loop is from the second job to the second to last job

                node = stage[schedule[i].sequence[j]];

                node->ExistDisNext = TRUE;
                node->DisNext->from = schedule[i].sequence[j];
                node->DisNext->to = schedule[i].sequence[j + 1];
                node->DisNext->length = node->operation.Process;
                node->DisNext->neglength = -node->DisNext->length;
                node->ExistDisPre = TRUE;
                node->DisPre->from = schedule[i].sequence[j - 1];
                node->DisPre->to = schedule[i].sequence[j];
                node->DisPre->length = stage[schedule[i].sequence[j - 1]]->operation.Process;
                node->DisPre->neglength = -node->DisPre->length;
            }

            // seperate the last job in the partialschedule
            node = stage[schedule[i].sequence[schedule[i].length - 1]];

            node->ExistDisNext = FALSE;
            node->ExistDisPre = TRUE;
            node->DisPre->from = schedule[i].sequence[schedule[i].length - 2];
            node->DisPre->to = schedule[i].sequence[schedule[i].length - 1];
            node->DisPre->length = stage[schedule[i].sequence[schedule[i].length - 2]]->operation.Process;
            node->DisPre->neglength = -node->DisPre->length;
        }  // else
    }
}

// Get the list of priorities of the stages based on the three criteria
// supposedly, the criteria is a weighted combination of workload, number of machines
// and the location of the stage.

// The weight parameter is something adjusted by testing

void Graph::Detect_Bottleneck_Stage()
{
    int i, j;
    int* workload;
    double center;          // the index of the middle stage of the shop
    double p1 = 1, p2 = 1;  // two weight parameters. this is just temparory value
    double* priority;       // bigger the value, higher the priority
    double totalmachine = 0.0;
    double biggest;
    int ThisStage;

    workload = new int[NumStages];
    priority = new double[NumStages];
    center = (NumStages + 1) / 2.0;

    for (i = 0; i < NumStages; i++)
    {
        workload[i] = 0;
        totalmachine += NumMachines[i];
        for (j = 0; j < NumJobs; j++) workload[i] += jobs[j].ProcessTime[i];
    }

    // Fill in the BottleList
    for (i = 0; i < NumStages; i++)
        //  priority[i] = p1*workload[i] + p2*(1/(NumMachines[i]/totalmachine)) + 1/(i-center);

        //     priority[i] = workload[i]/NumMachines[i];  //so far we only consider workload divided by # of
        //     machines since p1 and p2 would take a lot test to decide the right value

        priority[i] = workload[i];  // initial test shows total workload works better than workload/# machines

    for (j = 0; j < NumStages; j++)
    {
        biggest = 0.0;
        for (i = 0; i < NumStages; i++)
        {
            if (priority[i] > biggest)
            {
                ThisStage = i;
                biggest = priority[i];
            }
        }
        BottleList[j] = ThisStage;
        priority[ThisStage] = 0.0;
    }

    if (workload) delete[] workload;
    if (priority) delete[] priority;
}
