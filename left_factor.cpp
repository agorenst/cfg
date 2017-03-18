#include <map>
#include "cfg.h"
#include "./../trie/prefixtree.h"

#include <cassert>

using namespace std;
using namespace cfg;


int main() {
    auto G = read_grammar(cin);

    map<symbol, prefix_tree<symbol>> common_prefixes;
    
    for (auto&& p : G.prods) {
        common_prefixes[p.lhs].insert(p.rhs.begin(), p.rhs.end());
    }

    for (auto&& lhs_and_tree : common_prefixes) {
        cout << "NONTERMINAL: " << lhs_and_tree.first << endl;
        lhs_and_tree.second.simple_print(cout);
    }
}
