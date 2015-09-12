#include "relevant_extractor.h"
#include "mpi.h"
#include <omp.h>
#include <map>
#include <set>
#include <stdio.h>
#include <queue>
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

  if (rank == 0) {
    return master_relevant_find(file_queue, total_size);
  }
  else {
    return slave_relevant_find(file_queue, total_size);
  }
}

map<string, set<string> > slave_relevant_find(queue<pair<string, int> > &file_queue, int total_size) {

  printf("%d\n", total_size);

  bool done = false;
  omp_set_num_threads(omp_get_num_procs() + 1);
  map<string, set<string> > m;

  #pragma omp parallel shared(file_queue, m, done)
  {
    int is_master = 0;

    MPI_Is_thread_main(&is_master);
    if (is_master)
    {
      printf("%d threads\n", omp_get_num_threads());
      printf("Sleeping\n");
      sleep(2);
      printf("Woken\n");

      while (!done) {
        #pragma omp critical(queuepop)
        {
          file_queue.push(make_pair("./medium.txt", 20));
          total_size += 20;
          file_queue.push(make_pair("./medium.txt", 20));
          total_size += 20;
          file_queue.push(make_pair("./medium.txt", 20));
          total_size += 20;
          file_queue.push(make_pair("./medium.txt", 20));
          total_size += 20;
        }
        sleep(2);
        printf("Final\n");
        done = true;
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


map<string, set<string> > master_relevant_find(queue<pair<string, int> > &file_queue, int total_size) {

  printf("%d\n", total_size);

  bool done = false;
  omp_set_num_threads(omp_get_num_procs() + 1);
  map<string, set<string> > m;

  #pragma omp parallel shared(file_queue, m, done)
  {
    int is_master = 0;

    MPI_Is_thread_main(&is_master);
    if (is_master)
    {
      printf("%d threads\n", omp_get_num_threads());
      printf("Sleeping\n");
      sleep(2);
      printf("Woken\n");

      while (!done) {
        #pragma omp critical(queuepop)
        {
          file_queue.push(make_pair("./medium.txt", 20));
          total_size += 20;
          file_queue.push(make_pair("./medium.txt", 20));
          total_size += 20;
          file_queue.push(make_pair("./medium.txt", 20));
          total_size += 20;
          file_queue.push(make_pair("./medium.txt", 20));
          total_size += 20;
        }
        sleep(2);
        printf("Final\n");
        done = true;
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


  map<string, set<string> > m = get_relevant_words_from_files();

  for (map<string, set<string> >::iterator i = m.begin(); i != m.end(); ++i)
  {
    printf("%d\n", (int)(i->second).size());
  }
}