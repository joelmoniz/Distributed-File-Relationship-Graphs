#include "mpi.h"
#include "relevant_extractor.h"
#include <omp.h>
#include <stdio.h> 
#include "phase2.h"
#include "mpiproperties.h"

using namespace std;

int rank;

void test_mpi();

int main(int argc, char *argv[]) {
  int size, perm;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &perm);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  test_phase2_mp();
  // test_mpi();

  MPI_Finalize();
}

/*
int main(int argc, char *argv[]) {
  // get_relevant_words_from_files();
  test_phase2_mp();
}
*/

/*
int main(int argc, char *argv[]) {
  omp_set_num_threads(omp_get_num_procs());

  #pragma omp parallel 
  {
    // #pragma omp master
    // while(1);

    // printf("Test\n");
    test_relevant();
    // test_relevant();
    // test_relevant();
    // test_relevant();
    
    
  }
  // test_relevant();
}
*/

void test_mpi() {
  if (rank==0) {
    char buf[32];
    MPI_Status status;
    // receive message from any source
    MPI_Recv(buf, 32, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    printf("In process %d: Message:\n\"%s\"\n", rank, buf);
    char replybuf[32]= "message received";
    // send reply back to sender of the message received above
    MPI_Send(replybuf, 32, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
  }
  else {
    char buf[255] = "this is a test";
    MPI_Send(buf, 32, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    MPI_Status status;
    // receive message from any source
    MPI_Recv(buf, 255, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
    printf("In process %d: Message:\n\"%s\"\n", rank, buf);
  }
}