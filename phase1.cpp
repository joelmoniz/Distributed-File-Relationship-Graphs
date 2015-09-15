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
#include "mpiproperties.h"
#include <stdexcept>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "phase3.h"
#include <sys/stat.h>

using namespace std;

string originaldir;

//http://stackoverflow.com/a/874160
bool ends_with(string const &fullString, string const &ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

//http://www.johnloomis.org/ece537/notes/Files/Examples/printdir.html
void get_subdir(string dir, queue<string> &textfile_list, queue<string> &dir_list) {
  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;

  // if (dir.find(".git") == string::npos)

  if ((dp = opendir(dir.c_str())) == NULL) {
    // TODO: Figure out how to remove error
    // fprintf(stderr, "cannot open directory: %s\n", dir.c_str());
    return;
  }
  if (chdir(dir.c_str()) != 0)
    printf("err chdir");
  while ((entry = readdir(dp)) != NULL) {
    lstat(entry->d_name, &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {

      if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
        continue;

      string newdir = dir + "/" + string(entry->d_name);
      printf("Rank %d: %s\n", rank, newdir.c_str());

      // if (string(entry->d_name) != "." && string(entry->d_name) != "..")
      dir_list.push(newdir);
    }
    else if (dir != "")
    {
      string fname = string(entry->d_name);

      if (ends_with(fname, ".txt")) {
        textfile_list.push(dir + "/" + fname);
      }
      // else
      //   printf("%s\n", (dir + "/" + fname).c_str());
    }
  }

  chdir("..");
  closedir(dp);
}

void test_serial_traversal(int argc, char *argv[])
{
  char pwd[2] = ".";
  char topdir[500] = "";
  // char curdir[500] = "";
  if (argc != 2)
    strcpy(topdir, pwd);
  else if (argv[1][0] != '/')
  {
    realpath(argv[1], topdir);
  }
  // printf("%s\n", topdir);

  queue<string> tfl, subdir;
  subdir.push(string(topdir));

  while (!subdir.empty()) {
    get_subdir(subdir.front(), tfl, subdir);
    subdir.pop();
  }
  while (!tfl.empty()) {
    printf("%s\n", tfl.front().c_str());
    tfl.pop();
  }
}


queue<string> get_n_folders(int argc, char *argv[], int n)
{
  char pwd[2] = ".";
  char topdir[500] = "";
  // char curdir[500] = "";
  if (argc != 2)
    strcpy(topdir, pwd);
  else if (argv[1][0] != '/')
  {
    realpath(argv[1], topdir);
  }

  // originaldir = string(topdir);
  // printf("Original: %s\n", originaldir.c_str());

  queue<string> tfl, subdir;
  subdir.push(string(topdir));

  while (!subdir.empty() && subdir.size() < n) {
    get_subdir(subdir.front(), tfl, subdir);
    subdir.pop();
  }

  // TODO: Handle too few sub-folders better
  if (subdir.empty()) {
    subdir.push(string(topdir));
  }

  return subdir;
}

vector<string> slave_file_discovery() {

  bool done = false;
  omp_set_num_threads(omp_get_num_procs() + 1);
  vector<string> v;
  queue<string> dir_queue;


  int imt = 0;
  int c;
  MPI_Is_thread_main(&imt);

  char fname[MAX_PATH_SIZE];

  if (imt) {
    MPI_Recv(&c, MAX_PATH_SIZE, MPI_CHAR, 0, MASTER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (int i = 0; i < c; ++i)
    {
      MPI_Recv(fname, MAX_PATH_SIZE, MPI_CHAR, 0, MASTER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      dir_queue.push(string(fname));
    }
  }
  else {
    fprintf(stderr, "A fatal error has occured: master is not the master.\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int total_size = dir_queue.size();

  MPI_Barrier(MPI_COMM_WORLD);

  #pragma omp parallel shared(dir_queue, v, done, total_size)
  {
    int is_master_thread = 0;

    MPI_Is_thread_main(&is_master_thread);

    if (is_master_thread)
    {
      while (!done) {
        // Send status to master, receive update
        // node_files => node id, number of files to request
        int node_files[2];

        debug_mpi_dest(-1, 0);
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

          debug_mpi_dest(0, -1);

          if (num_requested_files == TIME_OUT)
            continue;
          else {
            // printf("%d files requested\n", num_requested_files);
            vector<string> fl(num_requested_files);
            int actual;
            #pragma omp critical(queuepop)
            {
              actual = min((int)dir_queue.size(), num_requested_files);
              for (int i = 0; i < actual; ++i)
              {
                fl[i] = dir_queue.front();
                dir_queue.pop();
                total_size--;
              }
            }


            debug_mpi_dest(1, src);
            MPI_Send(&actual, 1, MPI_INT, src, INTER_SLAVE_TAG, MPI_COMM_WORLD);

            for (int i = 0; i < actual; ++i)
            {
              MPI_Send(fl[i].c_str(), fl[i].length() + 1, MPI_CHAR, src, INTER_SLAVE_TAG, MPI_COMM_WORLD);
            }

          }
        }
        else {
          // ping node, get back reply
          int actual_size;

          debug_mpi_dest(2, node_files[0]);
          MPI_Send(&node_files[1], 1, MPI_INT, node_files[0], INTER_SLAVE_TAG, MPI_COMM_WORLD);
          // printf("%d   %d\n", node_files[0], node_files[1]);
          MPI_Recv(&actual_size, 1, MPI_INT, node_files[0], INTER_SLAVE_TAG, MPI_COMM_WORLD, &status);
          // printf("%d\n", actual_size);

          if (actual_size == END_PHASE || actual_size == 0)
            continue;

          vector<string> f(actual_size);
          char fname[MAX_PATH_SIZE];
          int sz;
          if (QUEUE_DEBUG)
            printf("Status: %d\n", actual_size);
          // TODO: Make more efficient by wrapping into a structure
          for (int i = 0; i < actual_size; i++) {
            MPI_Recv(fname, MAX_PATH_SIZE, MPI_CHAR, node_files[0], INTER_SLAVE_TAG, MPI_COMM_WORLD, &status);
            f[i] = string(fname);

            if (QUEUE_DEBUG)
              printf("From: %d In: %d %s\n", node_files[0], rank, fname);

            memset(fname, '\0', MAX_PATH_SIZE);
          }

          #pragma omp critical(queuepop)
          {
            for (int i = 0; i < actual_size; i++) {
              dir_queue.push(f[i]);
              total_size++;
            }
          }
          sleep(SLEEP_TIME);
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
          debug_mpi_dest(3, status.MPI_SOURCE);
          MPI_Send(&stat, 1, MPI_INT, status.MPI_SOURCE, INTER_SLAVE_TAG, MPI_COMM_WORLD);
        }
      }
    }
    else {
      bool q_not_empty = true;

      // TODO: Handle scenario when more files exist to be traversed,
      // but master hasn't assigned them yet
      while (q_not_empty || !done) {
        string dir_queue_element;
        #pragma omp critical(queuepop)
        {
          if (!dir_queue.empty()) {
            dir_queue_element = dir_queue.front();
            // printf("Here %s\n", dir_queue_element.c_str());
            total_size--;
            dir_queue.pop();
            q_not_empty = true;
          }
          else
            q_not_empty = false;
        }

        if (q_not_empty) {
          //void get_subdir(string dir, queue<string> &textfile_list, queue<string> &dir_list)
          queue<string> textfile_list;
          queue<string> dir_list;
          get_subdir(dir_queue_element, textfile_list, dir_list);

          #pragma omp critical(queuepop)
          {
            while (!dir_list.empty()) {
              dir_queue.push(dir_list.front());
              // printf("Here %s\n", dir_list.front().c_str());
              total_size++;
              dir_list.pop();
            }
          }

          #pragma omp critical(textfile_add)
          {
            while (!textfile_list.empty()) {
              v.push_back(textfile_list.front());
              printf("Here %s\n", textfile_list.front().c_str());
              textfile_list.pop();
            }
          }

        }
      }
    }
  }
  if (INITIAL_DEBUG) {
    printf("Done file size: %d\n", total_size);
  }
  return v;
}

// Assumption: no text files in first n sub-dirs encountered in bfs
void master_handle_distribution(int argc, char *argv[]) {
  vector<pair<int, int> > file_size_left(size - 1);

  queue<string> paths = get_n_folders(argc, argv, size - 1);

  for (int i = 1; i < size; i++) {
    int num_nodes = paths.size();
    int nn = num_nodes / (size - i);
    MPI_Send(&nn, 1, MPI_INT, i, MASTER_TAG, MPI_COMM_WORLD);
    for (int j = 0; j < nn; ++j)
    {
      MPI_Send(paths.front().c_str(), paths.front().length() + 1, MPI_CHAR, i, MASTER_TAG, MPI_COMM_WORLD);
      paths.pop();
    }
    file_size_left[i - 1] = make_pair(nn, i);
  }

  MPI_Barrier(MPI_COMM_WORLD);
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
      int take = (file_size_left[size - 2].first - left) / (2 * (size - 1));
      if (take < 1) {
        node_files[0] = STAY_PUT;
      }
      else {
        node_files[0] = file_size_left[size - 2].second;
        node_files[1] = take;
      }
    }


    debug_mpi_dest(4, status.MPI_SOURCE);
    MPI_Send(node_files, 2, MPI_INT, status.MPI_SOURCE, MASTER_TAG, MPI_COMM_WORLD);

  }

  int next = NEXT_PHASE_LOCKSTEP;

  // TODO: Improve efficiency with bcast
  for (int i = 1; i < size; ++i)
  {
    debug_mpi_dest(5, i);
    MPI_Send(&next, 1, MPI_INT, i, MASTER_TAG, MPI_COMM_WORLD);
  }

  // MPI_Bcast(NEXT_PHASE_LOCKSTEP,)
}

void test_phase1_mpi(int argc, char *argv[]) {

  // if (rank == 0) {
  //   MPI_Comm_size(MPI_COMM_WORLD, &size);
  //   printf("No. of processes created %d\n",size);
  // }
  // else
  //   printf("Hello World from process %d\n", rank);


  // TODO: Handle this neater
  char pwd[2] = ".";
  char topdir[500] = "";
  // char curdir[500] = "";
  if (argc != 2)
    strcpy(topdir, pwd);
  else if (argv[1][0] != '/')
  {
    realpath(argv[1], topdir);
  }

  originaldir = string(topdir);

  if (rank == 0) {
    master_handle_distribution(argc, argv);
  }
  else {
    vector<string> v = slave_file_discovery();
    printf("Here: rank %d\n", rank);
    for (int i = 0; i < v.size(); ++i)
    {
      printf("Rank: %d;  %s\n", rank, v[i].c_str());
    }
  }
}



void run_phase1_mpi(int argc, char *argv[]) {

  vector<string> v;

    // TODO: Handle this neater
  char pwd[2] = ".";
  char topdir[500] = "";
  // char curdir[500] = "";
  if (argc != 2)
    realpath(pwd, topdir);
  else if (argv[1][0] != '/')
  {
    realpath(argv[1], topdir);
  }

  originaldir = string(topdir);

  if (rank == 0) {
    master_handle_distribution(argc, argv);
  }
  else {
    v = slave_file_discovery();
  }

  MPI_Barrier(MPI_COMM_WORLD);

  for (int r = 1; r < size; r++) {
    if (r == rank) {

      stringstream sstm;
      sstm << "/data/node" << rank << "_files.txt";

      char x[200];
      getcwd(x, 200);
      printf("%s\n", x);
      string filelist =  originaldir + sstm.str();//sstm.str();
      printf("%s\n", filelist.c_str());
      FILE *fp1;
      fp1 = fopen(filelist.c_str(), "w");

      if (fp1 == NULL)
        printf("Ouch\n");

      printf("Here: rank %d\n", rank);
      for (int i = 0; i < v.size(); ++i)
      {
        fprintf(fp1, "%s\n", v[i].c_str());
      }

      // TODO: Handle better
      if (fp1 != NULL)
        fclose(fp1);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  // printf("Args: %d\n", argc);
}