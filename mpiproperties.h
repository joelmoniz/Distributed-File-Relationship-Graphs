#ifndef MPI_PROPS_
#define MPI_PROPS_

#define MASTER_TAG 0
#define INTER_SLAVE_TAG 1
#define LOCKSTEP_TAG 2

#include <string>


#define END_PHASE -2
#define STAY_PUT -1
#define NEXT_PHASE_LOCKSTEP -4

#define TIME_OUT -3

#define STAY_PUT_SLEEP_TIME 1

using namespace std;
extern int rank;

// NOTE: One extra process needed for communication
extern int size;

extern string originaldir;


int timed_request_for_communication(int &src);

#endif