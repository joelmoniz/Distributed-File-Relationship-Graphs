#include "relevant_extractor.h"
#include "mpi.h"
#include <omp.h>
#include <map>
#include <set>
#include <stdio.h>
#include <sstream>
#include <queue>
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include "phase2.h"
#include "phase3.h"
#include "mpiproperties.h"
#include <stdexcept>
#include "query.h"

void related_docs(string doc, int qnum) {
  if (rank == 0) {
    for (int r = 1; r < size; r++)
      MPI_Barrier(MPI_COMM_WORLD);
    return;
  }

  vector<string> v;

  int node = file_to_number_mapping[doc];

  for (int i = 0; i < adj_matrix_chunk.size(); i++) {
      if (adj_matrix_chunk[i][node] > 10) {
        v.push_back(number_to_file_mapping[i + node_first_file]);
      }
  }

  stringstream sstm;
  sstm << "/data/query" << qnum << ".txt";
  string filelist =  originaldir + sstm.str();

  for (int r = 1; r < size; r++) {
    // i++;
    if (r == rank) {
      if (r==1) {
        printf("File number mapping is: %s -> %d\n", doc.c_str(), file_to_number_mapping[doc]);
      }
      printf("Printing query %d from rank %d\n", qnum, rank);
      FILE *fp1;
      fp1 = fopen(filelist.c_str(), "w");
      for (int x = 0; x < v.size(); x++) {
        fprintf(fp1, "%s\n", v[x].c_str());
      }
      if (fp1 != NULL)
        fclose(fp1);
    }
    // i--;
    MPI_Barrier(MPI_COMM_WORLD);
  }
}

void common_to_both_docs(string doc1, string doc2, int qnum) {
  if (rank == 0) {
    for (int r = 1; r < size; r++)
      MPI_Barrier(MPI_COMM_WORLD);
    return;
  }

  vector<string> v;

  int node1 = file_to_number_mapping[doc1];
  int node2 = file_to_number_mapping[doc2];

  for (int i = 0; i < adj_matrix_chunk.size(); i++) {
      if (adj_matrix_chunk[i][node1] > 10 && adj_matrix_chunk[i][node2] > 10) {
        v.push_back(number_to_file_mapping[i + node_first_file]);
      }
  }

  stringstream sstm;
  sstm << "/data/query" << qnum << ".txt";
  string filelist =  originaldir + sstm.str();

  for (int r = 1; r < size; r++) {
    // i++;
    if (r == rank) {
      if (r==1) {
        printf("File number mapping is: %s -> %d\n", doc1.c_str(), file_to_number_mapping[doc1]);
        printf("File number mapping is: %s -> %d\n", doc2.c_str(), file_to_number_mapping[doc2]);
      }
      printf("Printing query %d from rank %d\n", qnum, rank);
      FILE *fp1;
      fp1 = fopen(filelist.c_str(), "w");
      for (int x = 0; x < v.size(); x++) {
        fprintf(fp1, "%s\n", v[x].c_str());
      }
      if (fp1 != NULL)
        fclose(fp1);
    }
    // i--;
    MPI_Barrier(MPI_COMM_WORLD);
  }
}
