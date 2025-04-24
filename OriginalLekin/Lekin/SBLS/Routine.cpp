// routine.cxx:
// This file contains all those routine functions which
// will be called frequently by the algorithms

#include "StdAfx.h"

#include "Constants.h"
#include "StageBB.h"

class Node_Index;

int Position_Min(int* list, int Num);
double Average(int* list, int Num);
double pos(double s);
int pos(int s);
int in(int j, int* s, int length);
int in(int Stage, int Job, Node_Index* QUEUE, int head, int QLength);
int max(int i, int j);
int Metropolis_Test(int, int);
double Random();
int Uniform(int, int);

static int iseed = 0;

void Randomize()
{
    iseed = int(time(NULL));  // 886831295; //886831207;//time(NULL);
    // cout << "Random number seeed: " << iseed << "\n";
}

// This is a random number generator. output a random number between (0,1)
double Random()
{
    int k1;
    k1 = iseed / 127773;
    iseed = 16807 * (iseed - k1 * 127773) - k1 * 2836;
    if (iseed < 0) iseed = iseed + 2147483647;
    return (iseed * 4.656612875E-10);
}

// This function generates an integer value uniformly distributed between low and //high
int Uniform(int low, int high)
{
    int i;
    int range;
    double ran;

    ran = Random();
    range = high - low + 1;  // number of slots

    for (i = 1; i <= range; i++)
        if (ran > (i - 1) / double(range) && ran < i / double(range)) return (low + i - 1);
    return (low);
}

int Position_Min(int* list, int Num)
{
    int i = 0;
    int lowest = INFINITY;
    int position = 0;

    while (i < Num)
    {
        if (list[i] < lowest)
        {
            position = i;
            lowest = list[i];
        }
        i++;
    }
    return (position);
}

double Average(int* list, int Num)
{
    double sum = 0.0;
    int i;

    for (i = 0; i < Num; i++) sum += list[i];

    return (sum / Num);
}

double pos(double s)
{
    return (s > 0 ? s : 0.0);
}

int pos(int s)
{
    return (s > 0 ? s : 0);
}

int in(int j, const int* s, int length)
{
    int i = 0;
    int in = FALSE;

    while (i < length && s[i] != j) i++;

    if (i < length) in = TRUE;
    return (in);
}

int in(int Stage, int Job, Node_Index* QUEUE, int head, int QLength)
{
    int i = head;
    int in = FALSE;

    while (i < QLength && !(QUEUE[i].StageIndex == Stage && QUEUE[i].JobIndex == Job)) i++;

    if (i < QLength) in = TRUE;
    return (in);
}

// if in, return position; if not, return -1
int Position_In(int j, const int* s, int length)
{
    int i = 0;

    while (i < length && s[i] != j) i++;

    return i < length ? i : -1;
}

int max(int i, int j)
{
    return (i >= j ? i : j);
}

int Metropolis_Test(int OldValue, int NewValue)
{
    double TEMPERATURE = 50.0;
    double prob, u;

    if (NewValue >= OldValue)
    {
        // cout << "o-n=" << OldValue-NewValue <<endl;
        prob = exp((OldValue - NewValue) / TEMPERATURE);
        u = Random();
        // cout << "prob=" << prob << "u" << u << endl;
        return u < prob ? 1 : 0;
    }
    else  // new < old
        return (1);
}
