#include "mpi.h"
#include <iostream>
#include <string>
#include <unistd.h>

#define DEBUG 							true
#define MSG_STATE 						100
#define MSG_SIZE 						1
#define INIT_NODE						0
#define DELAY							1
#define CRITICAL_SECTION_SLEEP			1	
#define SIMULATION_ROUND_START			4
#define START_STATE						0
#define MSG_ROUND						101

using namespace std;

int node_id;
int size;
int receiver;
int state;
int neighbor_state;
int round_num;
bool critical_section;
int k;

void print_debug_message (const char* message);
void send_state_msg ();
void wait_for_state_msg ();
void print_state ();
void print_simulation_state();

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &node_id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

  	receiver = (node_id + 1) % size;
  	
  	state = START_STATE;
  	round_num = 0;
  	critical_section = false;
  	k = size + 1;

  	srand (time(NULL) + node_id);

  	//print_state();

  	MPI_Barrier(MPI_COMM_WORLD);

  	send_state_msg();
  	wait_for_state_msg();

  	MPI_Barrier(MPI_COMM_WORLD);

	while (true) {
		if (round_num == SIMULATION_ROUND_START) {
			state = rand() % k;
			print_simulation_state ();
			round_num++;

			send_state_msg ();
		}

		if (node_id == INIT_NODE && state == neighbor_state) {
			critical_section = true;
			state = (state + 1) % k;
		} else if (node_id != INIT_NODE && state != neighbor_state) {
			critical_section = true;
			state = neighbor_state;
		}

		if (critical_section) {
			//print_debug_message("Entered critical section");
			sleep(CRITICAL_SECTION_SLEEP);
			critical_section = false;
			//print_debug_message("Left critical section");
			print_state();
			send_state_msg ();
		}

		wait_for_state_msg ();
	}

	MPI_Finalize();
}

void print_debug_message (const char* message) {
	if (DEBUG)
		cout << "Node [" << node_id << "][" << round_num << "]: " << message << endl;
}

void print_simulation_state () {
	if (DEBUG)
		cout << "Node [" << node_id << "][" << round_num << "]: simulated state -> " << state << endl; 
}

void print_state () {
	if (DEBUG)
		cout << "Node [" << node_id << "][" << round_num << "]: state -> " << state << endl; 
}

void send_state_msg () {
	int data = state;
	MPI_Request request;

	MPI_Isend(&data, MSG_SIZE, MPI_INT, receiver, MSG_STATE, MPI_COMM_WORLD, &request);
}

void wait_for_state_msg () {
	int data, flag;
	MPI_Status status;

	MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		
	if (flag) {
		MPI_Recv(&data, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		if (status.MPI_TAG == MSG_STATE) {
			neighbor_state = data;	
			round_num++;
		} 
	}
}