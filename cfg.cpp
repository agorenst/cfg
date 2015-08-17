#include "cfg.h"

#include <cassert>
#include <iterator>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>

// Can be understood as a map over all productions of *splitter,
// 
sequence<sequence<symbol>> grammar::develop_seq_at_iter(
        const sequence<symbol>& seq,
        sequence<symbol>::const_iterator splitter) const {
    assert(is_nonterminal(*splitter));
    // TODO: add assertion that splitter is actually an iterator for container seq?

    sequence<sequence<symbol>> ret_value = {};
    for (auto&& prod : productions_from_nonterminal(*splitter)) {
        sequence<symbol> next_development = {};
        next_development.insert(next_development.end(), seq.begin(), splitter);
        next_development.insert(next_development.end(), prod.rhs.begin(), prod.rhs.end());
        next_development.insert(next_development.end(), std::next(splitter), seq.end());
        ret_value.push_back(next_development);
    }
    return ret_value;
}

bool grammar::is_nonterminal(const symbol& s) const {
    for (auto&& p : productions) {
        if (p.lhs == s) { return true; }
    }
    return false;
}

template<class S>
std::vector<S> tokenize(S s) {
    std::stringstream strstr(s);
    // use stream iterators to copy the stream to the vector as whitespace separated strings
    std::istream_iterator<S> it(strstr);
    return std::vector<S>(it, std::istream_iterator<S>());
}

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
    for (auto&& p : g.productions) {
        o << p << std::endl;
    }
    o << std::endl;
    return o;
}


int main() {
    grammar g = read_grammar(std::cin);
    std::cout << g << std::endl;
    return 0;
}
