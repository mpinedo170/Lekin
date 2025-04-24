/* File name: Constants */

// Augusto 13.8.96
// #include <windows.h>

#define CONSTANTS

// Augusto 19.8.96  para borland c++

#define TRUE 1
#define FALSE 0

#define MAX_JOBS 2000
/* maximum number of jobs */

// Augusto 13.11.1996 #define MAX_STEPS 20
#define MAX_STEPS 100
/* maximum number of steps */
/* per job */

#define NULL_JOB -1
#define NULL_STEP -1
#define NULL_JOB_STEP -1
#define NULL_LENGTH 9999
#define NULL_TIME -1
/* initializing constants */

#define NULL_NUMBER -1

#define NULL_WORK_CENTER -1
#define NULL_MACHINE -1

#define NULL_LOCATION -1

#define MAX_NUMBER_SUBPROBLEMS 20
#define MAX_PLANT_WORK_CENTER 20
/* maximum number of work centers in a plant. */

#define MAX_MACHINES_WORK_CENTER 10
/* maximum number of machines in a work centers. */

#define MAX_OPERATIONS_WORK_CENTER 20
/* DELETE from here. */

/* For Templ_Stack.h */

//  Augusto 13.11.96 #define SIZE_STACK      20
#define SIZE_STACK 2000

/* For Cluster.h */
#define MAX_CLUSTERS 20
#define MAX_OPERATIONS_CLUSTER 200

#define MAX_WORK_CENTERS_CLUSTER 20

#define HEURISTIC 0
#define LOWER_BOUND 1
#define LOCAL_SEARCH 2

#define MAKESPAN 0
#define TARDINESS 1
#define COMPLETION 2

#define HEURISTIC_ORACLE 0
#define EXACT_ORACLE 1
#define BOUND_ORACLE 2

#define CHECK_TRUE 523
#define CHECK_FALSE 427

#define DELTA_SIGMA 10

//#define QUICK         242
//#define EARLIEST      336
//#define SECOND                576

#define SELECTED 111
#define FORBIDDEN 222
#define REDUNDANT 333
#define BLK_FIRST 444
#define BLANK 555
#define DIAGONAL 666

#define TEMP 1
#define NO_TEMP 0

#define NUMBER_VERSIONS 8
#define NUMBER_INSTANCES 22

#define EARLY 0
#define ESTIMATE 1
#define BOTTLE 2
#define BOUND 3
