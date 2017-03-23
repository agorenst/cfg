#include <set>
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "cfg.h"

#include "first.h"

using namespace std;
using namespace cfg;

template <class K>
void print_set(const map<K, set<symbol>>& S) {
  cout << "{" << endl;
  for (auto& p : S) {
    cout << p.first << " : { ";
    for (auto& s : p.second) {
      cout << s << " ";
    }
    cout << "}" << endl;
  }
  cout << "}" << endl;
}



int main() {
  auto G = read_grammar(cin);
  auto FIRST = compute_first(G);
  print_set(FIRST);
  auto FOLLOW = compute_follow(G);
  cout << "=========================" << endl;
  print_set(FOLLOW);
  cout << "=========================" << endl;
  auto PREDICT = compute_predict(G);
  print_set(PREDICT);
  cout << "=========================" << endl;
  auto x = compute_predict_predict_conflict(G);
  if (get<0>(x).lhs != "") {
    cout << get<0>(x) << endl;
    cout << get<1>(x) << endl;
  }
}

