#include "cfg.h"
#include "parse_tree.h"
#include <stack>

using namespace std;
using namespace cfg;

const cfg::grammar arithmetic{
    {"S", "S", "+", "S"},
    {"S", "S", "-", "S"},
    {"S", "S", "/", "S"},
    {"S", "S", "*", "S"},
    {"S", "n"}
};
const cfg::grammar lambdaGrammar {
    {"S", "(", "L", "N", "S", ")"},
    {"S", "N"},
    {"S", "(", "S", "S", ")"},
    {"N", "n"},
};

std::list<cfg::parse_tree> all_develop_of_leaf(const parse_tree& p) {
    std::list<parse_tree> ret_val = {};
    if (!p.has_undeveloped()) { return ret_val; }
    symbol to_develop = p.undeveloped_symbol();

    assert(p.g.is_nonterminal(to_develop));

    for (auto&& production : p.g.productions_from_nonterminal(to_develop)) {
        int index = p.g.index_of(production);
        ret_val.push_back(p.apply_production(index));
    }
    return ret_val;
}

int main(int argc, char* argv[]) {
    parse_tree start(lambdaGrammar);
    std::stack<parse_tree> work_list;
    work_list.push(start);
    while(work_list.size()) {
        auto x = work_list.top();
        work_list.pop();
        if (x.leaf_count() > atoi(argv[1])) {
            continue;
        }
        if (!x.has_undeveloped()) {
            x.print_leaves(cout);
            cout << '\n';
            //cerr << x << endl;
        }
        else {
            for (auto&& t : all_develop_of_leaf(x)) {
                work_list.push(t);
            }
        }
    }
}
