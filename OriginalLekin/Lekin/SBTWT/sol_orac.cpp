#include "StdAfx.h"

#include "sol_orac.h"

// Class_Solution_Oracle::Class_Solution_Oracle( Cluster *input_p_Cluster )
// Class_Solution_Oracle::Class_Solution_Oracle( int id , int input_number_work_centers , int  )
Class_Solution_Oracle::Class_Solution_Oracle()
{
    value = 0;
    time_out = FALSE;
    cluster_id = -1;

    number_work_centers = 0;
    number_machines_work_center = NULL;

    length_solution = NULL;
    solution = NULL;
}

void Class_Solution_Oracle::Terminate()
{
    int i, j;

    if (number_machines_work_center != NULL)
    {
        // If there is memory allocated
        for (i = 0; i < number_work_centers; i++)
        {
            for (j = 0; j < number_machines_work_center[i]; j++)
            {
                delete[] solution[i][j];
            }
            delete[] length_solution[i];
            delete[] solution[i];
        }
        delete[] length_solution;
        delete[] solution;
        delete[] number_machines_work_center;
    }
}

void Class_Solution_Oracle::operator=(const Class_Solution_Oracle& from)
{
    int i, j, k;

    value = from.value;
    time_out = from.time_out;
    cluster_id = from.cluster_id;
    number_work_centers = from.number_work_centers;

#ifdef CHECK_DEBUG
    assert(number_machines_work_center != NULL);
    assert(length_solution != NULL);
    assert(solution != NULL);
#endif

    // Copying the information:
    for (i = 0; i < number_work_centers; i++)
    {
        number_machines_work_center[i] = from.number_machines_work_center[i];
        // CHECK! whether necessary.

        for (j = 0; j < number_machines_work_center[i]; j++)
        {
            length_solution[i][j] = from.length_solution[i][j];
            for (k = 0; k < length_solution[i][j]; k++)
            {
                solution[i][j][k] = from.solution[i][j][k];
            }
        }
    }
    //
}

int Class_Solution_Oracle::operator==(const Class_Solution_Oracle& from)
{
    int i, j, k;

    if ((value != from.value) || (cluster_id != from.cluster_id) ||
        (number_work_centers != from.number_work_centers))
        return (FALSE);

    for (i = 0; i < number_work_centers; i++)
    {
        if (number_machines_work_center[i] != from.number_machines_work_center[i]) return (FALSE);

        for (j = 0; j < number_machines_work_center[i]; j++)
        {
            if (length_solution[i][j] != from.length_solution[i][j]) return (FALSE);
            for (k = 0; k < length_solution[i][j]; k++)
            {
                if (solution[i][j][k] != from.solution[i][j][k]) return (FALSE);
            }
        }
    }
    return (TRUE);
}

void Class_Solution_Oracle::Write(
    int location_scheduling_work_center, int scheduling_machine, int location_operation)
{
#ifdef CHECK_DEBUG
    assert(location_scheduling_work_center < number_work_centers);
    assert(scheduling_machine < number_machines_work_center[location_scheduling_work_center]);
#endif

    solution[location_scheduling_work_center][scheduling_machine]
            [length_solution[location_scheduling_work_center][scheduling_machine]] = location_operation;

    length_solution[location_scheduling_work_center][scheduling_machine]++;
};

void Class_Solution_Oracle::Unwrite(
    int location_scheduling_work_center, int scheduling_machine, int location_operation)
{
    length_solution[location_scheduling_work_center][scheduling_machine]--;

#ifdef CHECK_DEBUG
    assert(solution[location_scheduling_work_center][scheduling_machine]
                   [length_solution[location_scheduling_work_center][scheduling_machine]] ==
           location_operation);
#endif

    solution[location_scheduling_work_center][scheduling_machine]
            [length_solution[location_scheduling_work_center][scheduling_machine]] = NULL_JOB;
};
// Nth_Operation(): Obtain the location in the set of operations of the operation
// with a given postition in a machine of a work center.
int Class_Solution_Oracle::Nth_Operation(int location_work_center, int machine, int position)
{
#ifdef CHECK_DEBUG
    assert(position >= 0);
    assert(position < length_solution[location_work_center][machine]);
#endif

    return (solution[location_work_center][machine][position]);
};

// void Class_Solution_Oracle::Print( Cluster *p_Cluster )
/*void Class_Solution_Oracle::Print(  )
{
int i , j , k ;

        fprintf( par_output_file ,"cluster : %d ; value: %d \n", cluster_id , value );
        for ( i = 0 ; i < number_work_centers ; i++ ){
                for ( j = 0 ; j < number_machines_work_center[i] ; j++ ){
                        if ( length_solution[i][j] > 0 ){
                                fprintf( par_output_file , "w.c. %d , mach. %d :" , cluster_id , j );
                                for ( k = 0 ; k < length_solution[i][j] ; k++){
                                        fprintf( par_output_file ,"%d - ", solution[i][j][k] ) ;
                                }
                                                                                  fprintf( par_output_file
,"\n");
                        }
                }
                  }
}*/

int Class_Solution_Oracle::Is_Optimal_Predecessor()
{
    static int optimal[6][6] = {{1, 3, 0, 4, 5, 2}, {3, 0, 4, 1, 2, 5}, {0, 1, 3, 5, 2, 4},
        {1, 3, 0, 2, 4, 5}, {1, 5, 0, 3, 2, 4}, {1, 3, 0, 4, 5, 2}};

    int i, k;

    if ((value > 386) || (number_work_centers < 6)) return (FALSE);
    for (i = 0; i < number_work_centers; i++)
    {
        for (k = 0; k < length_solution[i][0]; k++)
        {
            if (solution[i][0][k] != optimal[i][k]) return (FALSE);
        }
    }
    return (TRUE);
}
/*
        if (    ( value > 786 )
{1,3,0,4,2,5},
{3,1,2,4,0,5},
{1,2,3,0,5,4},
{5,2,1,3,0,4},
{2,1,5,3,0,4},
{1,2,5,0,3,4}};
*/
/*
        if (    ( value > 415 )
{3,4,5,2,0,1},
{0,5,1,3,2,4},
{5,1,3,2,0,4},
{5,3,0,2,1,4},
{3,1,0,5,4,2},
{5,3,2,1,0,4}};
*/
/*
        if (    ( value > 20 )
                ||
                ( number_work_centers < 4 )
        )
{0,3,2,1},
{1,0,2,3},
{0,1,2,-1},
{1,2,0,-1}};
*/
/*
        if (    ( value > 556 )
                ||
                ( number_work_centers < 4 )
        )
{2,1,0,3,4,5},
{3,2,0,5,1,4},
{3,1,0,2,5,4},
{4,1,3,2,0,5},
{0,1,3,2,5,4},
{1,0,2,3,5,4}};
*/
/*
        if (    ( value > 925 )
                ||
                ( number_work_centers < 7 )
                  )
{2,4,1,3,0,6,5},
{3,2,6,5,0,1,4},
{3,4,1,2,0,5,6},
{4,1,3,2,6,0,5},
{0,4,3,1,2,6,5},
{1,2,0,3,4,5,6},
{1,2,0,3,4,5,6}};
*/
/*
        if (    ( value > 704 )
                ||
                ( number_work_centers < 6 )
        )
{0,1,4,5,3,2},
{0,5,4,3,2,1},
{5,4,0,1,3,2},
{0,5,4,1,2,3},
{0,1,4,5,3,2},
{5,4,0,1,2,3}} ;
*/
/*
        if (    ( value > 20 )
                ||
                ( number_work_centers < 4 )
        )
                                        { 0 , 3 , 2 , 1 } ,
                                        { 1 , 0 , 2 , 3 } ,
                                        { 0 , 1 , 2 , -1 } ,
                                        { 1 , 2 , 0 , -1 } }      ;
*/
/*
        if (    ( value > 1353 )
                                {       { 3 , 1 , 0 , 4 , 5 , 6 , 2 } ,
                                        { 4 , 3 , 0 , 1 , 6 , 5 , 2 } ,
                                        { 6 , 0 , 1 , 3 , 5 , 4 , 2 } ,
                                        { 6 , 1 , 3 , 0 , 4 , 5 , 2 } , //1 , 3
                                        { 3 , 1 , 0 , 4 , 6 , 5 , 2 } ,
                                        { 4 , 3 , 0 , 6 , 1 , 5 , 2 } ,
                                        { 3 , 5 , 4 , 0 , 6 , 1 , 2 } } ;
*/
/*
        if (    ( value > 1284 )
                                {       { 2,1,6,4,3,5,0 } ,
                                        { 4,2,3,6,0,1,5 } ,
                                        { 6,2(4,1,0,5,3 } ,
                                        { 2,6,1,4,3,0,5 } ,
                                        { 1,4,3,2,6,0,5 } ,
                                        { 4,2,6,1,0,3,5 } ,
                                        { 5,4,2,3,0,1,6 } } ;
*/

/*
        if (    ( value > 905 )
                {       { 6 , 0 , 1 , 5 , 4 , 2 , 3 } ,
                        { 0 , 3 , 6 , 2 , 1 , 5 , 4 } ,
                        { 5 , 6 , 1 , 0 , 2 , 4 , 3 } ,
                        { 6 , 5 , 2 , 3 , 0 , 1 , 4 } ,
                        { 1 , 5 , 0 , 2 , 6 , 4 , 3 } ,
                        { 5 , 1 , 0 , 4 , 6 , 2 , 3 } ,
                        { 0 , 5 , 6 , 1 , 2 , 4 , 3 } } ;
*/
/*
 */
/*static int optimal[8][8] =  {         { 0 , 4 , 6 , 1 , 7 , 5 , 2 , 3 } ,
                                        { 6 , 0 , 5 , 4 , 3 , 2 , 7 , 1 } ,
                                        { 4 , 7 , 5 , 0 , 1 , 6 , 3 , 2 } ,
                                        { 6 , 0 , 5 , 4 , 1 , 2 , 7 , 3 } ,
                                        { 0 , 1 , 4 , 6 , 5 , 7 , 3 , 2 } ,
                                        { 5 , 4 , 0 , 6 , 7 , 1 , 2 , 3 } ,
                                        { 6 , 0 , 5 , 1 , 4 , 7 , 3 , 2 } ,
                                        { 6 , 0 , 4 , 5 , 1 , 2 , 7 , 3 } } ;
int i ,  k;

        if (    ( value > 1237 )
                ||
                ( number_work_centers < 8 )
        )
*/
