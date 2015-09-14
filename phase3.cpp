#include <map>
#include <iostream>
#include <vector>
#include "mpi.h"
#include <sstream>
#include "phase2.h"
#include "mpiproperties.h"
#include "phase3.h"
#include <queue>
#include <map>
#include <string>

using namespace std;

map<string, int> file_to_node_mapping;
vector<string> node_to_file_mapping;

vector<int> adj_matrix_chunk;
vector<vector<pair<string, set<string> > > > external_list(2, vector<pair<string, set<string> > >());

int node_first_file;
int node_last_file;
int node_file_count;

void get_file_integer_map() {

  int total_size;
  int num = 0;
  if (rank == 0)
    return;
  for (int i = 1; i < size; ++i)
  {
    stringstream sstm;
    sstm << "./data/node" << i << "_files.txt";
    string filelist =  sstm.str();

    queue<pair<string, int> > file_queue = load_file_list(filelist, total_size);

    pair<string, int> p;

    if (i == rank) {
      node_first_file = num;
    }

    while (!file_queue.empty()) {
      p = file_queue.front();
      file_queue.pop();
      file_to_node_mapping[p.first] = num;
      num++;
    }

    if (i == rank) {
      node_last_file = num;
      node_file_count = node_last_file - node_first_file + 1;
    }

  }
  node_to_file_mapping.resize(num);

  for (map<string, int>::iterator i = file_to_node_mapping.begin(); i != file_to_node_mapping.end(); ++i)
  {
    node_to_file_mapping[i->second] = i->first;

    if (DEBUG_MAPPING)
      printf("Rank: %d; %s: %d\n", rank, (i->first).c_str(), (i->second));
  }
}

// TODO: Optimize as vector of strings, or custom data structure instead of so many messages
void send_file_keywords(string file, set<string> keyword_set) {
  int dest = ((rank + 1) == size) ? 1 : (rank + 1);
  int sz = keyword_set.size();

  MPI_Send(file.c_str(), file.length() + 1, MPI_CHAR, dest, KEYWORD_TRANSFER, MPI_COMM_WORLD);
  MPI_Send(&sz, 1, MPI_INT, dest, KEYWORD_TRANSFER, MPI_COMM_WORLD);

  for (set<string>::iterator i = keyword_set.begin(); i != keyword_set.end(); ++i)
  {
    MPI_Send((*i).c_str(), (*i).length() + 1, MPI_CHAR, dest, KEYWORD_TRANSFER, MPI_COMM_WORLD);
  }
}

// TODO: Optimize as vector of strings, or custom data structure instead of so many messages (same as in send_file_keywords)
pair<string, set<string> > recv_file_keywords() {
  int src = ((rank - 1) == 0) ? (size - 1) : (rank - 1);
  int sz;
  char file[255];
  char word[100];

  MPI_Recv(file, 255, MPI_CHAR, src, KEYWORD_TRANSFER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  MPI_Recv(&sz, 1, MPI_INT, src, KEYWORD_TRANSFER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  string f(file);
  set<string> keyword_set;

  while (sz--)
  {
    MPI_Recv(word, 100, MPI_CHAR, src, KEYWORD_TRANSFER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    keyword_set.insert(string(word));
  }
  return make_pair(f, keyword_set);
}

void send_graph(int iter) {
  vector<pair<string, set<string> > > l = external_list[(iter + 1) % 2];

  int dest = ((rank + 1) == size) ? 1 : (rank + 1);
  int sz = l.size();
  MPI_Send(&sz, 1, MPI_INT, dest, KEYWORD_TRANSFER, MPI_COMM_WORLD);

  for (int i = 0; i < sz; i++) {
    send_file_keywords(l[i].first, l[i].second);
  }
}

void receive_graph(int iter) {
  external_list[iter % 2].clear();

  int src = ((rank - 1) == 0) ? (size - 1) : (rank - 1);
  int sz;
  MPI_Recv(&sz, 1, MPI_INT, src, KEYWORD_TRANSFER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  for (int i = 0; i < sz; i++) {
    external_list[iter % 2].push_back(recv_file_keywords());
  }
}

void transfer_graph(int iter) {
  if (rank != 0 && iter < size - 1) {
    // 0 "indexed", but we don't want info exchange in 0th iter.
    if (rank % 2 == 0) {
      receive_graph(iter);
      send_graph(iter);
    }
    else {
      send_graph(iter);
      receive_graph(iter);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

void test_send_and_receive() {
  get_file_integer_map();
  switch (rank) {
  case 1:
  {
    string init[] = { "His", "Name"};
    external_list[1].push_back(make_pair("maurya", set<string>(init, init + 2)));
    string init2[] = { "Is", "Mine", "too"};
    external_list[1].push_back(make_pair("saurabh", set<string>(init2, init2 + 3)));
    break;
  }
  case 2:
  { string init[] = { "whenever", "wego"};
    external_list[1].push_back(make_pair("moniz", set<string>(init, init + 2)));
    string init2[] = { "out", "people", "shout"};
    external_list[1].push_back(make_pair("krishnan", set<string>(init2, init2 + 3)));
    break;
  }
  case 3:
  { string init[] = { "John", "Jacob"};
    external_list[1].push_back(make_pair("joel", set<string>(init, init + 2)));
    string init2[] = { "Jingle", "Heimer"};
    external_list[1].push_back(make_pair("gokul", set<string>(init2, init2 + 2)));
    break;
  }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  for (int i = 0; i < size; ++i)
  {
    printf("------------------------\n");
    printf("Iter %d\n", i);
    printf("------------------------\n");
    transfer_graph(i);
    for (int r = 1; r < size; r++) {
      if (r == rank) {
        printf("Rank: %d\n", rank);
        for (int j = 0; j < external_list[i % 2].size(); j++) {
          printf("first: %s\n", external_list[i % 2][j].first.c_str());

          for (set<string>::iterator k = external_list[i % 2][j].second.begin(); k != external_list[i % 2][j].second.end(); ++k)
          {
            printf("%s\n", (*k).c_str());
          }
        }
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
}