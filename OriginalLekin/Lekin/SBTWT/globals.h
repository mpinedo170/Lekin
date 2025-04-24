#ifndef GLOBALS_H
#define GLOBALS_H

// General parameters:

extern int par_option;
extern int par_objective;

extern char* par_name_file_static_data;
extern char* par_name_file_heuristic;
extern char* par_name_file_lower_bound;
extern char* par_name_file_results;

// Other parameters:

extern char* par_name_output_file;

extern int par_slack_active;
extern int par_look_ahead;

extern int par_print;
extern int par_scheduling_machine;
extern int par_animation_reoptimization;
extern int par_scheduling_operation;

// extern int par_accelerator_complete ;
// extern int par_min_machine ;
// extern int par_active_schedule ;

// Heuristic parameters:

extern int par_max_branches_main;
extern double par_threshold_main;
extern int par_reoptimization;

extern int par_max_branches_oracle;
extern double par_threshold_oracle;

extern double par_K;
extern double global_K;
// extern double par_B_oracle ;                   //Parameters for the ATC rule.

// Lower bound parameters:

extern int par_number_machines_spot_check;
extern int par_upper_bound;
extern int par_transition_speed;
extern int par_selection_rule;
extern int* par_optimal_sequence;
extern double par_lower_bound;
extern int par_max_number_unscheduled_one;
extern int par_max_number_bounds_one;
extern int par_max_number_unscheduled_two;
extern int par_max_number_bounds_two;
extern int par_max_number_unscheduled_three;
extern int par_max_number_bounds_three;
extern int par_max_number_unscheduled_four;
extern int par_max_number_bounds_four;
extern int par_max_number_unscheduled_five;
extern int par_max_number_bounds_five;

// Local Search parameters:

extern int par_sigma;
extern int par_max_number_centers;
extern int par_size_cluster_list;

// Global variables:

// extern FILE *par_output_file ;
// SOLEX extern int first_time ;
extern int first_time;
// SOLEX extern int hitting_time ;
extern int hitting_time;

// extern int par_flag_flow_shop ;

// Parameters for SLOW_INPUT:

extern char* par_name_file_graph;
extern char* par_name_file_plant;
extern char* par_name_file_clusters;

extern int Windows;
extern int contador;

#endif
