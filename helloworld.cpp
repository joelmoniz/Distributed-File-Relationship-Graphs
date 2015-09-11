// #include "mpi.h"
#include "relevant_extractor.h"
#include <omp.h>
#include <stdio.h> 
#include "phase2.h"

using namespace std;


/*
int main(int argc, char *argv[]) {
  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // if (rank == 0) {
  //   MPI_Comm_size(MPI_COMM_WORLD, &size);
  //   printf("No. of processes created %d\n",size);
  // }
  // else 
  //   printf("Hello World from process %d\n", rank);

  test_relevant();


  MPI_Finalize();
}
*/

int main(int argc, char *argv[]) {
  // get_relevant_words_from_files();
  test_phase2_mp();
}

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