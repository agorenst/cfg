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
std::vector<S> tokenize(S s) {
    std::stringstream strstr(s);
    // use stream iterators to copy the stream to the vector as whitespace separated strings
    std::istream_iterator<S> it(strstr);
    return std::vector<S>(it, std::istream_iterator<S>());
}

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

    symbol grammar::start_symbol() const {
        return prods.begin()->lhs;
    }

    // Essentially we filter over the productions of the grammar.
    sequence<production> grammar::productions_from_nonterminal(const symbol lhs) const {
        assert(is_nonterminal(lhs));
        sequence<production> ret_val = {};

        std::copy_if(prods.begin(), prods.end(), std::back_inserter(ret_val),
                [&](const production& p) { return p.lhs == lhs; });

        return ret_val;
    }

    bool grammar::is_nonterminal(const symbol& s) const {
        return std::any_of(prods.begin(), prods.end(),
                [&](const production &p) { return p.lhs == s; });
    }
    bool grammar::is_terminal(const symbol& s) const {
        return !is_nonterminal(s);
    }

    // IO operations
    grammar read_grammar(std::istream& input) {
        sequence<production> production_list = {};

        std::string line;
        while(getline(input, line)) {
            std::vector<symbol> tokens = tokenize(line);
            if (tokens.size() == 0) { continue; }
            production new_prod{*tokens.begin(), sequence<symbol>{next(tokens.begin()), tokens.end()}};
            production_list.push_back(new_prod);
        }
        return grammar{production_list};
    }

    std::ostream& operator<<(std::ostream& o, const production& p) {
        o << p.lhs << " ";
        for (auto&& s : p.rhs) {
            o << s << " ";
        }
        return o;
    }

    std::ostream& operator<<(std::ostream& o, const grammar& g) {
        for (auto&& p : g.prods) {
            o << p << std::endl;
        }
        o << std::endl;
        return o;
    }

}

using namespace cfg;

int main() {
    grammar g = read_grammar(std::cin);
    std::cout << g << std::endl;
    return 0;
}
