#ifndef RELEVANT_EXTRACTOR_
#define RELEVANT_EXTRACTOR_

#include <set>
#include <string>

#define KEYWORD_PC 0.10

using namespace std;

extern set<string> stopwords;

void test_relevant();
set<string> get_relevant_words(string f);
void setup_stopwords();

#endif