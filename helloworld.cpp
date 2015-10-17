#include "mpi.h"
#include "relevant_extractor.h"
#include <omp.h>
#include <stdio.h>
#include "phase2.h"
#include "mpiproperties.h"
#include "phase3.h"
#include "phase1.h"
#include "query.h"
#include <time.h>

using namespace std;

int rank, size;

void test_mpi();

int main(int argc, char *argv[]) {
  int perm;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &perm);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // test_phase2_mpi();

  // test_send_and_receive();
// /*
  time_t start,end;

  if (rank == 0) {
    time (&start);
  }

  run_phase1_mpi(argc, argv);

  if (rank == 0) {
    time (&end);
    double diff = difftime (end,start);
    printf("Phase 1 completed in %lf seconds\n", diff);
    time (&start);
  }

  // test_serial_traversal(argc, argv);



  run_phase2_mpi();

  if (rank == 0) {
    time (&end);
    double diff = difftime (end,start);
    printf("Phase 2 completed in %lf seconds\n", diff);
    time (&start);
  }

  run_phase3();

  if (rank == 0) {
    time (&end);
    double diff = difftime (end,start);
    printf("Phase 3 completed in %lf seconds\n", diff);
  }


  // printf("TELLLLLLLLLLLLLL\n");

  int qn;
  int i = 1;
  char q1[255], q2[255];

  string filelist = originaldir + "/data/query.txt";
  FILE *fp1;
  fp1 = fopen(filelist.c_str(), "r");

  while (fscanf(fp1, "%d", &qn) != EOF) {
    if (qn == 1) {
      fscanf(fp1, "%s", q1);
      // printf("%s\n", q1);

      if (rank == 0) {
        time (&start);
      }

      related_docs(string(q1), i);

      if (rank == 0) {
        time (&end);
        double diff = difftime (end,start);
        printf("Query %d completed in %lf seconds\n", i, diff);
      }
    }
    else if (qn == 2) {
      fscanf(fp1, "%s", q1);
      // printf("%s\n", q1);
      fscanf(fp1, "%s", q2);
      // printf("%s\n", q2);

      if (rank == 0) {
        time (&start);
      }

      common_to_both_docs(string(q1), string(q2), i);

      if (rank == 0) {
        time (&end);
        double diff = difftime (end,start);
        printf("Query %d completed in %lf seconds\n", i, diff);
      }
    }
    i++;
  }
// */
  // test_serial_traversal(argc, argv);
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
  if (rank == 0) {
    char buf[32];
    MPI_Status status;
    // receive message from any source
    MPI_Recv(buf, 32, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    printf("In process %d: Message:\n\"%s\"\n", rank, buf);
    char replybuf[32] = "message received";
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