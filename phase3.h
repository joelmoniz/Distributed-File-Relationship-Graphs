#ifndef PHASE_3_
#define PHASE_3_

#include <string>
#include <set>
#include <queue>
#include <utility>
#include <vector>
#include <queue>
#include <map>

extern map<string, int> file_to_node_mapping;
extern vector<string> node_to_file_mapping;
extern vector<int> adj_matrix_chunk;
extern vector<vector<pair<string, set<string> > > > external_list;

extern int node_first_file;
extern int node_last_file;
extern int node_file_count;

using namespace std;

void test_send_and_receive();

#define DEBUG_MAPPING 1

#define KEYWORD_TRANSFER 4

#endif