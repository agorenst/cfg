#include "first.h"

#include <set>
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "cfg.h"

// Following mainly the 3rd edition of Michael Scott's book, with some references to
// the 2nd edition of the dragon book.
// Weirdness: MScott's algorithms don't include EPS (epsilon) in the sets, but they do in the dragon book.
// But the dragon book also includes an "end of program" symbol no matter what?
// I should revisit all this to make sure I'm not making hidden assumptions. Presumably, these functions
// should be computable for all grammars, not jut LALR or whatever (though restricted grammars may be
// the only ones for which these sets are useful).
//
// Next up: creating a table-driven pasrer...


using namespace std;
using namespace cfg;

const symbol EPS = "";

map<symbol, set<symbol>> compute_first(const grammar& g) {
  map<symbol, set<symbol>> FIRST;

  // all terminals are their own first sets.
  for (auto& s : g.all_terminals()) FIRST[s] = {s};

  // add epsilon for any nonterminal that directly produces epsilon.
  for (auto& p : g.all_productions()) {
    if (p.rhs.size() == 0) { FIRST[p.lhs] = {EPS}; }
  }

  bool workDone = true;
  while (workDone) {
    workDone = false;

    for (auto& p : g.all_productions()) {

      // We proceed down this production, and we only continue
      // to the i+1 symbol if the i-th symbol could produce epsilon.
      bool wholeProdIsEps = true;
      for (auto& s : p.rhs) {

        // This symbol s could be the first in this production
        // to produce non-epsilon, so we inherit its first set.
        // But we shouldn't add epsilon!
        for (auto&& b : FIRST[s]) {
          if (b == EPS) { continue; }
          auto result = FIRST[p.lhs].insert(b);
          workDone |= get<1>(result);
        }

        // if this symbol doesn't have EPS, then we can't
        // continue.
        if (FIRST[s].find(EPS) == FIRST[s].end()) {
          wholeProdIsEps = false;
          break;
        }
      }
      // Isn't this necessary?
      if (wholeProdIsEps) {
        FIRST[p.lhs].insert(EPS);
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

map<symbol, set<symbol>> compute_follow(const grammar& g, bool Scott /*= false*/) {
  map<symbol, set<symbol>> FOLLOW;
  auto FIRST = compute_first(g);

  // In Scott, we include computation for nonterminals.
  // In C&T, we don't
  if (Scott) {
    for (auto&& a : g.all_symbols()) {
      FOLLOW[a] = {}; // redundant...
    }
  }

  bool workDone = true;
  while (workDone) {
    workDone = false;

    for (auto&& p : g.all_productions()) {
      auto TRAILER = FOLLOW[p.lhs];
      for (auto it = p.rhs.rbegin(); it != p.rhs.rend(); ++it) {
        if (Scott) {
          // Do the set insertion
          for (auto&& t : TRAILER) {
            auto res = FOLLOW[*it].insert(t);
            workDone |= get<1>(res);
          }
        }
        if (g.is_nonterminal(*it)) {
          if (!Scott) {
            // Do the set insertion
            for (auto&& t : TRAILER) {
              auto res = FOLLOW[*it].insert(t);
              workDone |= get<1>(res);
            }
          }

          // If we could produce epsilon, we don't have to clear
          // the TRAILER. We simply out what we could produce instead.
          if (FIRST[*it].find(EPS) != FIRST[*it].end()) {
            for (auto&& s : FIRST[*it]) {
              if (s == EPS) { continue; }
              TRAILER.insert(s);
            }
          }
          // Because we don't produce epsilon, the trailer becomes
          // our FIRST set.
          else {
            TRAILER = FIRST[*it];
          }
        }
        else {
          TRAILER = FIRST[*it]; // as this is a terminal, it's just itself..
        }
      }
    }
  }
  return FOLLOW;
}

map<symbol, set<symbol>> compute_follow_scott(const grammar& g) {
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

map<production, set<symbol>> compute_predict(const grammar& g) {
  auto FIRST = compute_first(g);
  auto FOLLOW = compute_follow(g);
  map<production, set<symbol>> PREDICT;
  for (auto& p : g.all_productions()) {
    PREDICT[p] = sequence_first(p.rhs.begin(), p.rhs.end(), FIRST);
    if (sequence_epsilon(p.rhs.begin(), p.rhs.end(), FIRST)) {
      PREDICT[p].insert(FOLLOW[p.lhs].begin(), FOLLOW[p.lhs].end());
    }
  }
  return PREDICT;
}

