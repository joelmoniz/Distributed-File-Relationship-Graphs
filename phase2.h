#ifndef PHASE_2_
#define PHASE_2_

#include <string>
#include <set>
#include <map>
#include <queue>
#include <utility>
#include <map>

map<string, set<string> > get_relevant_words_from_files();
void test_phase2_mp();
map<string, set<string> > master_relevant_find(queue<pair<string, int> > &file_queue, int total_size);
map<string, set<string> > slave_relevant_find(queue<pair<string, int> > &file_queue, int total_size);
queue<pair<string, int> > load_file_list(string filelist, int &total_size);
#endif