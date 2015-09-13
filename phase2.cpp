#include "relevant_extractor.h"
#include "mpi.h"
#include <omp.h>
#include <map>
#include <set>
#include <stdio.h>
#include <queue>
#include <algorithm>
#include <unistd.h>
#include "phase2.h"
#include "mpiproperties.h"

using namespace std;

map<string, set<string> > get_relevant_words_from_files() {

  // the one is the master, handling communication, etc.
  printf("%d processors\n", omp_get_num_procs());
  setup_stopwords();

  string filelist = "files.txt";

  int total_size = 0;

  queue<pair<string, int> > file_queue = load_file_list(filelist, total_size);

  return slave_relevant_find(file_queue, total_size);
}

map<string, set<string> > slave_relevant_find(queue<pair<string, int> > &file_queue, int total_size) {

  printf("%d\n", total_size);

  bool done = false;
  omp_set_num_threads(omp_get_num_procs() + 1);
  map<string, set<string> > m;

  #pragma omp parallel shared(file_queue, m, done)
  {
    int is_master_thread = 0;

    MPI_Is_thread_main(&is_master_thread);

    // TODO: Instead of taking x files, take based on size
    if (is_master_thread)
    {
      while (!done) {
        // Send status to master, receive update
        // node_files => node id, number of files to request
        int node_files[2];
        MPI_Send(&total_size, 1, MPI_INT, 0, MASTER_TAG, MPI_COMM_WORLD);
        MPI_Status status;
        MPI_Recv(node_files, 2, MPI_INT, 0, MASTER_TAG, MPI_COMM_WORLD, &status);

        if (node_files[0] == END_PHASE) {
          done = true;
          break;
        }
        else if (node_files[0] == STAY_PUT) {
          int src, num_requested_files;
          num_requested_files = timed_request_for_communication(src);

          if (num_requested_files == TIME_OUT)
            continue;
          else {
            vector<pair<string, int> > fl(num_requested_files);
            int actual;
            #pragma omp critical(queuepop)
            {
              actual = min((int)file_queue.size(), num_requested_files);
              for (int i = 0; i < num_requested_files; ++i)
              {
                fl[i] = file_queue.front();
                file_queue.pop();
              }
            }

            MPI_Send(&actual, 1, MPI_INT, src, INTER_SLAVE_TAG, MPI_COMM_WORLD);

            for (int i = 0; i < num_requested_files; ++i)
            {
              MPI_Send(fl[i].first.c_str(), fl[i].first.length(), MPI_CHAR, src, INTER_SLAVE_TAG, MPI_COMM_WORLD);
              MPI_Send(&fl[i].second, 1, MPI_INT, src, INTER_SLAVE_TAG, MPI_COMM_WORLD);
            }

          }
        }
        else {
          // ping node, get back reply

          int actual_size;

          MPI_Send(&node_files[1], 1, MPI_INT, node_files[0], INTER_SLAVE_TAG, MPI_COMM_WORLD);
          MPI_Recv(&actual_size, 1, MPI_INT, node_files[0], INTER_SLAVE_TAG, MPI_COMM_WORLD, &status);

          if (actual_size == END_PHASE)
            continue;

          vector<pair<string, int> > f(actual_size);
          char fname[MAX_PATH_SIZE];
          int sz;

          // TODO: Make more efficient by wrapping into a structure
          for (int i = 0; i < actual_size; i++) {
            MPI_Recv(fname, MAX_PATH_SIZE, MPI_CHAR, node_files[0], INTER_SLAVE_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&sz, 1, MPI_INT, node_files[0], INTER_SLAVE_TAG, MPI_COMM_WORLD, &status);
            f[i] = make_pair(string(fname), sz);
          }

          #pragma omp critical(queuepop)
          {
            for (int i = 0; i < actual_size; i++) {
              file_queue.push(f[i]);
            }
          }
          sleep(1);
        }
      }

      int x = 0;
      MPI_Status status;
      while (x != NEXT_PHASE_LOCKSTEP) {
        MPI_Recv(&x, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (x == NEXT_PHASE_LOCKSTEP)
          break;
        else {
          int stat = END_PHASE;
          MPI_Send(&stat, 1, MPI_INT, status.MPI_SOURCE, INTER_SLAVE_TAG, MPI_COMM_WORLD);
        }
      }
    }
    else {
      bool q_not_empty = true;

      // TODO: Handle scenario when more files exist to be traversed,
      // but master hasn't assigned them yet
      while (q_not_empty || !done) {
        // printf("Here\n");
        pair<string, int> file_queue_pair;
        string file;
        int file_size;
        #pragma omp critical(queuepop)
        {
          if (!file_queue.empty()) {
            file_queue_pair = file_queue.front();
            file = file_queue_pair.first;
            file_size = file_queue_pair.second;
            total_size -= file_size;
            file_queue.pop();
            q_not_empty = true;
          }
          else
            q_not_empty = false;
        }

        if (q_not_empty) {
          set<string> rel = get_relevant_words(file);

          #pragma omp critical(mapupdate)
          m[file] = rel;
          printf("%s\n", file.c_str());
        }
      }
    }
  }
  printf("Done file size: %d\n", total_size);
  return m;
}


// TODO: Make things like sorting, getting maximum, checking for non-zero values
// more efficient
void master_handle_communication() {
  vector<pair<int, int> > file_size_left(size - 1);

  for (int i = 0; i < size - 1; i++)
    file_size_left[i] = make_pair(1, i + 1);

  // TODO: Updated via a brute force search. Definitely optimize!!!

  bool done = false;

  set<int> done_list;

  while (!done) {
    // node_files => node id, number of files to request
    int node_files[2];
    MPI_Status status;
    int left;
    MPI_Recv(&left, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TAG, MPI_COMM_WORLD, &status);

    if (file_size_left[size - 2].first != 0) {
      for (int i = 0; i < size - 1; i++)
        if (file_size_left[i].second == status.MPI_SOURCE) {
          file_size_left[i].first = left;
          sort(file_size_left.begin(), file_size_left.end());
          break;
        }
    }

    node_files[1] = -1;
    if (file_size_left[size - 2].first == 0) {
      node_files[0] = END_PHASE;
      done_list.insert(status.MPI_SOURCE);
      if (done_list.size() == size - 1)
        done = true;
    }
    else {
      // TODO: Better heuristic?
      int take = (file_size_left[size - 2].first - left) / (size - 1);
      if (take < 1) {
        node_files[0] = STAY_PUT;
      }
      else {
        node_files[0] = take;
        node_files[1] = file_size_left[size - 2].second;
      }
    }

    MPI_Send(node_files, 2, MPI_INT, status.MPI_SOURCE, MASTER_TAG, MPI_COMM_WORLD);

  }

  int next = NEXT_PHASE_LOCKSTEP;

  // TODO: Improve efficiency with bcast
  for (int i = 1; i < size; ++i)
  {
    MPI_Send(&next, 1, MPI_INT, i, MASTER_TAG, MPI_COMM_WORLD);
  }

  // MPI_Bcast(NEXT_PHASE_LOCKSTEP,)
}


queue<pair<string, int> > load_file_list(string filelist, int &total_size) {

  FILE *fp1;
  char oneword[100];

  fp1 = fopen(filelist.c_str(), "r");

  int file_size = 0;

  queue<pair<string, int> > file_queue;
  while (fscanf(fp1, "%s %d", oneword, &file_size) != EOF) {
    file_queue.push(make_pair(string(oneword), file_size));
    total_size += file_size;
  }

  return file_queue;
}

void test_phase2_mp() {

  // if (rank == 0) {
  //   MPI_Comm_size(MPI_COMM_WORLD, &size);
  //   printf("No. of processes created %d\n",size);
  // }
  // else
  //   printf("Hello World from process %d\n", rank);


  if (rank == 0) {
    master_handle_communication();
  }
  else {
    map<string, set<string> > m = get_relevant_words_from_files();
    for (map<string, set<string> >::iterator i = m.begin(); i != m.end(); ++i)
    {
      printf("%d\n", (int)(i->second).size());
    }
  }


}