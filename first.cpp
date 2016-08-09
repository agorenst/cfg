#include <set>
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "cfg.h"


using namespace std;
using namespace cfg;

const symbol EPS = "";

map<symbol, set<symbol>> compute_first(const grammar& g) {
    map<symbol, set<symbol>> FIRST;
    for (auto& s : g.all_terminals()) {
        FIRST[s] = {s};
    }
    for (auto& p : g.all_productions()) {
        if (p.rhs.size() == 0) {
            FIRST[p.lhs].insert(EPS);
        }
    }
    bool workDone = true;
    while (workDone) {
        workDone = false;
        for (auto& p : g.all_productions()) {
            for (auto& s : p.rhs) {

                // Add the first set to our FIRST set.
                auto old_size = FIRST[p.lhs].size();
                FIRST[p.lhs].insert(FIRST[s].begin(), FIRST[s].end());
                if (old_size != FIRST[p.lhs].size()) { // this means we've added something.
                    workDone = true;
                }

                // if this symbol doesn't have EPS, then we can't
                // continue.
                if (FIRST[s].find(EPS) == FIRST[s].end()) {
                    break;
                }
            }
        }
    }
    return FIRST;
}

template<typename ITER>
set<symbol> sequence_first(ITER start, ITER finish, map<symbol, set<symbol>>& FIRST) {
    set<symbol> ret;
    for (; start != finish; ++start) {
        ret.insert(FIRST[*start].begin(), FIRST[*start].end());
        if (FIRST[*start].find(EPS) == FIRST[*start].end()) {
            break;
        }
    } 
    return ret;
}

template<typename ITER>
bool sequence_epsilon(ITER start, ITER finish, map<symbol, set<symbol>>& FIRST) {
    return all_of(start, finish, [&](const symbol& s) { return FIRST[s].find(EPS) != FIRST[s].end(); });
}

map<symbol, set<symbol>> compute_follow(const grammar& g) {
    map<symbol, set<symbol>> FOLLOW;
    auto FIRST = compute_first(g);

    for (auto& s : g.all_symbols()) {
        FOLLOW[s] = {};
    }

    bool workDone = true;
    while (workDone) {
        workDone = false;
        for (auto& p : g.all_productions()) {
            for (auto sit = p.rhs.begin(); sit != p.rhs.end(); ++sit) {
                // this is the alpha B beta case
                if (std::next(sit) != p.rhs.end()) {
                    auto seq_first = sequence_first(std::next(sit), p.rhs.end(), FIRST);
                    auto old_size = FOLLOW[*sit].size();
                    FOLLOW[*sit].insert(seq_first.begin(), seq_first.end());
                    if (old_size != FOLLOW[*sit].size()) { workDone = true; }

                    // If this sequence creates an epsilon. Is that the same as
                    // having eps \in seq_first?
                    if (sequence_epsilon(std::next(sit), p.rhs.end(), FIRST)) {
                        FOLLOW[*sit].insert(FOLLOW[p.lhs].begin(), FOLLOW[p.lhs].end());
                    }
                    if (old_size != FOLLOW[*sit].size()) { workDone = true; }
                }
                // this is the alpha B case (identical as if "beta" is already epsilon):
                else {
                    FOLLOW[*sit].insert(FOLLOW[p.lhs].begin(), FOLLOW[p.lhs].end());
                }
            }
        }
    }
    return FOLLOW;
}

void print_set(const map<symbol, set<symbol>>& S) {
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
}
