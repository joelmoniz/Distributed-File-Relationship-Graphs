#include "relevant_extractor.h"
#include <omp.h>
#include <map>
#include <set>
#include <omp.h>
#include <stdio.h>
#include <queue>
#include <unistd.h>
#include "phase2.h"

using namespace std;

map<string, set<string> > get_relevant_words_from_files() {

  // the one is the master, handling communication, etc.
  printf("%d processors\n", omp_get_num_procs());
  setup_stopwords();


  queue<string> filelist;


  return slave_relevant_find(filelist);
}

map<string, set<string> > slave_relevant_find(queue<string> &filelist) {
  bool done = false;
  omp_set_num_threads(omp_get_num_procs() + 1);
  map<string, set<string> > m;
  #pragma omp parallel shared(filelist, m, done)
  {
    if (omp_get_thread_num() == 0) {
      printf("%d threads\n", omp_get_num_threads());
      printf("Sleeping\n");
      sleep(2);
      printf("Woken\n");

      while (!done) {
        #pragma omp critical(queuepop)
        {
          filelist.push("./medium.txt");
          filelist.push("./medium.txt");
          filelist.push("./medium.txt");
          filelist.push("./medium.txt");
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
          if (!filelist.empty()) {
            file = filelist.front();
            filelist.pop();
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

void test_phase2_mp() {
  map<string, set<string> > m = get_relevant_words_from_files();

  for (map<string, set<string> >::iterator i = m.begin(); i != m.end(); ++i)
  {
    printf("%d\n", (int)(i->second).size());
  }
}