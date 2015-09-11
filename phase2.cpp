#include "relevant_extractor.h"
#include "mpi.h"
#include <omp.h>
#include <map>
#include <set>
#include <stdio.h>
#include <queue>
#include <unistd.h>
#include "phase2.h"

using namespace std;

map<string, set<string> > get_relevant_words_from_files() {

  // the one is the master, handling communication, etc.
  printf("%d processors\n", omp_get_num_procs());
  setup_stopwords();

  string filelist = "files.txt";

  int total_size = 0;

  queue<string> file_queue = load_file_list(filelist, total_size);

  return slave_relevant_find(file_queue, total_size);
}

map<string, set<string> > slave_relevant_find(queue<string> &file_queue, int total_size) {

  printf("%d\n", total_size);

  bool done = false;
  omp_set_num_threads(omp_get_num_procs() + 1);
  map<string, set<string> > m;

  #pragma omp parallel shared(file_queue, m, done)
  {
    int is_master = 0;

    MPI_Is_thread_main(&is_master);
    if (is_master)
      // #pragma omp master
    {
      printf("%d threads\n", omp_get_num_threads());
      printf("Sleeping\n");
      sleep(2);
      printf("Woken\n");

      while (!done) {
        #pragma omp critical(queuepop)
        {
          file_queue.push("./medium.txt");
          file_queue.push("./medium.txt");
          file_queue.push("./medium.txt");
          file_queue.push("./medium.txt");
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
        string file;
        #pragma omp critical(queuepop)
        {
          if (!file_queue.empty()) {
            file = file_queue.front();
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
  return m;
}


queue<string> load_file_list(string filelist, int &total_size) {

  FILE *fp1;
  char oneword[100];

  fp1 = fopen(filelist.c_str(), "r");

  int file_size = 0;

  queue<string> file_queue;
  while (fscanf(fp1, "%s %d", oneword, &file_size) != EOF) {
    file_queue.push(string(oneword));
    total_size += file_size;
  }

  return file_queue;
}

void test_phase2_mp() {
  map<string, set<string> > m = get_relevant_words_from_files();

  for (map<string, set<string> >::iterator i = m.begin(); i != m.end(); ++i)
  {
    printf("%d\n", (int)(i->second).size());
  }
}