#include "cfg.h"

#include <cassert>
#include <iterator>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

// This is a non-exposed helper function.
template<class S>
vector<S> tokenize(S s) {
    stringstream strstr(s);
    // use stream iterators to copy the stream to the vector as whitespace separated strings
    istream_iterator<S> it(strstr);
    return vector<S>(it, istream_iterator<S>());
}

// Here are the implementations of the exposed methods.
namespace cfg {
    production grammar::operator[](const int i) const {
        auto finder = prods.begin();
        advance(finder, i);
        return *finder;
    }
    int grammar::index_of(const production& p) const {
        int i = 0;
        for (auto&& q : prods) {
            if (q == p) { return i; }
            ++i;
        }
        return -1;
    }

    // This exploits the fact that productions are ordered.
    // The start symbol is always just the first nonterminal
    // we see.
    symbol grammar::start_symbol() const {
        return prods.begin()->lhs;
    }

    // Essentially we filter over the productions of the grammar.
    sequence<production> grammar::productions_from_nonterminal(const symbol lhs) const {
        assert(is_nonterminal(lhs));
        sequence<production> ret_val = {};

        copy_if(prods.begin(), prods.end(), back_inserter(ret_val),
                [&](const production& p) { return p.lhs == lhs; });

        return ret_val;
    }

    bool grammar::is_nonterminal(const symbol& s) const {
        return any_of(prods.begin(), prods.end(),
                [&](const production &p) { return p.lhs == s; });
    }
    bool grammar::is_terminal(const symbol& s) const {
        return !is_nonterminal(s);
    }

    // IO operations.

    // This essentially defines the format we expect the CFG
    // to be in. It's pretty simple, and counts on everything
    // being whitespace delimited.
    grammar read_grammar(istream& input) {
        sequence<production> production_list = {};

        string line;
        while(getline(input, line)) {
            vector<symbol> tokens = tokenize(line);
            if (tokens.size() == 0) { continue; }
            production new_prod{
                *tokens.begin(), // lhs
                sequence<symbol>{next(tokens.begin()), tokens.end()} // rhs
            };
            production_list.push_back(new_prod);
        }
        return grammar{production_list};
    }

    // Because we insert whitespace here, we should be able to read
    // back in any CFG we print out.
    ostream& operator<<(ostream& o, const production& p) {
        o << p.lhs << " ";
        for (auto&& s : p.rhs) {
            o << s << " ";
        }
        return o;
    }
    ostream& operator<<(ostream& o, const grammar& g) {
        for (auto&& p : g.prods) {
            o << p << endl;
        }
        o << endl;
        return o;
    }

}

// What follows here is user code, exercising just a small bit
// of the CFG class to make sure nothing's absolutely egregiously
// wrong.

//using namespace cfg;
//
//// An example of how we can succinctly define grammars "in-code"
//// for the hard-coded case.
//const grammar arithmetic{
//    {"S", "S", "+", "S"},
//    {"S", "S", "-", "S"},
//    {"S", "S", "/", "S"},
//    {"S", "S", "*", "S"},
//    {"S", "n"}
//};
//
//int main() {
//    // Sample usage: ./cfg < arith.cfg
//    grammar g = read_grammar(cin);
//    cout << g << endl;
//    cout << arithmetic << endl;
//    return 0;
//}
