// Input.h
// In this file, we declare the classes and data structures needed in Input.cxx
#ifndef INPUT_H
#define INPUT_H

#include "Constants.h"

class Graph;
class Data;
extern void Decomposition_Heuristic(const Data&);

class PartialSchedule;

class Data
{
    // Data about the plant
    int NumStages;
    int NumJobs;
    int* machines;  // the array of the number of machines at each stage

    // Data about the jobs
    int* ReleaseTime;
    int** ProcessTime;
    int* weight;
    int* DueTime;

    // Parameters needed
    double fpar;  // the parameter to decide the due dates

    // friend void Graph::Buildup_Graph(const Data&);
    friend class Graph;
    friend void Decomposition_Heuristic(const Data&);
    friend void Save(const Data&, PartialSchedule** BestSchedule);
    friend void Pass_To_sbls(const Data&);

public:
    Data();
    ~Data();
    void Initialize(char* FileName);
    void InitializeMachine(char* FileName);
    void InitializeJob(char* FileName);
    void Initialize();
    void Generate_ReleaseTime();
    void Generate_ProcessTime();
    void Generate_Weight();
    void Cal_DueTime();
};

#endif
