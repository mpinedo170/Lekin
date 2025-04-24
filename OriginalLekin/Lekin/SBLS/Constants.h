// Constants.h:

// This file contains all the constants needed in the algorithm

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define TRUE 1
#define FALSE 0

#define K 1  // The scaling parameter in the AATC rule

#ifdef INFINITY
#undef INFINITY
#endif

const int INFINITY = 99999;  // Just a sign that this is a huge number
const double BIGNUMBER = 99999.0;

const int MAXSTAGE = 30;
const int MAXJOB = 100;
const int MAXMACHINE = 20;

const int SIZE = 70;
const int ADDSIZE = 20;     // the maximum number of eligible columns to add to RMP
const int DELETESIZE = 15;  // the maximum number of columns to remove from RMP

// the parameter in the AATC rule
const double KPar = 1.0;

// constants related to the call of Cplex
#define MACSZ SIZE
#define MARSZ MAXJOB + 1
#define MATSZ SIZE*(MAXJOB + 1)

#endif
