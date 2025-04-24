//File name: Cluster_Work_Center.C

#ifndef CLUSTER_WORK_CENTER
#	include "clus_cen.h"
#endif 


Cluster_Work_Center::Cluster_Work_Center()
{
	id = NULL_WORK_CENTER ;
	p_Work_Center = NULL ;
	number_machines = NULL_NUMBER ; 
	number_operations = NULL_NUMBER ;
	total_number_scheduled = 0 ;
	pred_node = NULL ;
//	succ_node = NULL ;
};

void Cluster_Work_Center::Initialize( 	int input_id , 
							int input_number_operations ,
							int input_number_machines )
{
int i , j ;

	id = input_id ;
	number_operations = input_number_operations ;
	number_machines = input_number_machines ;

//Allocating memory:
	operation = new Cluster_Operation[ number_operations ] ;

	total_number_scheduled = 0 ;
	machine_number_scheduled = new int[ number_machines ] ;
	sequence = new type_p_int[ number_machines ] ;
	pred_node = new type_p_Node[ number_machines ] ;
//3	succ_node = new type_p_Node[ number_machines ] ;
	finishing_time_machine = new int[ number_machines ] ;
	for( i = 0 ; i < number_machines ; i++){
		machine_number_scheduled[i] = 0 ;
		sequence[i] = new int[ number_operations ] ;
		for( j = 0 ; j < number_operations ; j++ ){	
			sequence[i][j] = NULL_LOCATION ;
		}
		pred_node[ i ] = NULL ;
//3		succ_node[ i ] = NULL ;
		finishing_time_machine[i] = NULL_TIME ;
	}
	min_release_time = 0 ;
}
	
void Cluster_Work_Center::Terminate( )
{
int i ;

//De-allocating memory:
	for( i = 0 ; i < number_machines ; i++ )
		delete[] sequence[i] ;
	delete[] operation ;
	delete[] sequence ;
	delete[] pred_node ;
//3	delete[] succ_node ;
	delete[] finishing_time_machine ;
}

void Cluster_Work_Center::Copy_Graph_Plant_Information( Graph *p_Graph , Plant *p_Plant )

{
int i , k , l ;	
int job_graph, step_graph;

	p_Work_Center = &( p_Plant->plant_Work_Centers[id] ) ;

	for( i = 0 ; i < number_operations ; i++ ){		
					//Choose the operation in the cluster.
									
		job_graph = operation[i].job ;
		step_graph = operation[i].step;
				
		operation[i].length = p_Graph->graph_Node[job_graph][step_graph].Length();

		operation[i].p_release_time = p_Graph->graph_Node[job_graph][step_graph].Get_p_Release_Time() ;

		operation[i].due_time = p_Graph->graph_Node[job_graph][step_graph + 1].Get_p_Due_Time() ;

		operation[i].p_machine = p_Graph->graph_Node[job_graph][step_graph].Get_p_Machine() ;
		
		operation[i].p_is_scheduled = p_Graph->graph_Node[job_graph][step_graph].Get_p_Is_Scheduled() ;

//		operation[i].p_complete = p_Graph->graph_Node[job_graph][step_graph].Get_p_Complete() ;

//Finds the work center for each operation:
		for( k = 0 ; k < p_Plant->number_work_centers ; k++ ){
			for( l = 0 ; l < p_Plant->plant_Work_Centers[k].number_operations ; l++ ){						
						//Find its corresponding work center.
							
				if (	(	job_graph == p_Plant->plant_Work_Centers[k].steps_Work_Center[l].job	)
						&&
					(	step_graph == p_Plant->plant_Work_Centers[k].steps_Work_Center[l].step	)){
						//Same job and same work center.
					
					operation[i].work_center = k ;
					p_Graph->graph_Node[job_graph][step_graph].Set_Work_Center( k ) ;
						//It also writes the work center in the graph.

					k = p_Plant->number_work_centers ;
					l = p_Plant->plant_Work_Centers[k].number_operations ;
						//To cut the double loop.
				}
			}
		}	
//

		p_Graph->graph_Node[job_graph][step_graph].Set_p_Min_Release_Time( &min_release_time ) ;
						//All the operations in the Cluster_Work_Center point to the same number.
	}
}

void Cluster_Work_Center::Clean_Information( Graph *p_Graph )

{
int i ;	
int job_graph, step_graph;

	for( i = 0 ; i < number_operations ; i++ ){		
					//Choose the operation in the cluster.
									
		job_graph = operation[i].job ;
		step_graph = operation[i].step;
				
		p_Graph->graph_Node[job_graph][step_graph].Set_p_Min_Release_Time( NULL ) ;
						//All the operations in the Cluster_Work_Center point to NULL.
	}
}

//Check_Recirculation( ): makes unavailable the recirculation operations, and
//writes the recirculation operations into the prevoius ones.
void Cluster_Work_Center::Check_Recirculation( )
{
int j , k ;	

	for( j = number_operations - 1 ; j >= 0 ; j-- ){						
						//Go backwards.
			
		for( k = j + 1 ; k < number_operations ; k++ ){
						//Scan forward.
			
			if (	operation[j].job == operation[k].job	){
						//Same job.

					#ifdef CHECK_DEBUG			
						assert( operation[j].work_center == operation[k].work_center ) ;
					#endif
						
				if (	operation[k].step <= operation[j].step ){
						printf("ERROR: operation[k].step <= operation[j].step\n");
                        exit(1);
						//Operations are sorted by their step number.
							
				} else {
					operation[j].location_recirc = k ;
					operation[k].is_available = FALSE ;
					k = number_operations ;
						//To cut the loop.
				}
			}
		}	
	}
}

//Delete_Disjunctives( Graph *the_Graph ) deletes the disjunctive arcs in the backtracking process.
void Cluster_Work_Center::Delete_Disjunctives( Graph *the_Graph )
{
int i ;
int job_from , step_from ;

	for( i = 0 ; i < number_operations ; i++){
			
		job_from = operation[i].job;
		step_from = operation[i].step;
				
		the_Graph->Delete_Disjunctive( job_from , step_from );
	}

//Deletes the arcs of the predecessors and updates min_release_time.
	min_release_time = INT_MAX ;
	for( i = 0 ; i < number_machines ; i++){
		if ( pred_node[ i ] != NULL ) {
			the_Graph->Delete_Disjunctive( pred_node[ i ]->Get_Job() , pred_node[ i ]->Get_Step() );
			finishing_time_machine[ i ] = pred_node[ i ]->Get_Finishing_Time() ;
		} else {
			finishing_time_machine[ i ] = 0 ;
		}
		if ( finishing_time_machine[ i ] < min_release_time ) {
			min_release_time = finishing_time_machine[ i ] ;
		}
	}
					#ifdef CHECK_DEBUG			
						assert( min_release_time < INT_MAX ) ;
					#endif
}


void Cluster_Work_Center::Print( int number_jobs )
{
int j , k ;			//counters.		

//	fprintf( par_output_file ,"work center id: %d ;", id );
	fprintf( par_output_file ,"number of machines: %d \n", number_machines );

	for( j = 0 ; j < number_operations ; j++){
		
		fprintf( par_output_file ,"length %d, window:%d -> ",
			 operation[j].length , 
			 *(operation[j].p_release_time));
			
		for( k = 0 ; k < number_jobs ; k++ )
			if ( operation[j].due_time[k].Exist())
				fprintf( par_output_file , "%d, " , operation[j].due_time[k].Time());
			else
				fprintf( par_output_file ,"_, ");
		if ( operation[j].is_available == FALSE )
			fprintf( par_output_file ," not available ");
		fprintf( par_output_file ,"\n");
	};
	fprintf( par_output_file ,"\n");
}


void Cluster_Work_Center::Schedule_Operation( 
				//Output paramenters.
									int *p_new_penalty ,
//									int *p_job_from , int *p_step_from ,

									Graph *p_Graph ,
			
				//Input paramenters.
									int scheduling_machine ,
									int location_operation ,
					
									int update )
{
//int i ;
int location_operation_from ;
int job_to , step_to ;
int job_from , step_from ;
					#ifdef CHECK_DEBUG			
						assert( operation[location_operation].is_available == TRUE ) ;	
					#endif
//Setting flags.
	operation[location_operation].Set_Machine_Is_Scheduled( scheduling_machine ) ;

	if ( operation[location_operation].location_recirc != NULL_JOB_STEP )
		operation[ operation[location_operation].location_recirc ].is_available = TRUE ;
					//Releases the predesessor of an operation.

//Recording sequence.
	sequence[ scheduling_machine ][ machine_number_scheduled[ scheduling_machine ] ] = location_operation ;
	machine_number_scheduled[ scheduling_machine ]++ ;
	total_number_scheduled++ ;
	p_Work_Center->total_number_scheduled++ ;
//
	if ( machine_number_scheduled[ scheduling_machine ] == 1 ){
						//Then this location_operation is the first one in the machine.
			
		if ( pred_node[ scheduling_machine ] != NULL ) {
			job_from = pred_node[ scheduling_machine ]->Get_Job() ;
			step_from = pred_node[ scheduling_machine ]->Get_Step() ;
		} else {
			job_from = NULL_JOB;
		}

						//There is no new partial penalty.
	} else {
			
		location_operation_from = sequence[ scheduling_machine ][ machine_number_scheduled[ scheduling_machine ] - 2 ] ;
						//Obtain the location of the operation that was scheduled 
						//before in the same machine of the work center.

		job_from = Nth_Job( location_operation_from ) ;
		step_from = Nth_Step( location_operation_from ) ;
	}
								
	job_to = Nth_Job( location_operation ) ; 
	step_to = Nth_Step( location_operation );
		
//	if ( ( job_from != job_to) && ( job_from != NULL_JOB ) ) {
	if ( job_from != NULL_JOB ) {
						//If it is a recirculating job.
						//There is no new partial penalty.

//Updating the graph.
		p_Graph->Add_Disjunctive( job_from , step_from , job_to , step_to );
	}

					#ifdef CHECK_DEBUG	
//	if ( par_option == 2 )		
//		p_Graph->Check_Lemma( job_to , step_to );
					#endif

}

void Cluster_Work_Center::Unschedule_Operation( 			
				//Input paramenters.
									int unscheduling_machine ,
									int location_operation ,
										//For modifying the cluster.
				
									Graph *p_Graph ,
										//For modifying the graph.

									int update 
									)
{
int job_from , step_from ;
int job_to = Nth_Job( location_operation ) ; 
int step_to = Nth_Step( location_operation );
//int dead_lock ;
				//These are not used.

//Setting flags.
	operation[ location_operation ].Unset_Machine_Is_Scheduled() ;

	if ( operation[ location_operation ].location_recirc != NULL_JOB_STEP)
		operation[(operation[location_operation].location_recirc)].is_available = FALSE ;
				//Make the predecessor unavailable.

//Recording sequence.
	p_Work_Center->total_number_scheduled-- ;
	total_number_scheduled-- ;
	machine_number_scheduled[ unscheduling_machine ]-- ;
	sequence[ unscheduling_machine ][ machine_number_scheduled[ unscheduling_machine ] ] = NULL_LOCATION ;

//Updating the graph and finishing times.
	if ( p_Graph->graph_Node[ job_to ][ step_to ].From_Exists() ) {

		job_from = p_Graph->graph_Node[ job_to ][ step_to ].From_Job( ) ;
		step_from = p_Graph->graph_Node[ job_to ][ step_to ].From_Step( ) ;

		p_Graph->Delete_Disjunctive(job_from , step_from) ;
				//Otherwise is was never added.

	}
}


//Chart::Next_Idle_Work_Center_Machine(): returns the first idle work center and machine.
void Cluster_Work_Center::Next_Idle_Work_Center_Machine( 	int *p_machine ,
												int *p_time_next_idle 
											)
{
int i ;
int min_release_time_unsched = INT_MAX ; 	//The minimum release time among the unscheduled operations.
int min_finishing_machines = INT_MAX ; 	//The minimum finishing time among the scheduled machines.
int location_last_operation ;			//Location last operation in a given machine.
int finishing_time ; 					//Finishing time of such operation.

	*p_machine = 0 ;				//Default value for the machine.
	*p_time_next_idle = INT_MAX ;

//This part gets the minimum release time of the operations assigned to
//the work center than can be scheduled.
	if ( total_number_scheduled < number_operations ){
					//If all operations have been scheduled then *p_time_next_idle = INT_MAX.

		for ( i = 0 ; i < number_operations ; i++ ) {
			if ( 	!operation[i].Is_Scheduled()
							//The operation has not been scheduled. 
				&& 
				operation[i].Is_Available() 
//				&& 
//				operation[i].Cluster_Predecessor_Is_Scheduled() 
							//The job-predecessor operation in the cluster has been already scheduled.
							//If the operations has no job-predessors in the cluster, 
							//Cluster_Predecessor_Is_Scheduled() = TRUE
				&& 
				( operation[i].Get_Release_Time() < min_release_time_unsched ) ){

					min_release_time_unsched = operation[i].Get_Release_Time() ;
			}
		}
//
//This part get the earliet time a machine in the work center can be used.
		for ( i = ( number_machines - 1 ) ; i >= 0 ; i-- ){
							//It goes backward so the ties are solved by choosing the
							//machine with the smallest index.

			if ( machine_number_scheduled[ i ] > 0 ){
				location_last_operation = sequence[ i ][ machine_number_scheduled[ i ] - 1 ] ;
				finishing_time = operation[ location_last_operation ].Get_Release_Time() 
							+ 
							operation[ location_last_operation ].Get_Length() ;
				if ( finishing_time < min_finishing_machines ){
					min_finishing_machines = finishing_time ;
					*p_machine = i ;
				}
			} else {
				min_finishing_machines = 0 ;
				*p_machine = i ;	
			}			
		}
//
		*p_time_next_idle =max( min_release_time_unsched , min_finishing_machines ) ;

	}	
}


void Cluster_Work_Center::Set_Mach_Pred( Graph *p_Graph , int machine , int input_pred_job , int input_pred_step ) 
{
	pred_node[ machine ] = &( p_Graph->graph_Node[input_pred_job][input_pred_step] ) ;
					//The Node predessor in machine 
					//is graph_Node[input_pred_job][input_pred_step]
}

void Cluster_Work_Center::Set_Mach_Succ( Graph *p_Graph , int machine , int input_succ_job , int input_succ_step ) 
{
//3		succ_node[ machine ] = &( p_Graph->graph_Node[input_succ_job][input_succ_step] ) ;
					//The Node successor in machine 
					//is graph_Node[input_pred_job][input_pred_step]
}

void Cluster_Work_Center::Update_Machines_Time( Graph *p_Graph ) 
{
int i ;
int operation_finishing_time ;
//Updating the finishing times.

	min_release_time = INT_MAX ;
	for( i = 0 ; i < number_machines ; i++)
		finishing_time_machine[ i ] = 0 ;

	for( i = 0 ; i < number_operations ; i++){
		if ( *( operation[ i ].p_is_scheduled ) ) {
			operation_finishing_time = 
					p_Graph->graph_Node[ operation[ i ].job ][ operation[ i ].step ].Get_Finishing_Time() ;
			if ( operation_finishing_time > finishing_time_machine[ *( operation[ i ].p_machine ) ] ) {
			
				finishing_time_machine[ *( operation[ i ].p_machine ) ] = operation_finishing_time ;
			}
		}
	}
	for( i = 0 ; i < number_machines ; i++) {
		if ( finishing_time_machine[ i ] < min_release_time ) {
			min_release_time = finishing_time_machine[ i ] ;
		}
	}
					#ifdef CHECK_DEBUG			
						assert( min_release_time < INT_MAX ) ;
					#endif
}
