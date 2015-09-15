#include <stdio.h>
#include <set>
#include <string>
#include <stdarg.h>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include "relevant_extractor.h"
#include <stdexcept>
#include <cctype>

#define PRINT_WORDS 0

using namespace std;

set<string> stopwords;

// TODO: Setup as class

void test_relevant() {
  set<string> staph; 
  staph.insert("is");

  set<string> ans = get_relevant_words("./medium.txt");

  if (PRINT_WORDS) {
    for (set<string>::iterator i = ans.begin(); i != ans.end(); i++) {
      cout<<*i<<"\n";
    }
  }
  cout<<ans.size()<<"\n";
}

bool is_non_alpha(char c) {
  if (isalpha(c))
    return false;
  else
    return true;
}

void setup_stopwords() {
  string stop[] = {"a", "an", "the", "of", "on", "in"};
  stopwords = set<string>(stop, stop+sizeof(stop)/sizeof(string *));
}

set<string> get_relevant_words(string f) {
  map<string, int> m;
  /*
  ifstream file;
  file.open (f.c_str());
  string word;
  while (file >> word) {
    if (m.find(word) == m.end()) {
      m[word] = 0;
    }
    m[word]++;
  }
  */
  FILE *fp1;
  char oneword[100];

  fp1 = fopen(f.c_str(),"r");
  
  set<string> rel;

  if (fp1 == NULL)
    throw runtime_error("Could not open file");

  // printf("%s\n", f.c_str());
  while (fscanf(fp1,"%s",oneword) != EOF) {
    string word(oneword);
    if (stopwords.find(word) != stopwords.end()) {
      continue;
    }
    // http://stackoverflow.com/a/6319898
    word.erase(remove_if(word.begin(), word.end(), is_non_alpha), word.end());
    m[word]++;
  }



  if (m.size() == 0)
    return rel;
  vector<pair<int, string> > vp(m.size(), make_pair(0, ""));

  int cnt = 0;
  for (map<string, int>::iterator i = m.begin(); i != m.end(); i++, cnt++) {
    vp[cnt].first = i->second;
    vp[cnt].second = i->first;
  }

  sort(vp.begin(), vp.end(), greater<pair<int, string> >());

  for (cnt=0; (1.0*cnt)<=(vp.size()* KEYWORD_PC); cnt++) {
    rel.insert(vp[cnt].second);
  }

  int upper = vp[cnt-1].first;

  while (cnt<vp.size() && vp[cnt].first == upper) {
    rel.insert(vp[cnt].second);
    cnt++;
  }

  return rel;
}