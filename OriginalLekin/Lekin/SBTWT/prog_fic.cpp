#include <stdio.h>

// Programa operaciones ficticias, a partir de los resultados de la anterior programacion
void Prog_fict(char *name_file_input )
{       
//long int i , j , k , l , m;
int i , j , k, l ;
int min_job , min_step ;                        //Job and steps with the minimum release time in the current iteration.
int min_release_time ;                  //Minimum release time of the current iteration.
int finishing_time ;
int total_num_jobs;
int id, number_steps, release_time, due_date, completion_time_penalty, completion_time;
int number_work_centers;
int number_machines;
int number_operations_machine ; //Number of operation scheduled in the machine of the current iteration.
int Weighted_Tardiness, Makespan; 
FILE *this_file;

int *count_operations;

// ****  LECTURA DE DEL MAKESPAN *****************
        this_file = fopen(name_file_input, "r") ;

        fscanf(this_file,"%ld", &total_num_jobs);
        count_operations = new int[total_num_jobs];

        for( i = 0 ; i < total_num_jobs ; i++ ){
                fscanf(this_file,"%ld", &id);
                fscanf(this_file,"%ld", &number_steps);
                fscanf(this_file,"%ld", &release_time);
                fscanf(this_file,"%ld", &due_date);
                fscanf(this_file,"%ld", &completion_time_penalty);
                fscanf(this_file,"%ld", &completion_time);
                count_operations[i] = number_steps;
        }

                        //This part read the information about the operations.
        fscanf(this_file,"%ld", &number_work_centers);
        for( i = 0 ; i < number_work_centers ; i++ ){
                fscanf( this_file , "%ld" , &number_machines ) ;
                for( j = 0 ; j < number_machines ; j++ ){
                        fscanf( this_file , "%ld" , &number_operations_machine ) ;
                        for ( l = 0 ; l < number_operations_machine ; l++ ){
                            fscanf( this_file , "%ld" , &min_job ) ;
                            fscanf( this_file , "%ld" , &min_step ) ;
                            fscanf( this_file , "%ld" , &min_release_time ) ;
                            fscanf( this_file , "%ld" , &finishing_time ) ;
                        }                              
                }
        }

        fscanf( this_file , "%ld" , &Weighted_Tardiness ) ;
        fscanf( this_file , "%ld" , &Makespan );
        printf("\n WT %d      Ms %d ", Weighted_Tardiness, Makespan );

        fclose(this_file);


        new_release_time = (int)( (Makespan - new_release_time) / 2.0);


// ********* PROGRAMACION DE LAS OPERACIONES FICTICIAS **********
        this_file = fopen(name_file_input, "r") ;

        fscanf(this_file,"%ld", &total_num_jobs);
        count_operations = new int[total_num_jobs];

        for( i = 0 ; i < total_num_jobs ; i++ ){
                fscanf(this_file,"%ld", &id);
                fscanf(this_file,"%ld", &number_steps);
                fscanf(this_file,"%ld", &release_time);
                fscanf(this_file,"%ld", &due_date);
                fscanf(this_file,"%ld", &completion_time_penalty);
                fscanf(this_file,"%ld", &completion_time);
                count_operations[i] = number_steps;
        }

int entro;
int termino_anterior;

        fscanf(this_file,"%ld", &number_work_centers);
        for( i = 0 ; i < number_work_centers ; i++ ){
                fscanf( this_file , "%ld" , &number_machines ) ;
                for( j = 0 ; j < number_machines ; j++ ){
                        fscanf( this_file , "%ld" , &number_operations_machine ) ;
                        termino_anterio = 0;
                        entro = 0;
                        for ( l = 0 ; l < number_operations_machine ; l++ ){
                            fscanf( this_file , "%ld" , &min_job ) ;
                            fscanf( this_file , "%ld" , &min_step ) ;
                            fscanf( this_file , "%ld" , &min_release_time ) ;
                            fscanf( this_file , "%ld" , &finishing_time ) ;
                            if ( (finishing_time > new_release_time) && ( !entro ) ) {
                               entro = 1;
                               //Programar_operacion en maq = j, inicio = 0 termino = termino_anterior
                            }
                            termino_anterior = finishing_time;
                        }
                        if ( !entro && l > 0 ) {
                           //Programar_operacion maq = j, inicio = 0, termino = termino_anterior
                        }
                }
        }

        fclose(this_file);


}

