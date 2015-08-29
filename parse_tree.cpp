#include "parse_tree.h"

using namespace cfg;

parse_tree::node_state parse_tree::state(node const * n) const {
    if (g.is_terminal(n->my_symbol)) {
        assert(n->children.size() == 0);
        assert(n->production_index == -1);
        return node_state::terminal_leaf;
    }
    else {
        if (n->production_index == -1) {
            assert(n->children.size() == 0);
            return node_state::undeveloped_nonterminal;
        }
        else {
            assert(n->children.size() > 0);
            assert(verify_children(n));
            return node_state::developed_nonterminal;
        }
    }
}

bool parse_tree::verify_children(node const * n) const {
    production p = g[n->production_index];

    auto match_seq = std::mismatch(p.rhs.begin(), p.rhs.end(), n->children.begin(),
    [](const symbol& s, const std::shared_ptr<node> t) { return s == t->my_symbol; });
    return match_seq.second == n->children.end();
}

std::shared_ptr<parse_tree::node> parse_tree::deep_copy(node const* p) const {
    assert(p != nullptr);
    auto new_root = make_shared<node>(p->my_symbol);
    for (auto&& c : p->children) {
        new_root->children.push_back(deep_copy(c.get()));
    }
    new_root->production_index = p->production_index;
    return new_root;
}

bool parse_tree::internal_apply_production(int production_index) {
    auto child = undeveloped_child();
    if (child == nullptr) { return false; }
    if (state(child) != node_state::undeveloped_nonterminal) {
        return false;
    }
    production new_production = g[production_index];
    if (child->my_symbol != new_production.lhs) {
        return false;
    }
    else {
        for (auto&& s : new_production.rhs) {
            auto developed_child = make_shared<node>(s);
            child->children.push_back(developed_child);
        }
        child->production_index = production_index;
        assert(state(child) == node_state::developed_nonterminal);
        return true;
    }
}




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
    parse_tree start(arithmetic);
    std::stack<parse_tree> work_list;
    work_list.push(start);
    while(work_list.size()) {
        auto x = work_list.top();
        work_list.pop();
        if (x.leaf_count() > atoi(argv[1])) {
            continue;
        }
        if (!x.has_undeveloped()) {
            cerr << x << endl;
        }
        else {
            for (auto&& t : all_develop_of_leaf(x)) {
                work_list.push(t);
            }
        }
    }
}
