#include "mpi.h"
#include <unistd.h>
#include <signal.h>
#include "mpiproperties.h"

MPI_Request req;

using namespace std;

void end_wait(int sig) {
  // printf("Cancelling %d...\n", sig);
  MPI_Cancel(&req);
}

// TODO: Should it stay put and keep waiting until timer is up,
// as opposed to returning as soon as first command is received?
int timed_request_for_communication(int &src) {

  int files = TIME_OUT;
  MPI_Irecv(&files, 1, MPI_INT, MPI_ANY_SOURCE, INTER_SLAVE_TAG, MPI_COMM_WORLD, &req);
  
  struct sigaction sa;
  sa.sa_handler = &end_wait;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  if (sigaction(SIGALRM, &sa, 0) == -1) {
    perror("Unable to set SIGALRM action");
  }

  MPI_Status status;

  alarm(STAY_PUT_SLEEP_TIME);
  MPI_Wait(&req, &status);
  alarm(0);

  if (files != TIME_OUT) {
    src = status.MPI_SOURCE;
    // printf("No time out thanks to %d\n", src);
  }

  return files;
}