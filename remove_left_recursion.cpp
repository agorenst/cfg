#include <set>
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "cfg.h"

#include <cassert>

using namespace std;
using namespace cfg;



int main() {
    auto G = read_grammar(cin);
    auto nonTerminals = G.all_nonterminals();

    sequence<production> new_grammar_productions;
    set<production> replaced;
    for (auto it = nonTerminals.begin();
    it != nonTerminals.end();
    ++it) {
        for (auto jt = nonTerminals.begin();
        jt != it;
        ++jt) {
            for (auto p : G.productions_from_nonterminal(*it)) {
                if (p.rhs.size() > 0 && *(p.rhs.begin()) == *jt) {
                    for (auto q : G.productions_from_nonterminal(*jt)) {
                        sequence<symbol> new_rhs(q.rhs.begin(), q.rhs.end());
                        for (auto st = next(p.rhs.begin()); st != p.rhs.end(); ++st) { new_rhs.push_back(*st); }
                        //new_rhs.insert(next(p.rhs.begin()), p.rhs.end());
                        //cout << "Adding new production!" << endl;
                        new_grammar_productions.push_back({*it, new_rhs});
                    }
                    replaced.insert(p);
                }
            }

        }
    }
    for (auto p : G.prods) {
        if (replaced.find(p) == replaced.end()) {
            new_grammar_productions.push_back(p);
        }
    }

    auto G1 = grammar(new_grammar_productions);

    sequence<production> final_productions;
    int new_nonterm = 0;
    for (auto A : G1.all_nonterminals()) {
        cout << A << endl;

        // filter all of the productions from A into
        // either left-recursive RHS, or not.
        sequence<sequence<symbol>> left_recursive_rhs;
        sequence<sequence<symbol>> beta_rhs;
        auto productions = G1.productions_from_nonterminal(A);
        for_each(productions.begin(),
                 productions.end(),
                [&](const production& p) {
                    if (p.rhs.size() > 0 && *(p.rhs.begin()) == A) {
                        // we drop the A at the start, because we won't need it.
                        left_recursive_rhs.push_back(sequence<symbol>(next(p.rhs.begin()), p.rhs.end()));
                    }
                    else {
                        beta_rhs.push_back(p.rhs);
                    }
                });

        // so we have our two sets.
        cout << "Done partitioning" << endl;

        // IF there was no left recursion, insert our productions as normal
        // and continue;
        if (left_recursive_rhs.size() == 0) {
            for (auto p : G1.productions_from_nonterminal(A)) {
                final_productions.push_back(p);
            }
        }
        // otherwise, we do the transformation:
        else {
            auto new_nonterminal = A+std::to_string(new_nonterm++);
            cout << "new_nonterminal " << new_nonterminal << endl;
            auto nonterminals = G1.all_nonterminals();
            assert(nonterminals.find(new_nonterminal) == nonterminals.end());
            for (auto rhs : beta_rhs) {
                auto new_rhs = rhs;
                new_rhs.push_back(new_nonterminal);
                final_productions.push_back(production{A, new_rhs});
            }
            for (auto rhs : left_recursive_rhs) { // remember, we've already removed the leading "A".
                auto new_rhs = rhs;
                new_rhs.push_back(new_nonterminal); // right-recursive!
                final_productions.push_back(production{new_nonterminal, new_rhs});
            }
            final_productions.push_back(production{new_nonterminal, {}});
        }
    }

    cout << grammar(final_productions) << endl;
}
