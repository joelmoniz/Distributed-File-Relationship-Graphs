#ifndef PHASE_2_
#define PHASE_2_

#include <string>
#include <set>
#include <queue>
#include <utility>
#include <map>

#define DEBUG_MPI_DEST 0
#define INITIAL_DEBUG 0
#define QUEUE_DEBUG 0

#define SLEEP_TIME 1

#define UPDATE_FREQ 1
#define MAX_PATH_SIZE 255

using namespace std;

map<string, set<string> > get_relevant_words_from_files();
void test_phase2_mpi();
map<string, set<string> > master_relevant_find(queue<pair<string, int> > &file_queue, int total_size);
map<string, set<string> > slave_relevant_find(queue<pair<string, int> > &file_queue, int total_size);
queue<pair<string, int> > load_file_list(string filelist, int &total_size);


void test_phase2_without_mpi();
void run_phase2_mpi();
#endif