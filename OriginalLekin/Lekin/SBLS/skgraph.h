/************************************************************************/
/*									*/
/*									*/
/*			    G R A P H . H				*/
/*									*/
/*									*/
/************************************************************************/


void construct_graph ();

void init_longest_path ();

int find_longest_path ();

void save_lp ();

int get_lp (struct graph_node *);

void get_result ();

void free_memory ();

void save_actual_schedule ();

void save_actual_schedule_as_best_workload_schedule ();

void delete_act_job_order_in_graph ();

void read_best_job_order_in_graph ();

void read_best_job_order_for_this_workload_in_graph ();

void read_act_job_order_in_graph ();

void control2 ();

